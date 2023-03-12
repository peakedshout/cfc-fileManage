package cfile

import (
	"github.com/peakedshout/cfc-fileManage/ctool"
	"github.com/peakedshout/go-CFC/client"
	"github.com/peakedshout/go-CFC/loger"
	"github.com/peakedshout/go-CFC/tool"
	"os"
	"sync"
	"time"
)

// 初始化远程
func InitRemote(cc *client.DeviceBox, serverName, rawKey string) *FileContext {
	fc := &FileContext{
		Id:      0,
		Current: "",
		remote: &RemoteInfo{
			odjName:    serverName,
			key:        tool.Key{},
			rid:        "",
			sub:        nil,
			cc:         cc,
			stop:       make(chan uint8, 1),
			subMap:     make(map[string]map[int]*client.SubBox),
			subLock:    sync.Mutex{},
			onceCloser: sync.Once{},
			taskCtx:    nil,
		},
		Err:      "",
		FileList: nil,
	}
	sub, err := cc.GetSubBox(fc.remote.odjName)
	if err != nil {
		fc.Err = err.Error()
		loger.SetLogWarn(err)
		return fc
	}
	fc.remote.sub = sub
	fc.remote.key = sub.NewKey(rawKey)
	fc.remote.taskCtx = tool.NewTaskContext(fc.remote.sub, fc.remote.key)
	go func() {
		defer fc.remote.sub.Close()
		err := fc.remote.taskCtx.ReadCMsg()
		if err != nil {
			loger.SetLogWarn(err)
		}
	}()
	err = fc.remote.taskCtx.NewTaskCbCMsg(ctool.InitFileQ1, 200, fc.remote.SetCFMsg(nil)).WaitCb(10*time.Second, func(cMsg tool.ConnMsg) error {
		err1 := cMsg.CheckConnMsgHeaderAndCode(ctool.InitFileA1, 200)
		if err1 != nil {
			return err1
		}
		var info *FileContext
		rid, err1 := GetCFMsg(cMsg.Data, &info)
		if err1 != nil {
			return err1
		}
		if info.Err != "" {
			return tool.ErrReqBadAny(info.Err)
		}
		fc.remote.SetId(rid)
		fc.Current = info.Current
		fc.FileList = info.FileList
		fc.Config = info.Config
		go func() {
			tk := time.NewTicker(10 * time.Second)
			defer tk.Stop()
			for {
				select {
				case <-tk.C:
					err1 := fc.remote.taskCtx.WriteCMsg(ctool.Ping, "", 200, nil)
					if err1 != nil {
						loger.SetLogWarn(err1)
						return
					}
				case <-fc.remote.stop:
					return
				}
			}
		}()
		return nil
	})
	if err != nil {
		fc.Err = err.Error()
		loger.SetLogWarn(err)
		return fc
	}
	return fc
}

// 打开远程
func (fc *FileContext) OpenRemote(p string) {
	if fc.remote == nil || fc.remote.sub == nil {
		fc.Err = ErrFcNotRemote.Error()
		loger.SetLogWarn(ErrFcNotRemote)
		return
	}
	err := fc.remote.taskCtx.NewTaskCbCMsg(ctool.OpenFileQ1, 200, fc.remote.SetCFMsg(p)).WaitCb(10*time.Second, func(cMsg tool.ConnMsg) error {
		err1 := cMsg.CheckConnMsgHeaderAndCode(ctool.OpenFileA1, 200)
		if err1 != nil {
			return err1
		}
		var info *FileContext
		_, err1 = GetCFMsg(cMsg.Data, &info)
		if err1 != nil {
			return err1
		}
		if info.Err != "" {
			return tool.ErrReqBadAny(info.Err)
		}
		fc.Current = info.Current
		fc.FileList = info.FileList
		fc.UpdateRemoteRootSize(RootSizeInfo{
			RootSize:    info.Config.RootSize,
			MaxRootSize: info.Config.MaxRootSize,
			RootSizeErr: info.Config.RootSizeErr,
		})
		return nil
	})
	if err != nil {
		fc.Err = err.Error()
		loger.SetLogWarn(err)
		return
	}
}

// 初始化本地
func InitLocal(config *ConfigInfo) *FileContext {
	fc := &FileContext{
		Id:       0,
		Current:  "",
		remote:   nil,
		Config:   config,
		Err:      "",
		FileList: nil,
	}
	err := fc.CheckPermission([]string{PermissionLink, PermissionOpen})
	if err != nil {
		fc.Err = err.Error()
		loger.SetLogWarn(err)
		return fc
	}

	goPath := ""
	if config.DeviceType == DeviceTypeServer {
		goPath = config.Root
	} else {
		goPath, err = os.UserHomeDir()
		if err != nil {
			fc.Err = err.Error()
			loger.SetLogWarn(err)
			return fc
		}
	}
	f, err := os.Open(goPath)
	if err != nil {
		err = osIsErr(err)
		fc.Err = err.Error()
		loger.SetLogWarn(err)
		return fc
	}
	defer f.Close()
	list, err := f.Readdir(0)
	if err != nil {
		err = osIsErr(err)
		fc.Err = err.Error()
		loger.SetLogWarn(err)
		return fc
	}
	fc.FileList = FtoF(list)
	fc.UpdateLocalRootSize()
	return fc
}

// 打开本地
func (fc *FileContext) OpenLocal(p string) {
	fc.Err = ""
	err := fc.CheckPermission([]string{PermissionLink, PermissionOpen})
	if err != nil {
		fc.Err = err.Error()
		loger.SetLogWarn(err)
		return
	}
	gopath, err := fc.CleanPath(p, true)
	if err != nil {
		fc.Err = err.Error()
		loger.SetLogWarn(err)
	}
	f, err := os.Open(gopath)
	if err != nil {
		err = osIsErr(err)
		fc.Err = err.Error()
		loger.SetLogWarn(err)
		return
	}
	defer f.Close()
	list, err := f.Readdir(0)
	if err != nil {
		err = osIsErr(err)
		fc.Err = err.Error()
		loger.SetLogWarn(err)
		return
	}
	fc.FileList = FtoF(list)
}
