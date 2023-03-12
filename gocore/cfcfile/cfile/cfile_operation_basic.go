package cfile

import (
	"errors"
	"github.com/peakedshout/cfc-fileManage/ctool"
	"github.com/peakedshout/go-CFC/loger"
	"github.com/peakedshout/go-CFC/tool"
	"io/fs"
	"log"
	"os"
	"path"
	"path/filepath"
	"time"
)

// --local--

// 复制本地
func (fc *FileContext) CopyLocalFileContext(o, n string, tough bool) error {
	err := fc.CheckPermission([]string{PermissionLink, PermissionOpen, PermissionCopy})
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}
	from, err := fc.CleanPath(o, false)
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}
	to, err := fc.CleanPath(n, false)
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}
	f, err := os.Stat(from)
	if err != nil {
		err = osIsErr(err)
		loger.SetLogWarn(err)
		return err
	}

	err = fc.checkRootSizeIsRangeOutExSize(f.Size())
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}

	if !tough && checkLocalFileIsExist(to) {
		err = errors.New("new path is exit :" + to)
		loger.SetLogWarn(err)
		return err
	}

	log.Println("copy...", from, "->", to)

	//复制
	if f.IsDir() {
		return filepath.WalkDir(from, func(path string, d fs.DirEntry, err error) error {
			if !d.IsDir() {
				return CopyFileOrDir(path, from, to)
			} else {
				if err = os.MkdirAll(path, 0777); err != nil {
					return osIsErr(err)
				}
			}
			if err != nil {
				err = osIsErr(err)
				loger.SetLogWarn(err)
			}
			return err
		})
	} else {
		err = CopyFileOrDir(from, from, to)
		if err != nil {
			err = osIsErr(err)
			loger.SetLogWarn(err)
		}
		return err
	}
}

// 剪切本地
func (fc *FileContext) MoveLocalFileContext(o, n string, tough bool) error {
	err := fc.CheckPermission([]string{PermissionLink, PermissionOpen, PermissionMove})
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}
	from, err := fc.CleanPath(o, false)
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}
	to, err := fc.CleanPath(n, false)
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}
	f, err := os.Stat(from)
	if err != nil {
		err = osIsErr(err)
		loger.SetLogWarn(err)
		return err
	}

	err = fc.checkRootSizeIsRangeOutExSize(0)
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}

	if !tough && checkLocalFileIsExist(to) {
		err = errors.New("new path is exit :" + to)
		loger.SetLogWarn(err)
		return err
	}
	log.Println("move...", from, "->", to)

	defer func() {
		if err == nil {
			err = os.RemoveAll(from)
			if err != nil {
				err = osIsErr(err)
				loger.SetLogWarn(err)
			}
		}
	}()
	//剪切
	if f.IsDir() {
		return filepath.WalkDir(from, func(path string, d fs.DirEntry, err error) error {
			if !d.IsDir() {
				return MoveFileOrDir(path, from, to)
			} else {
				if err = os.MkdirAll(path, 0777); err != nil {
					return osIsErr(err)
				}
			}
			if err != nil {
				err = osIsErr(err)
				loger.SetLogWarn(err)
			}
			return err
		})
	} else {
		err = MoveFileOrDir(from, from, to)
		if err != nil {
			err = osIsErr(err)
			loger.SetLogWarn(err)
		}
		return err
	}
}

// 重命名本地
func (fc *FileContext) RenameLocalFileContext(o, n string, tough bool) error {
	err := fc.CheckPermission([]string{PermissionLink, PermissionOpen, PermissionRename})
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}
	from, err := fc.CleanPath(o, false)
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}

	f, err := os.Stat(from)
	if err != nil {
		err = osIsErr(err)
		loger.SetLogWarn(err)
		return err
	}

	dir := path.Dir(from)
	name := path.Base(n)

	to := path.Join(dir, name)

	err = fc.checkRootSizeIsRangeOutExSize(0)
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}

	if !tough && checkLocalFileIsExist(to) {
		err = errors.New("new path is exit :" + to)
		loger.SetLogWarn(err)
		return err
	}
	log.Println("move...", from, "->", to)

	defer func() {
		if err == nil {
			err = os.RemoveAll(from)
			if err != nil {
				err = osIsErr(err)
				loger.SetLogWarn(err)
			}
		}
	}()
	//剪切
	if f.IsDir() {
		return filepath.WalkDir(from, func(path string, d fs.DirEntry, err error) error {
			if !d.IsDir() {
				return MoveFileOrDir(path, from, to)
			} else {
				if err = os.MkdirAll(path, 0777); err != nil {
					return osIsErr(err)
				}
			}
			if err != nil {
				err = osIsErr(err)
				loger.SetLogWarn(err)
			}
			return err
		})
	} else {
		err = MoveFileOrDir(from, from, to)
		if err != nil {
			err = osIsErr(err)
			loger.SetLogWarn(err)
		}
		return err
	}
}

// 删除本地
func (fc *FileContext) RemoveLocalFileContext(p string) error {
	err := fc.CheckPermission([]string{PermissionLink, PermissionOpen, PermissionRemove})
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}
	gopath, err := fc.CleanPath(p, false)
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}
	err = osIsErr(os.RemoveAll(gopath))
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}
	return nil
}

// 创建本地文件夹
func (fc *FileContext) MkDirLocalFileContext(p string) error {
	err := fc.CheckPermission([]string{PermissionLink, PermissionOpen, PermissionMkDir})
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}
	gopath, err := fc.CleanPath(p, false)
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}
	err = fc.checkRootSizeIsRangeOutExSize(0)
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}
	err = osIsErr(os.MkdirAll(gopath, 0777))
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}
	return nil
}

// --local--

// --remote--

// 复制远程
func (fc *FileContext) CopyRemoteFileContext(o, n string, tough bool) (err error) {
	if fc.remote == nil || fc.remote.sub == nil {
		err = ErrFcNotRemote
		loger.SetLogWarn(err)
		return err
	}
	err = fc.remote.taskCtx.NewTaskCbCMsg(ctool.CopyFileQ1, 200, fc.remote.SetCFMsg(ONOpReq{
		Old:   o,
		New:   n,
		Tough: tough,
	})).WaitCb(10*time.Second, func(cMsg tool.ConnMsg) error {
		return cMsg.CheckConnMsgHeaderAndCode(ctool.CopyFileA1, 200)
	})
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}
	return nil
}

// 剪切远程
func (fc *FileContext) MoveRemoteFileContext(o, n string, tough bool) (err error) {
	if fc.remote == nil || fc.remote.sub == nil {
		err = ErrFcNotRemote
		loger.SetLogWarn(err)
		return err
	}
	err = fc.remote.taskCtx.NewTaskCbCMsg(ctool.MoveFileQ1, 200, fc.remote.SetCFMsg(ONOpReq{
		Old:   o,
		New:   n,
		Tough: tough,
	})).WaitCb(10*time.Second, func(cMsg tool.ConnMsg) error {
		return cMsg.CheckConnMsgHeaderAndCode(ctool.MoveFileA1, 200)
	})
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}
	return nil
}

// 重命名远程
func (fc *FileContext) RenameRemoteFileContext(o, n string, tough bool) (err error) {
	if fc.remote == nil || fc.remote.sub == nil {
		err = ErrFcNotRemote
		loger.SetLogWarn(err)
		return err
	}
	err = fc.remote.taskCtx.NewTaskCbCMsg(ctool.RenameFileQ1, 200, fc.remote.SetCFMsg(ONOpReq{
		Old:   o,
		New:   n,
		Tough: tough,
	})).WaitCb(10*time.Second, func(cMsg tool.ConnMsg) error {
		return cMsg.CheckConnMsgHeaderAndCode(ctool.RenameFileA1, 200)
	})
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}
	return nil
}

// 删除远程
func (fc *FileContext) RemoveRemoteFileContext(p string) (err error) {
	if fc.remote == nil || fc.remote.sub == nil {
		err = ErrFcNotRemote
		loger.SetLogWarn(err)
		return err
	}
	err = fc.remote.taskCtx.NewTaskCbCMsg(ctool.RemoveFileQ1, 200, fc.remote.SetCFMsg(p)).WaitCb(10*time.Second, func(cMsg tool.ConnMsg) error {
		return cMsg.CheckConnMsgHeaderAndCode(ctool.RemoveFileA1, 200)
	})
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}
	return err
}

// 创建远程文件夹
func (fc *FileContext) MkDirRemoteFileContext(p string) (err error) {
	if fc.remote == nil || fc.remote.sub == nil {
		err = ErrFcNotRemote
		loger.SetLogWarn(err)
		return err
	}
	err = fc.remote.taskCtx.NewTaskCbCMsg(ctool.MkDirFileQ1, 200, fc.remote.SetCFMsg(p)).WaitCb(10*time.Second, func(cMsg tool.ConnMsg) error {
		return cMsg.CheckConnMsgHeaderAndCode(ctool.MkDirFileA1, 200)
	})
	if err != nil {
		loger.SetLogWarn(err)
		return err
	}
	return nil
}

// --remote--
