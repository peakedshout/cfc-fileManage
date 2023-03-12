package cfile

import (
	"cfcfile/ctool"
	"encoding/base64"
	"github.com/peakedshout/go-CFC/loger"
	"github.com/peakedshout/go-CFC/tool"
	"time"
)

// --read--

// --remote--

// 远程读取
func (fc *FileContext) ReadRemoteFileContext(req ReadSomeReq, timeout time.Duration) (b []byte, err error) {
	if fc.remote == nil || fc.remote.cc == nil {
		err = ErrFcNotRemote
		loger.SetLogWarn(err)
		return nil, err
	}
	err = fc.remote.taskCtx.NewTaskCbCMsg(ctool.ReadFileQ1, 200, fc.remote.SetCFMsg(req)).WaitCb(10*time.Second, func(cMsg tool.ConnMsg) error {
		err1 := cMsg.CheckConnMsgHeaderAndCode(ctool.ReadFileA1, 200)
		if err1 != nil {
			return err1
		}
		var data string
		_, err1 = GetCFMsg(cMsg.Data, &data)
		if err1 != nil {
			return err1
		}
		b, err1 = base64.StdEncoding.DecodeString(data)
		if err1 != nil {
			return err1
		}
		return nil
	})
	if err != nil {
		loger.SetLogWarn(err)
		return nil, err
	}
	return b, nil
}

// --remote--

// --local--

// 本地读取
func (fc *FileContext) ReadLocalFileContext(req ReadSomeReq, read bool) ([]byte, error) {
	if read {
		err := fc.CheckPermission([]string{PermissionLink, PermissionOpen, PermissionRead})
		if err != nil {
			loger.SetLogWarn(err)
			return nil, err
		}
	} else {
		err := fc.CheckPermission([]string{PermissionLink, PermissionOpen, PermissionDownload})
		if err != nil {
			loger.SetLogWarn(err)
			return nil, err
		}
	}
	err := fc.checkRootSizeIsRangeOutExSize(0)
	if err != nil {
		loger.SetLogWarn(err)
		return nil, err
	}

	p, err := fc.CleanPath(req.Path, false)
	if err != nil {
		loger.SetLogWarn(err)
		return nil, err
	}
	b, err := ReadSome(p, req.Offset, req.Limit)
	if err != nil {
		loger.SetLogWarn(err)
		return nil, err
	}
	return b, nil
}

// --local--

// --read--
