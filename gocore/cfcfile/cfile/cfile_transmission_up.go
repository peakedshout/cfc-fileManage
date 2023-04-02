package cfile

import (
	"errors"
	"fmt"
	"github.com/peakedshout/cfc-fileManage/ctool"
	"github.com/peakedshout/cfc-fileManage/memory"
	"github.com/peakedshout/go-CFC/loger"
	"github.com/peakedshout/go-CFC/tool"
	"os"
	"path"
	"time"
)

// --client--

// 创建上传
func (fc *FileContext) NewTaskUpFile(p, o string, tough bool, sizeBuff int) (infoPath string, err error) {
	if fc.remote == nil || fc.remote.sub == nil {
		err = ErrFcNotRemote
		loger.SetLogWarn(err)
		return "", err
	}
	gopath := path.Clean(p)
	outPath := path.Clean(o)

	info, err := fc.GetFileInfo(gopath)
	if err != nil {
		loger.SetLogWarn(err)
		return "", err
	}
	send := UpFileDetailInfo{
		FilePath: outPath,
		Tough:    tough,
		Size:     info.Size,
	}
	err = fc.remote.taskCtx.NewTaskCbCMsg(ctool.UploadFileQ1, 200, fc.remote.SetCFMsg(send)).WaitCb(10*time.Second, func(cMsg tool.ConnMsg) error {
		err1 := cMsg.CheckConnMsgHeaderAndCode(ctool.UploadFileA1, 200)
		if err1 != nil {
			return err1
		}
		return nil
	})
	if err != nil {
		loger.SetLogWarn(err)
		return "", err
	}
	req := NewTransmissionFileTaskInfoReq{
		RemotePath: outPath,
		LocalPath:  gopath,
		ServerName: fc.remote.odjName,
		Key:        fc.remote.key.GetRawKey(),
		SizeBuff:   sizeBuff,
		IsUp:       true,
	}
	err = NewTransmissionFileTaskInfo(info, req)
	if err != nil {
		loger.SetLogWarn(err)
		return "", err
	}
	return getCFCUploadInfo(gopath), nil
}

// 开始上传
func (fc *FileContext) StartTaskUpFile(p string, tough bool) (err error) {
	t1 := time.Now()
	var speed float64
	defer func() {
		t2 := time.Now().Sub(t1)
		s := t2.Seconds()
		fmt.Println("StartTaskUpFile Used:", t2, "AvgSpeed:", FormatFileSize(int64(speed/s))+"/s")
	}()
	if fc.remote == nil || fc.remote.sub == nil {
		err = ErrFcNotRemote
		loger.SetLogWarn(err)
		return err
	}
	p, _ = fc.CleanPath(p, false)
	tf, err := ReadTransmissionFileTaskInfo(p, fc.remote.key.GetRawKey())
	if err != nil {
		err = errors.New("parsing failure1 :" + err.Error())
		loger.SetLogWarn(err)
		return err
	}

	//状态处理w
	stopcb := false
	defer func() {
		if stopcb == true {
			err = tf.SetStatus(UpStatusUploadPause)
			if err != nil {
				loger.SetLogWarn(err)
			}
		} else if err != nil {
			if errors.Is(err, os.ErrNotExist) {
				tf.SetStatus(UpStatusUploadFileNil)
			} else if errors.Is(err, ErrFileHashCheckFailed) {
				tf.Reset()
				tf.SetStatus(UpStatusUploadFailure)
			} else {
				tf.SetStatus(UpStatusUploadFailure)
			}
		} else {
			err = tf.SetStatus(UpStatusUploadCompleted)
			if err != nil {
				loger.SetLogWarn(err)
			}
		}
	}()

	if tf.ServerName != fc.remote.odjName {
		err = ErrServerNameIsInconsistency
		loger.SetLogWarn(err)
		return err
	}

	info, err := fc.GetFileInfo(tf.LocalPath)
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}
	if info.ModTime != tf.ModTime || info.Hash != tf.Hash || info.Size != tf.Size {
		err = ErrFileIsModified
		loger.SetLogWarn(err)
		return err
	}

	send := UpFileDetailInfo{
		FilePath: tf.RemotePath,
		Tough:    tough,
		Size:     tf.Size,
		Check:    true,
	}
	err = fc.remote.taskCtx.NewTaskCbCMsg(ctool.UploadFileQ1, 200, fc.remote.SetCFMsg(send)).WaitCb(10*time.Second, func(cMsg tool.ConnMsg) error {
		err1 := cMsg.CheckConnMsgHeaderAndCode(ctool.UploadFileA1, 200)
		if err1 != nil {
			return err1
		}
		var reset bool
		_, err1 = GetCFMsg(cMsg.Data, &reset)
		if err1 != nil {
			return err1
		}
		if reset {
			tf.Reset()
			err1 = tf.SetStatus(UpStatusUploadReset)
			if err1 != nil {
				return err1
			}
		}
		return nil
	})
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}
	speed = float64(tf.SizeBuff * tf.GetTaskNum())

	//判断还有没有任务
	if tf.GetTaskNum() == 0 {
		err = fc.remote.taskCtx.NewTaskCbCMsg(ctool.UploadFileQ2, 200, fc.remote.SetCFMsg(UploadFileDataReq{
			FilePath: tf.RemotePath,
			Size:     tf.Size,
			Hash:     tf.Hash,
		})).WaitCb(10*time.Second, func(cMsg tool.ConnMsg) error {
			err1 := cMsg.CheckConnMsgHeaderAndCode(ctool.UploadFileA2, 200)
			if err1 != nil {
				return err1
			}
			return nil
		})
		if err != nil {
			loger.SetLogWarn(err)
			return err
		}
		return nil
	}

	err = tf.SetStatus(UpStatusUploadInProgress)
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}
	sub, err := fc.remote.cc.GetSubBox(tf.ServerName)
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}
	defer sub.Close()

	err = loginCheck(sub, fc.Config.UserName, tf.Key)
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}

	defer func() {
		memory.FDel(p)
	}()
	fc.remote.SetSub(p, sub)
	defer fc.remote.DelSub(p)
	f, err := os.OpenFile(tf.LocalPath, os.O_RDWR, 0666)
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}
	defer f.Close()

	tc := tool.NewTaskContext(sub, sub.NewKey(tf.Key))
	go func() {
		err := tc.ReadCMsg()
		if err != nil {
			sub.SetInfoLog(err)
			sub.Close()
		}
	}()
	offset := tf.Get()
	data, stop, errChan := fc.ReadFileToSend(tf.LocalPath, int64(offset*tf.SizeBuff), tf.SizeBuff)

	tid := tool.NewId(1)
	task := tc.NewTaskCb(tid, nil).SetKeep(true)

	//设置暂停回调
	memory.FSetOnly(p, func() {
		stopcb = true
		task.OverTask(ErrTransmissionStop)
		sub.Close()
	})

	go func() {
		r := true
		req := UploadFileDataReq{
			FilePath: tf.RemotePath,
			Size:     tf.Size,
			Hash:     tf.Hash,
			Offset:   offset,
			SizeBuff: tf.SizeBuff,
			Data:     nil,
		}
		for r {
			select {
			case err := <-errChan:
				task.OverTask(err)
				return
			case b, ok := <-data:
				if ok {
					req.Data = b
					err := tc.WriteCMsg(ctool.UploadFileQ2, tid, 100, fc.remote.SetCFMsg(req))
					if err != nil {
						task.OverTask(err)
						return
					}
				} else {
					req.Data = nil
					req.SizeBuff = 0
					req.Offset = 0
					err := tc.WriteCMsg(ctool.UploadFileQ2, tid, 200, fc.remote.SetCFMsg(req))
					if err != nil {
						task.OverTask(err)
						return
					}
					r = false
				}
			}
			req.Offset++
		}
	}()

	err = task.KeepWaitCb(10*time.Second, func(cMsg tool.ConnMsg) error {
		if cMsg.Header != ctool.UploadFileA2 {
			return tool.ErrReqUnexpectedHeader
		}
		if cMsg.Code == 100 {
			var info1 int
			_, err1 := GetCFMsg(cMsg.Data, &info1)
			if err1 != nil {
				stop <- 1
				return err1
			}
			err1 = tf.VerifyOffset(info1)
			if err1 != nil {
				stop <- 1
				return err1
			}
		} else if cMsg.Code == 200 {
			stop <- 1
			task.OverTask(nil)
			return nil
		} else {
			stop <- 1
			return errors.New("UploadBad: " + cMsg.Data.(string))
		}
		return nil
	})
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}
	return nil
}

// --client--

// --server--

// 上传检查文件
func (fc *FileContext) NewOrCheckUpFile(ufi UpFileDetailInfo) (reset bool, err error) {
	err = fc.CheckPermission([]string{PermissionLink, PermissionOpen, PermissionUpload})
	if err != nil {
		loger.SetLogWarn(err)
		return reset, err
	}
	ufi.FilePath, err = fc.CleanPath(ufi.FilePath, false)
	if err != nil {
		loger.SetLogWarn(err)
		return reset, err
	}

	err = fc.checkRootSizeIsRangeOutExSize(ufi.Size)
	if err != nil {
		loger.SetLogWarn(err)
		return reset, err
	}

	if !ufi.Check {
		if !ufi.Tough && checkLocalFileIsExist(ufi.FilePath) {
			err = errors.New("file is exist: " + ufi.FilePath)
			loger.SetLogWarn(err)
			return reset, err
		}
		err = MakeZeroFile(getCFCUploadFile(ufi.FilePath), int(ufi.Size))
		if err != nil {
			loger.SetLogWarn(err)
			return reset, err
		}
	} else {
		stat, err := os.Stat(getCFCUploadFile(ufi.FilePath))
		if err != nil {
			if os.IsNotExist(err) { //如果下载文件不存在，重置
				reset = true
				err = MakeZeroFile(getCFCUploadFile(ufi.FilePath), int(ufi.Size))
				if err != nil {
					err = errors.New("remote make file :" + osIsErr(err).Error())
					loger.SetLogWarn(err)
					return reset, err
				}
			} else {
				err = errors.New("remote parsing failure :" + osIsErr(err).Error())
				loger.SetLogWarn(err)
				return reset, err
			}
		}
		if stat.Size() != ufi.Size || ufi.Tough {
			if ufi.Tough { //重置
				reset = true
				err = MakeZeroFile(getCFCUploadFile(ufi.FilePath), int(ufi.Size))
				if err != nil {
					err = errors.New("remote make file :" + osIsErr(err).Error())
					loger.SetLogWarn(err)
					return reset, err
				}
			} else {
				err = errors.New("remote parsing failure :" + "file information is inconsistent")
				loger.SetLogWarn(err)
				return reset, err
			}
		}
	}
	return reset, nil
}

// 接受写入文件
func (fc *FileContext) ReceiveDataToWrite(info UploadFileDataReq, over bool) (err error) {
	err = fc.CheckPermission([]string{PermissionLink, PermissionOpen, PermissionUpload})
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}
	info.FilePath, err = fc.CleanPath(info.FilePath, false)
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}

	err = fc.checkRootSizeIsRangeOutExSize(0)
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}

	f, err := os.OpenFile(getCFCUploadFile(info.FilePath), os.O_RDWR, 0666)
	if err != nil {
		err = osIsErr(err)
		loger.SetLogWarn(err)
		return err
	}
	defer f.Close()
	s, err := f.Stat()
	if err != nil {
		err = osIsErr(err)
		loger.SetLogWarn(err)
		return err
	}
	if s.Size() != info.Size {
		err = errors.New("upload write bad: Unexpected file")
		loger.SetLogWarn(err)
		return err
	}
	if over {
		f.Close()
		if info.Hash != "" {
			h, err := GetFileHash(getCFCUploadFile(info.FilePath))
			if err != nil {
				loger.SetLogWarn(err)
				return err
			}
			if h != info.Hash {
				fmt.Println("hash check err:", h, info.Hash)
				os.Remove(getCFCUploadFile(info.FilePath))
				err = ErrFileHashCheckFailed
				loger.SetLogWarn(err)
				return err
			}
		}
		//善后
		err = os.Rename(getCFCUploadFile(info.FilePath), info.FilePath)
		if err != nil {
			err = osIsErr(err)
			loger.SetLogWarn(err)
			return err
		}
	} else {
		if len(info.Data) != 0 {
			_, err := f.WriteAt(info.Data, int64(info.Offset*info.SizeBuff))
			if err != nil {
				err = osIsErr(err)
				loger.SetLogWarn(err)
				return err
			}
		}
	}
	return nil
}

// --server--
