package cfile

import (
	"encoding/json"
	"errors"
	"fmt"
	"github.com/peakedshout/cfc-fileManage/memory"
	"github.com/peakedshout/go-CFC/loger"
	"github.com/peakedshout/go-CFC/tool"
	"io"
	"io/fs"
	"math"
	"os"
	"path/filepath"
	"sync"
	"time"
)

func NewTransmissionFileTaskInfo(info FileDetailInfo, req NewTransmissionFileTaskInfoReq) error {
	if req.SizeBuff == 0 {
		if req.IsUp {
			req.SizeBuff = upFileSizeBuff
		} else {
			req.SizeBuff = downFileSizeBuff
		}
	}
	status := DownStatusDownloadReset
	t := TransmissionTypeDown
	if req.IsUp {
		status = UpStatusUploadReset
		t = TransmissionTypeUp
	}
	num := int(math.Ceil(float64(info.Size) / float64(req.SizeBuff)))
	tf := TransmissionFileTaskInfo{
		FileName:   info.FileName,
		Size:       info.Size,
		ModTime:    info.ModTime,
		ServerName: req.ServerName,
		Key:        req.Key,
		RemotePath: req.RemotePath,
		LocalPath:  req.LocalPath,
		Hash:       info.Hash,
		Status:     status,
		Type:       t,
		Num:        num,
		SizeBuff:   req.SizeBuff,
		TaskOffset: 0,
		lock:       sync.Mutex{},
	}
	return tf.Set()
}

func (tf *TransmissionFileTaskInfo) getType() (isUp bool, err error) {
	if tf.Type == TransmissionTypeUp {
		return true, nil
	} else if tf.Type == TransmissionTypeDown {
		return false, nil
	} else {
		return false, ErrTransmissionType
	}
}
func (tf *TransmissionFileTaskInfo) SetStatus(status string) error {
	tf.lock.Lock()
	defer tf.lock.Unlock()
	tf.Status = status
	return tf.SetNoLock()
}
func (tf *TransmissionFileTaskInfo) Reset() {
	tf.lock.Lock()
	defer tf.lock.Unlock()
	tf.TaskOffset = 0
}
func (tf *TransmissionFileTaskInfo) Get() int {
	tf.lock.Lock()
	defer tf.lock.Unlock()
	return tf.TaskOffset
}
func (tf *TransmissionFileTaskInfo) GetTaskNum() int {
	tf.lock.Lock()
	defer tf.lock.Unlock()
	return tf.Num - tf.TaskOffset
}
func (tf *TransmissionFileTaskInfo) Next() int {
	tf.lock.Lock()
	defer tf.lock.Unlock()
	tf.TaskOffset++
	return tf.TaskOffset
}
func (tf *TransmissionFileTaskInfo) NextSet() (int, error) {
	tf.lock.Lock()
	defer tf.lock.Unlock()
	tf.TaskOffset++
	return tf.TaskOffset, tf.SetNoLock()
}
func (tf *TransmissionFileTaskInfo) Set() error {
	tf.lock.Lock()
	return tf.SetNoLock()
}
func (tf *TransmissionFileTaskInfo) SetNoLock() error {
	isUp, err := tf.getType()
	if err != nil {
		return err
	}
	p := ""
	if isUp {
		p = getCFCUploadInfo(tf.LocalPath)
	} else {
		p = getCFCDownloadInfo(tf.LocalPath)
	}
	f, err := os.Create(p)
	if err != nil {
		return osIsErr(err)
	}
	defer f.Close()
	b, err := tool.Encrypt(tool.MustMarshal(tf), []byte(tf.Key))
	if err != nil {
		return err
	}
	_, err = f.Write(b)
	if err != nil {
		return osIsErr(err)
	}
	return nil
}
func (tf *TransmissionFileTaskInfo) VerifyOffset(offset int) error {
	tf.lock.Lock()
	defer tf.lock.Unlock()
	if offset == tf.TaskOffset {
		tf.TaskOffset++
		return tf.SetNoLock()
	} else {
		return errors.New("upload offset is inconsistency")
	}
}
func ReadTransmissionFileTaskInfo(p string, key string) (*TransmissionFileTaskInfo, error) {
	var info TransmissionFileTaskInfo
	b, err := os.ReadFile(p)
	if err != nil {
		return nil, osIsErr(err)
	}
	b2, err := tool.Decrypt(b, []byte(key))
	if err != nil {
		return nil, err
	}
	err = json.Unmarshal(b2, &info)
	if err != nil {
		return nil, err
	}
	return &info, nil
}

func getCFCUploadInfo(p string) string {
	return p + ".CFCUpload_info"
}
func getCFCUploadFile(p string) string {
	return p + ".CFCUpload_file"
}

func getCFCDownloadInfo(p string) string {
	return p + ".CFCDownload_info"
}
func getCFCDownloadFile(p string) string {
	return p + ".CFCDownload_file"
}

func (fc *FileContext) ReadFileToSend(p string, offset int64, limit int) (<-chan []byte, chan<- uint8, <-chan error) {
	data := make(chan []byte)
	stop := make(chan uint8, 1)
	errChan := make(chan error)
	go func() {
		p, err := fc.CleanPath(p, false)
		if err != nil {
			errChan <- err
			return
		}
		f, err := os.Open(p)
		if err != nil {
			errChan <- osIsErr(err)
			return
		}
		defer f.Close()
		_, err = f.Seek(offset, 0)
		if err != nil {
			errChan <- osIsErr(err)
			return
		}
		for true {
			select {
			case <-stop:
				return
			default:
				buf := make([]byte, limit)
				n, err := f.Read(buf)
				if err != nil {
					if err == io.EOF {
						close(data)
						return
					}
					errChan <- osIsErr(err)
					return
				}
				if n == 0 {
					close(data)
					return
				}
				data <- buf[:n]
			}
		}
	}()
	return data, stop, errChan
}

// 停止传输
func (fc *FileContext) StopTaskTransmissionFile(p string) {
	p, _ = fc.CleanPath(p, false)
	memory.FGetRunDel(p)
}

// 获取传输进度
func (fc *FileContext) GetTransmissionTaskProgress(path string) (info TransmissionFileTaskInfoProgress, err error) {
	tf, err := ReadTransmissionFileTaskInfo(path, fc.remote.key.GetRawKey())
	if err != nil {
		loger.SetLogWarn(err)
		return info, err
	}
	if tf.ServerName != fc.remote.odjName {
		err = ErrServerNameIsInconsistency
		loger.SetLogWarn(err)
		return info, err
	}
	info = TransmissionFileTaskInfoProgress{
		FileName:     tf.FileName,
		Type:         tf.Type,
		Progress:     tf.TaskOffset,
		Total:        tf.Num,
		Status:       tf.Status,
		NetworkSpeed: fc.GetTransmissionSpeed(path),
	}
	if info.Type == TransmissionTypeDown {
		info.From = tf.RemotePath
		info.To = tf.LocalPath
	} else {
		info.To = tf.RemotePath
		info.From = tf.LocalPath
	}
	return info, nil
}

// 只用与本地扫描，不掩盖err
func (fc *FileContext) ScanAllCFCInfo(p string, ignoreErr bool) (resp []ScanCFCInfoResp, err error) {
	if fc.remote == nil || fc.remote.cc == nil {
		err = ErrFcNotRemote
		loger.SetLogWarn(err)
		return nil, err
	}
	t := time.Now()
	n, suc, dis := 0, 0, 0
	defer func() {
		fmt.Println("ScanCFCInfo used:", time.Now().Sub(t).String(), "call:", n, "success:", suc, "discover:", dis)
	}()
	err = filepath.WalkDir(p, func(path string, d fs.DirEntry, err error) error {
		n++
		if !ignoreErr && err != nil {
			return err
		}
		if !d.IsDir() && (filepath.Ext(path) == ".CFCDownload_info" || filepath.Ext(path) == ".CFCUpload_info") {
			dis++
			var info ScanCFCInfoResp
			df, err := ReadTransmissionFileTaskInfo(path, fc.remote.key.GetRawKey())
			if err != nil {
				info.ErrMsg = err.Error()
			} else if df.ServerName != fc.remote.odjName {
				info.ErrMsg = ErrServerNameIsInconsistency.Error()
			} else {
				if filepath.Ext(path) == ".CFCDownload_info" {
					info.DownInfoPath = path
				} else {
					info.UpInfoPath = path
				}
			}
			suc++
			resp = append(resp, info)
		}
		return nil
	})
	if err != nil {
		loger.SetLogWarn(err)
	}
	return resp, err
}
