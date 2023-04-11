package cfile

import (
	"github.com/peakedshout/go-CFC/client"
	"github.com/peakedshout/go-CFC/tool"
	"sync"
)

// -基本-

type FileContext struct {
	Id       int64 //CFC_FileContext_t
	Current  string
	remote   *RemoteInfo
	Config   *ConfigInfo
	Err      string
	FileList []FileInfo
}

type ConfigInfo struct {
	DeviceName     string //local is nil
	DeviceType     string
	Root           string `json:"-"` //local is nil
	RootSize       int64
	MaxRootSize    int64
	RootSizeErr    string //root计算错误时会有
	rootSizeLock   sync.Mutex
	UserName       string
	RawKey         string `json:"-"`
	PermissionList []string
}

type RemoteInfo struct {
	odjName  string //local is nil
	userName string //local is nil
	key      tool.Key
	rid      string
	sub      *client.SubBox
	cc       *client.DeviceBox
	stop     chan uint8

	subMap  map[string]map[int]*client.SubBox
	subLock sync.Mutex

	onceCloser sync.Once

	taskCtx *tool.TaskCbContext
}

// remote cb
type readCbContext struct {
	fn   func(tool.ConnMsg)
	wait chan error
	stop chan struct{}
}

// -基本-

// -状态-

type RemoteStatus struct {
	RootSize        int64
	MaxRootSize     int64
	RootSizeShow    string
	MaxRootSizeShow string
	RootSizeErr     string //root计算错误时会有
	DelayRaw        int64
	Delay           string
	DelayErr        string //延迟错误
	NetworkSpeed    tool.NetworkSpeedView
}

type RootSizeInfo struct {
	RootSize    int64
	MaxRootSize int64
	RootSizeErr string
}

// -状态-

// -传输-

type UpFileDetailInfo struct {
	FilePath string
	Tough    bool
	Size     int64
	Check    bool //检查
}
type UploadFileDataReq struct {
	FilePath string
	Size     int64
	Hash     string
	Offset   int
	SizeBuff int
	Data     []byte
}

type DownFileReq struct {
	Path   string
	Offset int64
	Limit  int
}

type ReadSomeReq struct {
	Path   string
	Offset int64
	Limit  int
}

type NewTransmissionFileTaskInfoReq struct {
	RemotePath string
	LocalPath  string
	ServerName string
	UserName   string
	Key        string
	SizeBuff   int
	IsUp       bool
}

type TransmissionFileTaskInfoProgress struct {
	FileName     string //只显示文件名
	Type         string //TransmissionTypeUp / TransmissionTypeDown
	From         string //来源
	To           string //目标
	Progress     int    //进度
	Total        int    //总
	Status       string //状态
	NetworkSpeed tool.NetworkSpeedView
}
type TransmissionFileTaskInfo struct {
	FileName   string
	Size       int64
	ModTime    string
	ServerName string
	UserName   string
	Key        string
	RemotePath string
	LocalPath  string
	Hash       string
	Status     string
	Type       string //TransmissionTypeUp / TransmissionTypeDown
	Num        int    //总数
	SizeBuff   int    //块大小
	TaskOffset int    //进度偏移
	lock       sync.Mutex
}

type ScanCFCInfoResp struct {
	DownInfoPath string
	UpInfoPath   string
	ErrMsg       string
}

// -传输-

// -操作-

type ONOpReq struct {
	Old   string
	New   string
	Tough bool
}

// -操作-

// -文件-

type FileDetailInfo struct {
	FileName string
	Size     int64
	ModTime  string
	Hash     string
}

type FileInfo struct {
	Name    string
	Size    string
	RawSize int64
	Mode    string
	ModTime string
	IsDir   bool
	//Sys     any
}

// -文件-

// -消息-

type CFCFileCMsg struct {
	Id   string
	Data any
}

// -消息-

// -登陆-
type CFCLoginReq struct {
	LoginName string
	LoginKey  string
}

// -登陆-
