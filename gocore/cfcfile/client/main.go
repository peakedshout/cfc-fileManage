package main

//#include <stdlib.h>
//#include "./cfcfile_api2.h"
import "C"
import (
	"github.com/peakedshout/cfc-fileManage/cfile"
	"github.com/peakedshout/cfc-fileManage/cflog"
	"github.com/peakedshout/cfc-fileManage/ctool"
	"github.com/peakedshout/cfc-fileManage/memory"
	"github.com/peakedshout/go-CFC/client"
	"github.com/peakedshout/go-CFC/tool"
	"log"
	"strings"
	"time"
	"unsafe"
)

func main() {
}
func init() {
	memory.Init()
	memory.InitGF()
	cflog.Init()
}

//export InitCFC
func InitCFC(name, addr, key string) *C.char {
	log.Println("My Device is " + name + " ...")

	data := struct {
		ClientContext C.CFC_ClientContext_t
		ErrMsg        string
	}{}
	c, err := client.LinkProxyServer(strings.Clone(name), strings.Clone(addr), strings.Clone(key))
	if err != nil {
		data.ErrMsg = err.Error()
	} else {
		id := memory.Set(c)
		data.ClientContext = C.CFC_ClientContext_t(id)
	}
	return C.CString(ctool.MustMarshal(data))
}

//export Close
func Close(c C.CFC_ClientContext_t) {
	a, ok := memory.Get(int64(c))
	if !ok {
		return
	}
	a.(*client.DeviceBox).Close()
	memory.Del(int64(c))
}

//export InitRemoteFileContext
func InitRemoteFileContext(c C.CFC_ClientContext_t, name, key string) *C.char {
	a, ok := memory.Get(int64(c))
	if !ok {
		panic("memory not found CFC_ClientContext_t")
	}
	n := strings.Clone(name)
	k := strings.Clone(key)
	f := cfile.InitRemote(a.(*client.DeviceBox), n, k)
	if f.Err == "" {
		id := memory.Set(f)
		f.Id = id
	}
	cstr := C.CString(ctool.MustMarshal(f))
	return cstr
}

//export OpenRemoteFileContext
func OpenRemoteFileContext(fc C.CFC_FileContext_t, path string) *C.char {
	a, ok := memory.Get(int64(fc))
	if !ok {
		panic("memory not found CFC_FileContext_t")
	}
	f := a.(*cfile.FileContext)
	f.OpenRemote(path)
	cstr := C.CString(ctool.MustMarshal(f))
	return cstr
}

//export GetRemoteStatusFileContext
func GetRemoteStatusFileContext(fc C.CFC_FileContext_t) *C.char {
	a, ok := memory.Get(int64(fc))
	if !ok {
		panic("memory not found CFC_FileContext_t")
	}
	f := a.(*cfile.FileContext)
	resp := f.GetRemoteStatus()
	return C.CString(ctool.MustMarshal(resp))
}

//export DownRemoteFileContextToNewTask
func DownRemoteFileContextToNewTask(fc C.CFC_FileContext_t, path, out string, tough bool, sizeBuff int) *C.char {
	a, ok := memory.Get(int64(fc))
	if !ok {
		panic("memory not found CFC_FileContext_t")
	}
	f := a.(*cfile.FileContext)
	data := struct {
		ErrMsg   string
		InfoPath string
	}{}
	infoPath, err := f.NewTaskDownFile(path, out, tough, sizeBuff)
	if err != nil {
		data.ErrMsg = err.Error()
	} else {
		data.InfoPath = infoPath
	}
	return C.CString(ctool.MustMarshal(data))
}

//export DownRemoteFileContextToStartDown
func DownRemoteFileContextToStartDown(fc C.CFC_FileContext_t, path string, tough bool) *C.char {
	a, ok := memory.Get(int64(fc))
	if !ok {
		panic("memory not found CFC_FileContext_t")
	}
	f := a.(*cfile.FileContext)
	err := f.StartTaskDownFile(path, tough)
	if err != nil {
		return C.CString(ctool.MustMarshal(err.Error()))
	}
	return nil
}

//export UpRemoteFileContextToNewTask
func UpRemoteFileContextToNewTask(fc C.CFC_FileContext_t, path, out string, tough bool, sizeBuff int) *C.char {
	a, ok := memory.Get(int64(fc))
	if !ok {
		panic("memory not found CFC_FileContext_t")
	}
	f := a.(*cfile.FileContext)
	data := struct {
		ErrMsg   string
		InfoPath string
	}{}
	infoPath, err := f.NewTaskUpFile(path, out, tough, sizeBuff)
	if err != nil {
		data.ErrMsg = err.Error()
	} else {
		data.InfoPath = infoPath
	}
	return C.CString(ctool.MustMarshal(data))
}

//export UpRemoteFileContextToStartUp
func UpRemoteFileContextToStartUp(fc C.CFC_FileContext_t, path string, tough bool) *C.char {
	a, ok := memory.Get(int64(fc))
	if !ok {
		panic("memory not found CFC_FileContext_t")
	}
	f := a.(*cfile.FileContext)
	err := f.StartTaskUpFile(path, tough)
	if err != nil {
		return C.CString(ctool.MustMarshal(err.Error()))
	}
	return nil
}

//export TransmissionFileRemoteFileContextToStop
func TransmissionFileRemoteFileContextToStop(fc C.CFC_FileContext_t, path string) {
	a, ok := memory.Get(int64(fc))

	if !ok {
		panic("memory not found CFC_FileContext_t")
	}
	f := a.(*cfile.FileContext)
	f.StopTaskTransmissionFile(path)
}

//export TransmissionRemoteFileContextToGetProgress
func TransmissionRemoteFileContextToGetProgress(fc C.CFC_FileContext_t, path string) *C.char {
	a, ok := memory.Get(int64(fc))
	if !ok {
		panic("memory not found CFC_FileContext_t")
	}
	f := a.(*cfile.FileContext)
	data := struct {
		Info   cfile.TransmissionFileTaskInfoProgress
		ErrMsg string
	}{}
	info, err := f.GetTransmissionTaskProgress(path)
	if err != nil {
		data.ErrMsg = err.Error()
	} else {
		data.Info = info
	}
	return C.CString(ctool.MustMarshal(data))
}

//export CopyRemoteFileContext
func CopyRemoteFileContext(fc C.CFC_FileContext_t, oldPath, newPath string, tough bool) *C.char {
	a, ok := memory.Get(int64(fc))
	if !ok {
		panic("memory not found CFC_FileContext_t")
	}
	f := a.(*cfile.FileContext)
	err := f.CopyRemoteFileContext(oldPath, newPath, tough)
	if err != nil {
		return C.CString(ctool.MustMarshal(err.Error()))
	}
	return nil
}

//export MoveRemoteFileContext
func MoveRemoteFileContext(fc C.CFC_FileContext_t, oldPath, newPath string, tough bool) *C.char {
	a, ok := memory.Get(int64(fc))
	if !ok {
		panic("memory not found CFC_FileContext_t")
	}
	f := a.(*cfile.FileContext)
	err := f.MoveRemoteFileContext(oldPath, newPath, tough)
	if err != nil {
		return C.CString(ctool.MustMarshal(err.Error()))
	}
	return nil
}

//export RenameRemoteFileContext
func RenameRemoteFileContext(fc C.CFC_FileContext_t, oldPath, newPath string, tough bool) *C.char {
	a, ok := memory.Get(int64(fc))
	if !ok {
		panic("memory not found CFC_FileContext_t")
	}
	f := a.(*cfile.FileContext)
	err := f.RenameRemoteFileContext(oldPath, newPath, tough)
	if err != nil {
		return C.CString(ctool.MustMarshal(err.Error()))
	}
	return nil
}

//export RemoveRemoteFileContext
func RemoveRemoteFileContext(fc C.CFC_FileContext_t, path string) *C.char {
	a, ok := memory.Get(int64(fc))
	if !ok {
		panic("memory not found CFC_FileContext_t")
	}
	f := a.(*cfile.FileContext)
	err := f.RemoveRemoteFileContext(path)
	if err != nil {
		return C.CString(ctool.MustMarshal(err.Error()))
	}
	return nil
}

//export MkDirRemoteFileContext
func MkDirRemoteFileContext(fc C.CFC_FileContext_t, path string) *C.char {
	a, ok := memory.Get(int64(fc))
	if !ok {
		panic("memory not found CFC_FileContext_t")
	}
	f := a.(*cfile.FileContext)
	err := f.MkDirRemoteFileContext(path)
	if err != nil {
		return C.CString(ctool.MustMarshal(err.Error()))
	}
	return nil
}

//export ReadRemoteFileContext
func ReadRemoteFileContext(fc C.CFC_FileContext_t, path string, offset int64, limit int, timeout int64) *C.char {
	a, ok := memory.Get(int64(fc))
	if !ok {
		panic("memory not found CFC_FileContext_t")
	}
	f := a.(*cfile.FileContext)
	data := struct {
		Data   string
		ErrMsg string
	}{}
	b, err := f.ReadRemoteFileContext(cfile.ReadSomeReq{Path: path, Offset: offset, Limit: limit}, time.Duration(timeout)*time.Second)
	if err != nil {
		data.ErrMsg = err.Error()
	} else {
		data.Data = tool.MustBytesToBase64(b).Data
	}
	return C.CString(ctool.MustMarshal(data))
}

//export ScanCFCInfoRemoteFileContext
func ScanCFCInfoRemoteFileContext(fc C.CFC_FileContext_t, localPath string, ignoreErr bool) *C.char {
	a, ok := memory.Get(int64(fc))
	if !ok {
		panic("memory not found CFC_FileContext_t")
	}
	f := a.(*cfile.FileContext)
	data := struct {
		Data   []cfile.ScanCFCInfoResp
		ErrMsg string
	}{}
	resp, err := f.ScanAllCFCInfo(localPath, ignoreErr)
	if err != nil {
		data.ErrMsg = err.Error()
	} else {
		data.Data = resp
	}
	return C.CString(ctool.MustMarshal(data))
}

//export InitLocalFileContext
func InitLocalFileContext() *C.char {
	config := &cfile.ConfigInfo{
		DeviceType:     cfile.DeviceTypeClient,
		PermissionList: cfile.PermissionToDefaultClient,
	}
	f := cfile.InitLocal(config)
	if f.Err == "" {
		id := memory.Set(f)
		f.Id = id
	}
	cstr := C.CString(ctool.MustMarshal(f))
	return cstr
}

//export OpenLocalFileContext
func OpenLocalFileContext(fc C.CFC_FileContext_t, path string) *C.char {
	a, ok := memory.Get(int64(fc))
	if !ok {
		return nil
	}
	f := a.(*cfile.FileContext)
	f.OpenLocal(strings.Clone(path))
	cstr := C.CString(ctool.MustMarshal(f))
	return cstr
}

//export CopyLocalFileContext
func CopyLocalFileContext(fc C.CFC_FileContext_t, oldPath, newPath string, tough bool) *C.char {
	a, ok := memory.Get(int64(fc))
	if !ok {
		panic("memory not found CFC_FileContext_t")
	}
	f := a.(*cfile.FileContext)
	err := f.CopyLocalFileContext(oldPath, newPath, tough)
	if err != nil {
		return C.CString(ctool.MustMarshal(err.Error()))
	}
	return nil
}

//export MoveLocalFileContext
func MoveLocalFileContext(fc C.CFC_FileContext_t, oldPath, newPath string, tough bool) *C.char {
	a, ok := memory.Get(int64(fc))
	if !ok {
		panic("memory not found CFC_FileContext_t")
	}
	f := a.(*cfile.FileContext)
	err := f.MoveLocalFileContext(oldPath, newPath, tough)
	if err != nil {
		return C.CString(ctool.MustMarshal(err.Error()))
	}
	return nil
}

//export RenameLocalFileContext
func RenameLocalFileContext(fc C.CFC_FileContext_t, oldPath, newPath string, tough bool) *C.char {
	a, ok := memory.Get(int64(fc))
	if !ok {
		panic("memory not found CFC_FileContext_t")
	}
	f := a.(*cfile.FileContext)
	err := f.RenameLocalFileContext(oldPath, newPath, tough)
	if err != nil {
		return C.CString(ctool.MustMarshal(err.Error()))
	}
	return nil
}

//export RemoveLocalFileContext
func RemoveLocalFileContext(fc C.CFC_FileContext_t, path string) *C.char {
	a, ok := memory.Get(int64(fc))
	if !ok {
		panic("memory not found CFC_FileContext_t")
	}
	f := a.(*cfile.FileContext)
	err := f.RemoveLocalFileContext(path)
	if err != nil {
		return C.CString(ctool.MustMarshal(err.Error()))
	}
	return nil
}

//export MkDirLocalFileContext
func MkDirLocalFileContext(fc C.CFC_FileContext_t, path string) *C.char {
	a, ok := memory.Get(int64(fc))
	if !ok {
		panic("memory not found CFC_FileContext_t")
	}
	f := a.(*cfile.FileContext)
	err := f.MkDirLocalFileContext(path)
	if err != nil {
		return C.CString(ctool.MustMarshal(err.Error()))
	}
	return nil
}

//export ReadLocalFileContext
func ReadLocalFileContext(fc C.CFC_FileContext_t, path string, offset int64, limit int) *C.char {
	a, ok := memory.Get(int64(fc))
	if !ok {
		panic("memory not found CFC_FileContext_t")
	}
	f := a.(*cfile.FileContext)
	data := struct {
		Data   string
		ErrMsg string
	}{}
	b, err := f.ReadLocalFileContext(cfile.ReadSomeReq{Path: path, Offset: offset, Limit: limit}, true)
	if err != nil {
		data.ErrMsg = err.Error()
	} else {
		data.Data = tool.MustBytesToBase64(b).Data
	}
	return C.CString(ctool.MustMarshal(data))
}

//export CloseFileContext
func CloseFileContext(fc C.CFC_FileContext_t) {
	a, ok := memory.Get(int64(fc))
	if !ok {
		return
	}
	f := a.(*cfile.FileContext)
	f.Close()
	memory.Del(int64(fc))
}

//export FreeString
func FreeString(c *C.char) {
	C.free(unsafe.Pointer(c))
}

//export LogListen
func LogListen(port int) *C.char {
	err := cflog.Listen(port)
	if err != nil {
		return C.CString(ctool.MustMarshal(err))
	}
	return nil
}

//export LogOpt
func LogOpt(level uint8, stack bool) {
	cflog.Opt(level, stack)
}
