package ctool

import (
	"encoding/json"
	"github.com/peakedshout/go-CFC/tool"
)

const (
	InitFileQ1 = "InitFileQ1"
	InitFileA1 = "InitFileA1"
	OpenFileQ1 = "OpenFileQ1"
	OpenFileA1 = "OpenFileA1"

	CopyFileQ1   = "CopyFileQ1"
	CopyFileA1   = "CopyFileA1"
	MoveFileQ1   = "MoveFileQ1"
	MoveFileA1   = "MoveFileA1"
	RenameFileQ1 = "RenameFileQ1"
	RenameFileA1 = "RenameFileA1"
	RemoveFileQ1 = "RemoveFileQ1"
	RemoveFileA1 = "RemoveFileA1"
	MkDirFileQ1  = "MkDirFileQ1"
	MkDirFileA1  = "MkDirFileA1"

	FileInfoQ1 = "FileInfoQ1"
	FileInfoA1 = "FileInfoA1"

	ReadFileQ1 = "ReadFileQ1"
	ReadFileA1 = "ReadFileA1"

	DownloadFileQ1 = "DownloadFileQ1"
	DownloadFileA1 = "DownloadFileA1"

	UploadFileQ1 = "UploadFileQ1"
	UploadFileA1 = "UploadFileA1"
	UploadFileQ2 = "UploadFileQ2"
	UploadFileA2 = "UploadFileA2"

	StatusQ1 = "StatusQ1"
	StatusA1 = "StatusA1"

	Ping = "Ping"
	Pong = "Pong"
)

type DefaultMsg struct {
	Msg string
	Err string
}

func MustMarshal(a any) string {
	b, err := json.Marshal(a)
	if err != nil {
		panic(err)
	}
	return string(b)
}
func MustSetMsg(msg, err string) string {
	return MustMarshal(DefaultMsg{
		Msg: msg,
		Err: err,
	})
}

type MultiDownTask struct {
	Id int
	//Num  int
	Data string
}

type MultiDownInfo struct {
	Id  string
	Num int //总数
}

func SetMultiDownTask(id int, b []byte) *MultiDownTask {
	return &MultiDownTask{
		Id: id,
		//Num:  num,
		Data: tool.MustBytesToBase64(b).Data,
	}
}
