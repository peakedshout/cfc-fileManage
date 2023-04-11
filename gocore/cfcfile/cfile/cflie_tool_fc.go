package cfile

import (
	"github.com/peakedshout/cfc-fileManage/ctool"
	"github.com/peakedshout/go-CFC/client"
	"github.com/peakedshout/go-CFC/loger"
	"github.com/peakedshout/go-CFC/tool"
	"os"
	"path"
	"strings"
	"time"
)

func loginCheck(sub *client.SubBox, userName, rawKey string) error {
	err := sub.WriteCMsg(ctool.LoginQ1, "", 200, CFCLoginReq{
		LoginName: userName,
		LoginKey:  rawKey,
	})
	if err != nil {
		return err
	}
	err = sub.ReadCMsgCb(func(cMsg tool.ConnMsg) (bool, error) {
		err1 := cMsg.CheckConnMsgHeaderAndCode(ctool.LoginA1, 200)
		if err1 != nil {
			return false, err
		}
		return false, nil
	})
	if err != nil {
		return err
	}
	return nil
}

func (fc *FileContext) rootPathTrim(p string) string {
	if fc.Config.DeviceType == DeviceTypeServer {
		return strings.TrimPrefix(path.Clean(p), fc.Config.Root)
	} else {
		return p
	}
}

// 本地获取文件信息
func (fc *FileContext) GetFileInfo(p string) (info FileDetailInfo, err error) {
	err = fc.CheckPermission([]string{PermissionLink, PermissionOpen})
	if err != nil {
		return info, err
	}
	gopath, err := fc.CleanPath(p, false)
	if err != nil {
		return info, err
	}
	info, err = GetFileDetailInfo(gopath, true)
	return
}

// 检查root大小是否超出预设
func (fc *FileContext) checkRootSizeIsRangeOutExSize(exSize int64) error {
	fc.Config.rootSizeLock.Lock()
	defer fc.Config.rootSizeLock.Unlock()
	if fc.Config.DeviceType == DeviceTypeClient {
		return nil
	}
	if fc.Config.MaxRootSize == 0 {
		return nil
	}
	if exSize+fc.Config.RootSize > fc.Config.MaxRootSize {
		return ErrExceedMaxRootSize
	}
	return nil
}

// 检查root大小是否超出预设
func (fc *FileContext) checkRootSizeIsRangeOut(p string) error {
	fc.Config.rootSizeLock.Lock()
	defer fc.Config.rootSizeLock.Unlock()
	if fc.Config.DeviceType == DeviceTypeClient {
		return nil
	}
	if fc.Config.MaxRootSize == 0 {
		return nil
	}
	var exSize int64
	if p != "" { //the path is must clean
		info, err := os.Stat(p)
		if err != nil {
			return ErrComputeStorage
		}
		exSize = info.Size()
	}
	if exSize+fc.Config.RootSize > fc.Config.MaxRootSize {
		return ErrExceedMaxRootSize
	}
	return nil
}

// 关闭
func (fc *FileContext) Close() {
	if fc.remote != nil && fc.remote.sub != nil {
		fc.remote.onceCloser.Do(func() {
			fc.remote.sub.Close()
			fc.remote.stop <- 1
		})
	}
}

// 检查权限
func (fc *FileContext) CheckPermission(permissions ...[]string) error {
	if !fc.Config.checkPermission(permissions...) {
		return ErrMissingPermission
	}
	return nil
}

// 清理路径
func (fc *FileContext) CleanPath(p string, cover bool) (string, error) {
	if fc.Config.DeviceType == DeviceTypeClient {
		p1 := path.Clean(p)
		if cover {
			fc.Current = p1
		}
		return p1, nil
	}
	if fc.Config.DeviceType == DeviceTypeServer {
		p1 := path.Clean(p)
		p2 := path.Join(fc.Config.Root, p1)
		if !strings.HasPrefix(p2, fc.Config.Root) {
			return "", ErrRootPathRangeOut
		}
		if cover {
			fc.Current = p1
		}
		return p2, nil
	}
	return "", ErrDeviceType
}

// 更新本地root信息(服务端用)
func (fc *FileContext) UpdateLocalRootSize() {
	if fc.Config.DeviceType == DeviceTypeClient {
		return
	}
	fc.Config.rootSizeLock.Lock()
	defer fc.Config.rootSizeLock.Unlock()
	err := fc.Config.updateRootSize()
	if err != nil {
		fc.Config.RootSizeErr = err.Error()
	} else {
		fc.Config.RootSizeErr = ""
	}
	return
}

// 更新远程root信息(客户端用)
func (fc *FileContext) UpdateRemoteRootSize(info RootSizeInfo) {
	if fc.Config.DeviceType == DeviceTypeClient {
		return
	}
	fc.Config.rootSizeLock.Lock()
	defer fc.Config.rootSizeLock.Unlock()
	fc.Config.RootSize = info.RootSize
	fc.Config.MaxRootSize = info.MaxRootSize
	fc.Config.RootSizeErr = info.RootSizeErr
}

// 获取root信息
func (fc *FileContext) GetRootSize() RootSizeInfo {
	var info RootSizeInfo
	if fc.Config.DeviceType == DeviceTypeClient {
		info.RootSizeErr = "not data: is client"
		return info
	}
	fc.Config.rootSizeLock.Lock()
	defer fc.Config.rootSizeLock.Unlock()
	info.RootSize = fc.Config.RootSize
	info.MaxRootSize = fc.Config.MaxRootSize
	info.RootSizeErr = fc.Config.RootSizeErr
	return info
}

// 获取该会话的网速
func (fc *FileContext) GetSpeed() tool.NetworkSpeedView {
	if fc.remote == nil || fc.remote.sub == nil {
		return tool.NetworkSpeedView{}
	}
	return fc.remote.GetAllSubSpeed()
}

// 获取该传输任务的网速
func (fc *FileContext) GetTransmissionSpeed(key string) tool.NetworkSpeedView {
	if fc.remote == nil || fc.remote.sub == nil {
		return tool.NetworkSpeedView{}
	}
	return tool.CountAllNetworkSpeedView(fc.remote.GetSubSpeed(key)[0]...)
}

// 获取远程状态
func (fc *FileContext) GetRemoteStatus() (resp RemoteStatus) {
	if fc.remote == nil || fc.remote.sub == nil || fc.remote.cc == nil {
		fc.Err = ErrFcNotRemote.Error()
		return
	}
	list, err := fc.remote.cc.GetOtherDelayPing(fc.remote.sub.GetLocalName(), fc.remote.sub.GetRemoteName())
	if err != nil {
		resp.DelayErr = err.Error()
		loger.SetLogWarn(err)
		return
	}
	if len(list) != 2 {
		resp.DelayErr = "bad req to GetOtherDelayPing"
		loger.SetLogWarn(resp.DelayErr)
		return
	}
	if !list[0].Active {
		resp.DelayErr += list[0].Name + " is inactive;"
	}
	if !list[1].Active {
		resp.DelayErr += list[1].Name + " is inactive;"
	}
	if resp.DelayErr != "" {
		loger.SetLogWarn(resp.DelayErr)
		return
	}
	tPing := list[0].Ping.Ping + list[1].Ping.Ping
	resp.Delay = tPing.String()
	resp.DelayRaw = tPing.Nanoseconds()
	err = fc.remote.taskCtx.NewTaskCbCMsg(ctool.StatusQ1, 200, fc.remote.SetCFMsg(nil)).WaitCb(10*time.Second, func(cMsg tool.ConnMsg) error {
		var info RootSizeInfo
		err1 := cMsg.CheckConnMsgHeaderAndCode(ctool.StatusA1, 200)
		if err1 != nil {
			info.RootSizeErr = err1.Error()
			return err1
		}
		_, err1 = GetCFMsg(cMsg.Data, &info)
		if err1 != nil {
			info.RootSizeErr = err1.Error()
			return err1
		}
		fc.UpdateRemoteRootSize(info)
		return nil
	})
	if err != nil {
		resp.RootSizeErr = err.Error()
		loger.SetLogWarn(err)
		return
	}
	info := fc.GetRootSize()
	resp.RootSize = info.RootSize
	resp.MaxRootSize = info.MaxRootSize
	resp.RootSizeShow = FormatFileSize(info.RootSize)
	resp.RootSizeErr = info.RootSizeErr
	resp.MaxRootSizeShow = FormatFileSize(info.MaxRootSize)
	resp.NetworkSpeed = fc.GetSpeed()
	return resp
}
