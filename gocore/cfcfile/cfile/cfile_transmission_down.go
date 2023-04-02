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

// 创建下载
func (fc *FileContext) NewTaskDownFile(p, o string, tough bool, sizeBuff int) (infoPath string, err error) {
	if fc.remote == nil || fc.remote.sub == nil {
		err = ErrFcNotRemote
		loger.SetLogWarn(err)
		return "", err
	}
	gopath := path.Clean(p)
	outPath := path.Clean(o)
	if !tough && checkLocalFileIsExist(outPath) {
		err = errors.New("file is exist: " + outPath)
		loger.SetLogWarn(err)
		return "", err
	}

	var info FileDetailInfo
	err = fc.remote.taskCtx.NewTaskCbCMsg(ctool.FileInfoQ1, 200, fc.remote.SetCFMsg(gopath)).WaitCb(10*time.Second, func(cMsg tool.ConnMsg) error {
		err1 := cMsg.CheckConnMsgHeaderAndCode(ctool.FileInfoA1, 200)
		if err1 != nil {
			return err1
		}
		_, err1 = GetCFMsg(cMsg.Data, &info)
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
		RemotePath: gopath,
		LocalPath:  outPath,
		ServerName: fc.remote.odjName,
		Key:        fc.remote.key.GetRawKey(),
		SizeBuff:   sizeBuff,
		IsUp:       false,
	}
	err = NewTransmissionFileTaskInfo(info, req)
	if err != nil {
		loger.SetLogWarn(err)
		return "", err
	}
	err = MakeZeroFile(getCFCDownloadFile(outPath), int(info.Size))
	if err != nil {
		err = errors.New("make file :" + osIsErr(err).Error())
		loger.SetLogWarn(err)
		return "", err
	}
	return getCFCDownloadInfo(outPath), nil
}

// 开始下载
func (fc *FileContext) StartTaskDownFile(p string, tough bool) (err error) {
	t1 := time.Now()
	var speed float64
	defer func() {
		t2 := time.Now().Sub(t1)
		s := t2.Seconds()
		fmt.Println("StartTaskDownFile Used:", t2, "AvgSpeed:", FormatFileSize(int64(speed/s))+"/s")
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
	//状态处理
	stopcb := false
	defer func() {
		if stopcb == true {
			err = tf.SetStatus(DownStatusDownloadPause)
			if err != nil {
				loger.SetLogWarn(err)
			}
		} else if err != nil {
			if errors.Is(err, os.ErrNotExist) {
				tf.SetStatus(DownStatusDownloadFileNil)
			} else if errors.Is(err, ErrFileHashCheckFailed) {
				tf.Reset()
				tf.SetStatus(DownStatusDownloadFailure)
			} else {
				tf.SetStatus(DownStatusDownloadFailure)
			}
		} else {
			err = tf.SetStatus(DownStatusDownloadCompleted)
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
	stat, err := os.Stat(getCFCDownloadFile(tf.LocalPath))
	if err != nil {
		if os.IsNotExist(err) { //如果下载文件不存在，重置
			tf.Reset()
			err = tf.SetStatus(DownStatusDownloadReset)
			if err != nil {
				err = errors.New("parsing failure2 :" + err.Error())
				loger.SetLogWarn(err)
				return err
			}
			err = MakeZeroFile(getCFCDownloadFile(tf.LocalPath), int(tf.Size))
			if err != nil {
				err = errors.New("make file :" + osIsErr(err).Error())
				loger.SetLogWarn(err)
				return err
			}
		} else {
			err = errors.New("parsing failure3 :" + osIsErr(err).Error())
			loger.SetLogWarn(err)
			return err
		}
	}
	if stat.Size() != tf.Size || tough {
		if tough { //重置
			tf.Reset()
			err = tf.SetStatus(DownStatusDownloadReset)
			if err != nil {
				err = errors.New("parsing failure4 :" + err.Error())
				loger.SetLogWarn(err)
				return err
			}
			err = MakeZeroFile(getCFCDownloadFile(tf.LocalPath), int(tf.Size))
			if err != nil {
				err = errors.New("make file :" + osIsErr(err).Error())
				loger.SetLogWarn(err)
				return err
			}
		} else {
			err = errors.New("parsing failure5 :" + "file information is inconsistent")
			loger.SetLogWarn(err)
			return err
		}
	}
	var info FileDetailInfo
	err = fc.remote.taskCtx.NewTaskCbCMsg(ctool.FileInfoQ1, 200, fc.remote.SetCFMsg(tf.RemotePath)).WaitCb(10*time.Second, func(cMsg tool.ConnMsg) error {
		err1 := cMsg.CheckConnMsgHeaderAndCode(ctool.FileInfoA1, 200)
		if err1 != nil {
			return err1
		}
		_, err1 = GetCFMsg(cMsg.Data, &info)
		if err1 != nil {
			return err1
		}
		return nil
	})
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}
	if info.ModTime != tf.ModTime || info.Hash != tf.Hash || info.Size != tf.Size {
		err = ErrFileIsModified
		loger.SetLogWarn(err)
		return err
	}
	speed = float64(tf.SizeBuff * tf.GetTaskNum())

	err = tf.SetStatus(DownStatusDownloadInProgress)
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}

	//判断还有没有任务
	if tf.GetTaskNum() == 0 {
		if tf.Hash != "" {
			h, err := GetFileHash(getCFCDownloadFile(tf.LocalPath))
			if err != nil {
				loger.SetLogWarn(err)
				return err
			}
			//hash 不匹配 放弃下载文件，重置下载任务
			if h != tf.Hash {
				tf.Reset()
				os.Remove(getCFCDownloadFile(tf.LocalPath))
				err = ErrFileHashCheckFailed
				loger.SetLogWarn(err)
				return err
			}
		}
		//善后
		err = os.Rename(getCFCUploadFile(tf.LocalPath), tf.LocalPath)
		if err != nil {
			err = osIsErr(err)
			loger.SetLogWarn(err)
			return err
		}
		return nil
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
	f, err := os.OpenFile(getCFCDownloadFile(tf.LocalPath), os.O_RDWR, 0666)
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}
	defer f.Close()
	offset := tf.Get()
	req := DownFileReq{
		Path:   tf.RemotePath,
		Offset: int64(offset * tf.SizeBuff),
		Limit:  tf.SizeBuff,
	}
	tc := tool.NewTaskContext(sub, sub.NewKey(tf.Key))
	go func() {
		err := tc.ReadCMsg()
		if err != nil {
			sub.SetInfoLog(err)
			sub.Close()
		}
	}()
	task := tc.NewTaskCbCMsg(ctool.DownloadFileQ1, 200, fc.remote.SetCFMsg(req)).SetKeep(true)

	//设置暂停回调
	memory.FSetOnly(p, func() {
		stopcb = true
		task.OverTask(ErrTransmissionStop)
		sub.Close()
	})

	err = task.KeepWaitCb(10*time.Second, func(cMsg tool.ConnMsg) error {
		if cMsg.Header != ctool.DownloadFileA1 {
			return tool.ErrReqUnexpectedHeader
		}
		if cMsg.Code == 200 {
			task.OverTask(nil)
			return nil
		} else if cMsg.Code == 100 {
			var b []byte
			_, err1 := GetCFMsg(cMsg.Data, &b)
			if err1 != nil {
				return err1
			}
			_, err1 = f.WriteAt(b, int64(offset*tf.SizeBuff))
			if err1 != nil {
				return err1
			}
			offset, err1 = tf.NextSet()
			if err1 != nil {
				return err1
			}
			return nil
		} else {
			return errors.New("sub bad: " + cMsg.Data.(string))
		}
	})
	f.Close()
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}
	if tf.Hash != "" {
		h, err := GetFileHash(getCFCDownloadFile(tf.LocalPath))
		if err != nil {
			loger.SetLogWarn(err)
			return err
		}
		//hash 不匹配 放弃下载文件，重置下载任务
		if h != tf.Hash {
			tf.Reset()
			os.Remove(getCFCDownloadFile(tf.LocalPath))
			err = ErrFileHashCheckFailed
			loger.SetLogWarn(err)
			return err
		}
	}
	//善后
	err = os.Rename(getCFCDownloadFile(tf.LocalPath), tf.LocalPath)
	if err != nil {
		err = osIsErr(err)
		loger.SetLogWarn(err)
		return err
	}
	return nil
}

// --client--
