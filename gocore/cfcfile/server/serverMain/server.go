package serverMain

import (
	"encoding/json"
	"github.com/peakedshout/cfc-fileManage/cfile"
	"github.com/peakedshout/cfc-fileManage/ctool"
	"github.com/peakedshout/cfc-fileManage/memory"
	"github.com/peakedshout/go-CFC/client"
	"github.com/peakedshout/go-CFC/loger"
	"github.com/peakedshout/go-CFC/server"
	"github.com/peakedshout/go-CFC/tool"
	"net"
	"os"
	"sync"
	"time"
)

var userLock sync.Mutex
var userM map[string]*cfile.ConfigInfo

func init() {
	userM = make(map[string]*cfile.ConfigInfo)
}

func RunAll(cp string) {
	info := readConfig(cp)
	checkConfig(info)
	if info.Setting.LogLevel == 0 {
		info.Setting.LogLevel = loger.LogLevelWarn
	}
	loger.SetLoggerLevel(info.Setting.LogLevel)
	loger.SetLoggerStack(info.Setting.LogStack)

	err := tool.ReRun(info.Setting.ReLinkTime, func() {
		addr, err := net.ResolveTCPAddr("tcp", info.Ct.Addr)
		if err != nil {
			loger.SetLogError("bad config")
		}
		if addr.IP.String() == "127.0.0.1" {
			s := server.NewProxyServer(info.Ct.Addr, info.Ct.Key)
			defer s.Close()
			time.Sleep(1 * time.Second)
		}

		c, err := RunMain(info.Config, &info.Ct)
		if err != nil {
			loger.SetLogWarn(err)
			return
		}
		defer c.Close()
		c.Wait()
	})
	if err != nil {
		loger.SetLogError(err)
	}
}

var sm *memory.SMemory

func RunMain(bconfig ctool.BaseConfig, ctConfig *ctool.CtConfig) (*client.DeviceBox, error) {
	sm = memory.InitS()

	c, err := client.LinkProxyServer(bconfig.DeviceName, ctConfig.Addr, ctConfig.Key)
	if err != nil {
		loger.SetLogWarn(err)
		return nil, err
	}
	go func() {
		err = c.ListenSubBox(func(sub *client.SubBox) {
			defer sub.Close()
			sub.SetDeadline(time.Now().Add(20 * time.Second))
			var config *cfile.ConfigInfo
			err = sub.ReadCMsgCb(func(cMsg tool.ConnMsg) (bool, error) {
				err1 := cMsg.CheckConnMsgHeaderAndCode(ctool.LoginQ1, 200)
				if err1 != nil {
					return false, err
				}
				var info cfile.CFCLoginReq
				err1 = cMsg.Unmarshal(&info)
				if err1 != nil {
					return false, err1
				}
				u, ok := getUserM(info.LoginName)
				if !ok {
					err1 = cfile.ErrLoginUserErr
					return false, err1
				}
				if u.RawKey != info.LoginKey {
					err1 = cfile.ErrLoginUserErr
					return false, err1
				}
				config = u
				return false, nil
			})
			if err != nil {
				sub.WriteCMsg(ctool.LoginA1, "", 400, err.Error())
				return
			}
			err = sub.WriteCMsg(ctool.LoginA1, "", 200, nil)
			if err != nil {
				return
			}
			ping := make(chan uint8, 1)
			stop := make(chan uint8, 1)
			defer func() { stop <- 1 }()

			tc := tool.NewTaskContext(sub, sub.NewKey(config.RawKey))
			ctx := &handleCtx{
				tc:  tc,
				sub: sub,
			}
			tc.SetNoCb(func(cMsg tool.ConnMsg) {
				if cMsg.Header == ctool.UploadFileQ2 {
					sub.SetInfoLog(cMsg.Header, cMsg.Id, cMsg.Code)
				} else {
					sub.SetInfoLog(cMsg)
				}
				switch cMsg.Header {
				case ctool.InitFileQ1:
					fc := cfile.InitLocal(config)
					if fc.Err == "" {
						rid := sm.Set(fc)
						go func() {
							for {
								select {
								case <-ping:
									if sub.SetDeadline(time.Now().Add(20*time.Second)) != nil {
										sm.Del(rid)
										fc.Close()
										return
									}
									fc.UpdateLocalRootSize()
								case <-stop:
									stop <- 1
									sm.Del(rid)
									fc.Close()
									return
								}
							}
						}()
						err1 := tc.WriteCMsg(ctool.InitFileA1, cMsg.Id, 200, cfile.SetCFMsg(rid, fc))
						if err1 != nil {
							loger.SetLogWarn(err1)
						}
					} else {
						err1 := tc.WriteCMsg(ctool.InitFileA1, cMsg.Id, 400, fc.Err)
						if err1 != nil {
							loger.SetLogWarn(err1)
						}
					}
				case ctool.Ping:
					ping <- 1
				default:
					ctx.cMsgHandler(cMsg)
				}
			})
			err = tc.ReadCMsg()
			if err != nil {
				sub.SetWarnLog(err)
				return
			}
		})
		if err != nil {
			c.SetWarnLog(err)
			return
		}
	}()
	return c, nil
}

type handleCtx struct {
	tc  *tool.TaskCbContext
	sub *client.SubBox
}

func (ctx *handleCtx) cMsgHandlerPre(header string, cMsg tool.ConnMsg) (fc *cfile.FileContext, msg cfile.CFCFileCMsg, err error) {
	msg, err = cfile.GetCFMsgStep1(cMsg.Data)
	if err != nil {
		ctx.tc.WriteCMsg(header, cMsg.Id, 400, err.Error())
		loger.SetLogWarn(err)
		return nil, msg, err
	}
	if msg.Id == "" {
		err = cfile.ErrFileContextIsNotFound
		ctx.tc.WriteCMsg(header, cMsg.Id, 400, err.Error())
		if err != nil {
			loger.SetLogWarn(err)
		}
		return nil, msg, err
	}
	odj, ok := sm.Get(msg.Id)
	if !ok {
		err = cfile.ErrFileContextIsNotFound
		ctx.tc.WriteCMsg(header, cMsg.Id, 400, err.Error())
		if err != nil {
			loger.SetLogWarn(err)
		}
		return nil, msg, err
	}
	fc, ok = odj.(*cfile.FileContext)
	if !ok {
		sm.Del(msg.Id)
		err = cfile.ErrFileContextIsNotFound
		ctx.tc.WriteCMsg(header, cMsg.Id, 400, err.Error())
		if err != nil {
			loger.SetLogWarn(err)
		}
		return nil, msg, err
	}
	return fc, msg, err
}

func (ctx *handleCtx) cMsgHandler(cMsg tool.ConnMsg) (err error) {
	switch cMsg.Header {
	case ctool.OpenFileQ1:
		header := ctool.OpenFileA1
		fc, msg, err := ctx.cMsgHandlerPre(header, cMsg)
		if err != nil {
			return err
		}

		var info string
		err = msg.GetCFMsgStep2(&info)
		if err != nil {
			loger.SetLogWarn(err)
			return err
		}
		fc.OpenLocal(info)
		err = ctx.tc.WriteCMsg(header, cMsg.Id, 200, msg.SetCFMsg(fc))
		if err != nil {
			loger.SetLogWarn(err)
			return err
		}
	case ctool.CopyFileQ1:
		header := ctool.CopyFileA1
		fc, msg, err := ctx.cMsgHandlerPre(header, cMsg)
		if err != nil {
			return err
		}

		var info cfile.ONOpReq
		err = msg.GetCFMsgStep2(&info)
		if err != nil {
			loger.SetLogWarn(err)
			return err
		}
		err = fc.CopyLocalFileContext(info.Old, info.New, info.Tough)
		if err != nil {
			ctx.tc.WriteCMsg(header, cMsg.Id, 400, err.Error())
			return err
		}
		err = ctx.tc.WriteCMsg(header, cMsg.Id, 200, msg.SetCFMsg(nil))
		if err != nil {
			loger.SetLogWarn(err)
			return err
		}
	case ctool.MoveFileQ1:
		header := ctool.MoveFileA1
		fc, msg, err := ctx.cMsgHandlerPre(header, cMsg)
		if err != nil {
			return err
		}

		var info cfile.ONOpReq
		err = msg.GetCFMsgStep2(&info)
		if err != nil {
			loger.SetLogWarn(err)
			return err
		}
		err = fc.MoveLocalFileContext(info.Old, info.New, info.Tough)
		if err != nil {
			ctx.tc.WriteCMsg(header, cMsg.Id, 400, err.Error())
			return err
		}
		err = ctx.tc.WriteCMsg(header, cMsg.Id, 200, msg.SetCFMsg(nil))
		if err != nil {
			loger.SetLogWarn(err)
			return err
		}
	case ctool.RenameFileQ1:
		header := ctool.RenameFileA1
		fc, msg, err := ctx.cMsgHandlerPre(header, cMsg)
		if err != nil {
			return err
		}

		var info cfile.ONOpReq
		err = msg.GetCFMsgStep2(&info)
		if err != nil {
			loger.SetLogWarn(err)
			return err
		}
		err = fc.RenameLocalFileContext(info.Old, info.New, info.Tough)
		if err != nil {
			ctx.tc.WriteCMsg(header, cMsg.Id, 400, err.Error())
			return err
		}
		err = ctx.tc.WriteCMsg(header, cMsg.Id, 200, msg.SetCFMsg(nil))
		if err != nil {
			loger.SetLogWarn(err)
			return err
		}
	case ctool.RemoveFileQ1:
		header := ctool.RemoveFileA1
		fc, msg, err := ctx.cMsgHandlerPre(header, cMsg)
		if err != nil {
			return err
		}

		var info string
		err = msg.GetCFMsgStep2(&info)
		if err != nil {
			loger.SetLogWarn(err)
			return err
		}
		err = fc.RemoveLocalFileContext(info)
		if err != nil {
			ctx.tc.WriteCMsg(header, cMsg.Id, 400, err.Error())
			return err
		}
		err = ctx.tc.WriteCMsg(header, cMsg.Id, 200, msg.SetCFMsg(nil))
		if err != nil {
			loger.SetLogWarn(err)
			return err
		}
	case ctool.MkDirFileQ1:
		header := ctool.MkDirFileA1
		fc, msg, err := ctx.cMsgHandlerPre(header, cMsg)
		if err != nil {
			return err
		}

		var info string
		err = msg.GetCFMsgStep2(&info)
		if err != nil {
			loger.SetLogWarn(err)
			return err
		}
		err = fc.MkDirLocalFileContext(info)
		if err != nil {
			ctx.tc.WriteCMsg(header, cMsg.Id, 400, err.Error())
			return err
		}
		err = ctx.tc.WriteCMsg(header, cMsg.Id, 200, msg.SetCFMsg(nil))
		if err != nil {
			loger.SetLogWarn(err)
			return err
		}
	case ctool.FileInfoQ1:
		header := ctool.FileInfoA1
		fc, msg, err := ctx.cMsgHandlerPre(header, cMsg)
		if err != nil {
			return err
		}

		var info string
		err = msg.GetCFMsgStep2(&info)
		if err != nil {
			loger.SetLogWarn(err)
			return err
		}
		data, err := fc.GetFileInfo(info)
		if err != nil {
			ctx.tc.WriteCMsg(header, cMsg.Id, 400, err.Error())
			return err
		}
		err = ctx.tc.WriteCMsg(header, cMsg.Id, 200, msg.SetCFMsg(data))
		if err != nil {
			loger.SetLogWarn(err)
			return err
		}
	case ctool.ReadFileQ1:
		header := ctool.ReadFileA1
		fc, msg, err := ctx.cMsgHandlerPre(header, cMsg)
		if err != nil {
			return err
		}

		var info cfile.ReadSomeReq
		err = msg.GetCFMsgStep2(&info)
		if err != nil {
			loger.SetLogWarn(err)
			return err
		}
		b, err := fc.ReadLocalFileContext(info, true)
		if err != nil {
			ctx.tc.WriteCMsg(header, cMsg.Id, 400, err.Error())
			return err
		}
		err = ctx.tc.WriteCMsg(header, cMsg.Id, 200, msg.SetCFMsg(tool.MustBytesToBase64(b).Data))
		if err != nil {
			loger.SetLogWarn(err)
			return err
		}
	case ctool.DownloadFileQ1:
		header := ctool.DownloadFileA1
		fc, msg, err := ctx.cMsgHandlerPre(header, cMsg)
		if err != nil {
			return err
		}

		var info cfile.DownFileReq
		err = msg.GetCFMsgStep2(&info)
		if err != nil {
			loger.SetLogWarn(err)
			return err
		}
		err = ctx.sub.SetDeadline(time.Time{})
		if err != nil {
			loger.SetLogWarn(err)
			return err
		}
		defer func() {
			ctx.sub.SetDeadline(time.Now().Add(10 * time.Second))
		}()
		data, stop, errChan := fc.ReadFileToSend(info.Path, info.Offset, info.Limit)
		r := true
		for r {
			select {
			case err := <-errChan:
				r = false
				ctx.tc.WriteCMsg(header, cMsg.Id, 400, err.Error())
			case b, ok := <-data:
				if ok {
					err = ctx.tc.WriteCMsg(header, cMsg.Id, 100, msg.SetCFMsg(b))
					if err != nil {
						stop <- 1
						r = false
						loger.SetLogWarn(err)
						return err
					}
				} else {
					err = ctx.tc.WriteCMsg(header, cMsg.Id, 200, msg.SetCFMsg(nil))
					if err != nil {
						stop <- 1
						r = false
						loger.SetLogWarn(err)
						return err
					}
					r = false
				}
			}
		}
	case ctool.UploadFileQ1:
		header := ctool.UploadFileA1
		fc, msg, err := ctx.cMsgHandlerPre(header, cMsg)
		if err != nil {
			return err
		}

		var info cfile.UpFileDetailInfo
		err = msg.GetCFMsgStep2(&info)
		if err != nil {
			loger.SetLogWarn(err)
			return err
		}
		result, err := fc.NewOrCheckUpFile(info)
		if err != nil {
			ctx.tc.WriteCMsg(header, cMsg.Id, 400, err.Error())
			return err
		}
		err = ctx.tc.WriteCMsg(header, cMsg.Id, 200, msg.SetCFMsg(result))
		if err != nil {
			loger.SetLogWarn(err)
			return err
		}
	case ctool.UploadFileQ2:
		header := ctool.UploadFileA2
		fc, msg, err := ctx.cMsgHandlerPre(header, cMsg)
		if err != nil {
			return err
		}

		var info cfile.UploadFileDataReq
		err = msg.GetCFMsgStep2(&info)
		if err != nil {
			loger.SetLogWarn(err)
			return err
		}
		over := cMsg.Code == 200
		err = fc.ReceiveDataToWrite(info, over)
		if err != nil {
			ctx.tc.WriteCMsg(header, cMsg.Id, 400, err.Error())
			return err
		}
		if over {
			err = ctx.tc.WriteCMsg(header, cMsg.Id, 200, msg.SetCFMsg(nil))
			if err != nil {
				loger.SetLogWarn(err)
				return err
			}
		} else {
			err = ctx.tc.WriteCMsg(header, cMsg.Id, 100, msg.SetCFMsg(info.Offset))
			if err != nil {
				loger.SetLogWarn(err)
				return err
			}
		}
		err = ctx.sub.SetDeadline(time.Now().Add(10 * time.Second))
		if err != nil {
			loger.SetLogWarn(err)
			return err
		}
	case ctool.StatusQ1:
		header := ctool.StatusA1
		fc, msg, err := ctx.cMsgHandlerPre(header, cMsg)
		if err != nil {
			return err
		}
		err = ctx.tc.WriteCMsg(header, cMsg.Id, 200, msg.SetCFMsg(fc.GetRootSize()))
		if err != nil {
			loger.SetLogWarn(err)
			return err
		}
	}
	return nil
}

func readConfig(cPath string) ctool.ServerConfig {
	b, err := os.ReadFile(cPath)
	if err != nil {
		loger.SetLogError(err)
	}
	var info ctool.ServerConfig
	err = json.Unmarshal(b, &info)
	if err != nil {
		loger.SetLogError(err)
	}
	return info
}
func checkConfig(info ctool.ServerConfig) {
	if info.Ct.Addr == "" || len(info.Ct.Key) != 32 {
		loger.SetLogError("bad config")
	}
	if info.Config.DeviceName == "" {
		loger.SetLogError("bad config")
	}
	if info.Config.DeviceType == cfile.DeviceTypeServer {
		for _, one := range info.Config.UserConfigs {
			if one.UserName == "" || len(one.UserKey) != 32 {
				loger.SetLogError("bad user config", one.UserName, one.UserKey)
			}
			_, err := os.Stat(one.Root)
			if one.Root == "" || err != nil {
				loger.SetLogError("bad root config", one.Root, err)
			}
			if one.MaxRootSize < 0 {
				loger.SetLogError("bad MaxRootSize config", one.MaxRootSize)
			}
			checkPermissionList(one.PermissionList)

			config := &cfile.ConfigInfo{
				DeviceName:     info.Config.DeviceName,
				DeviceType:     info.Config.DeviceType,
				Root:           one.Root,
				RootSize:       0,
				MaxRootSize:    one.MaxRootSize,
				UserName:       one.UserName,
				RawKey:         one.UserKey,
				PermissionList: one.PermissionList,
			}
			setUserM(one.UserName, config)
		}
	} else if info.Config.DeviceType == cfile.DeviceTypeClient {
	} else {
		loger.SetLogError("bad config")
	}
}

func checkPermissionList(l []string) {
	m := make(map[string]bool)
	for _, one := range cfile.PermissionToAll {
		m[one] = true
	}
	for _, one := range l {
		if !m[one] {
			loger.SetLogError("bad config", l, "not found:", one)
		}
	}
}

func setUserM(name string, config *cfile.ConfigInfo) {
	userLock.Lock()
	defer userLock.Unlock()
	userM[name] = config
}
func getUserM(name string) (*cfile.ConfigInfo, bool) {
	userLock.Lock()
	defer userLock.Unlock()
	config, ok := userM[name]
	return config, ok
}
func RunTest(info ctool.ServerConfig) (*client.DeviceBox, error) {
	checkConfig(info)
	c, err := RunMain(info.Config, &info.Ct)
	if err != nil {
		loger.SetLogError(err)
	}
	return c, nil
}
