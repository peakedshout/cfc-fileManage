| 对象备注               | 说明                  |
|--------------------|---------------------|
| ClientContext      | 最基本的实现连接对象，是远程连接的基础 |
| FileContext_Remote | 远程的文件处理对象           |
| FileContext_Local  | 本地的文件处理对象           |

| 权限值                | 功能         |
|--------------------|------------|
| PermissionLink     | 许可连接//基本   |
| PermissionOpen     | 许可访问文件//基本 |
| PermissionUpload   | 许可上传       |
| PermissionCopy     | 许可复制       |
| PermissionMove     | 许可剪切       |
| PermissionRename   | 许可重命名      |
| PermissionMkDir    | 许可创建文件夹    |
| PermissionRemove   | 许可删除       |
| PermissionDownload | 许可下载       |
| PermissionRead     | 许可读取       |

| k                            | v       |
|------------------------------|---------|
| TransmissionTypeUp           | 上传      |
| TransmissionTypeDown         | 下载      |
| UpStatusUploadReset          | 上传重置    |
| UpStatusUploadInProgress     | 上传进行中   |
| UpStatusUploadCompleted      | 上传完成    |
| UpStatusUploadFailure        | 上传失败    |
| UpStatusUploadPause          | 上传暂停    |
| UpStatusUploadFileNil        | 上传文件不存在 |
| DownStatusDownloadReset      | 下载重置    |
| DownStatusDownloadInProgress | 下载进行中   |
| DownStatusDownloadCompleted  | 下载完成    |
| DownStatusDownloadFailure    | 下载失败    |
| DownStatusDownloadPause      | 下载暂停    |
| DownStatusDownloadFileNil    | 下载文件不存在 |


| 方法                                         | 对象备注                                 | 权限                                                 | 参数                                                                | 输出                               | 版本  | 功能                                |
|--------------------------------------------|--------------------------------------|----------------------------------------------------|-------------------------------------------------------------------|----------------------------------|-----|-----------------------------------|
| InitCFC                                    |                                      |                                                    | name, ip, port, key string                                        | ClientContext,ErrMsg             | 内测  | 与中转服务器建立连接，采用cfc协议通信              |
| Close                                      | ClientContext                        |                                                    | c ClientContext                                                   |                                  | 内测  | 关闭与中转服务器主连接，会断开所有该连接的子连接          |
| InitRemoteFileContext                      | ClientContext                        | PermissionLink, PermissionOpen                     | c ClientContext, name, key string                                 | FileContext                      | 内测  | rpc->远程建立一个本地操作文件的对象，会得到一些远程的基本信息 |
| OpenRemoteFileContext                      | FileContext_Remote                   | PermissionLink, PermissionOpen                     | fc FileContext_Remote, path string                                | FileContext                      | 内测  | rpc->远程打开路径，并返回和当前路径路径列表（root隐藏）  |
| GetRemoteStatusFileContext                 | FileContext_Remote                   | PermissionLink, PermissionOpen                     | fc FileContext_Remote                                             | RemoteStatus                     | 内测  | rpc->获取远程的一些状态：存储、延迟、网速           |
| DownRemoteFileContextToNewTask             | FileContext_Remote                   | PermissionLink, PermissionOpen, PermissionDownload | fc FileContext_Remote, path, out string, tough bool, sizeBuff int | InfoPath,  ErrMsg                | 内测  | rpc->新建下载任务文件                     |
| DownRemoteFileContextToStartDown           | FileContext_Remote                   | PermissionLink, PermissionOpen, PermissionDownload | fc FileContext_Remote, path string, tough bool                    | ErrMsg                           | 内测  | rpc->选中下载任务文件，执行下载                |
| UpRemoteFileContextToNewTask               | FileContext_Remote                   | PermissionLink, PermissionOpen, PermissionUpload   | fc FileContext_Remote, path, out string, tough bool, sizeBuff int | InfoPath,  ErrMsg                | 内测  | rpc->新建上传任务文件                     |
| UpRemoteFileContextToStartUp               | FileContext_Remote                   | PermissionLink, PermissionOpen, PermissionUpload   | fc FileContext_Remote, path string, tough bool                    | ErrMsg                           | 内测  | rpc->选中上传任务文件，执行下载                |
| TransmissionFileRemoteFileContextToStop    | FileContext_Remote                   |                                                    | fc FileContext_Remote, path string                                |                                  | 内测  | rpc->选中下载任务文件，停止下载，不会反馈是否真的关闭与否   |
| TransmissionRemoteFileContextToGetProgress | FileContext_Remote                   |                                                    | fc FileContext_Remote, path string                                | TransmissionFileTaskInfoProgress | 内测  | rpc->选中下载任务文件，解析得到下载进度            |
| CopyRemoteFileContext                      | FileContext_Remote                   | PermissionLink, PermissionOpen, PermissionCopy     | fc FileContext_Remote, oldPath, newPath string, tough bool        | ErrMsg                           | 内测  | rpc->远程复制（文件、文件夹）                 |
| MoveRemoteFileContext                      | FileContext_Remote                   | PermissionLink, PermissionOpen, PermissionMove     | fc FileContext_Remote, oldPath, newPath string, tough bool        | ErrMsg                           | 内测  | rpc->远程剪切（文件、文件夹）                 |
| RenameRemoteFileContext                    | FileContext_Remote                   | PermissionLink, PermissionOpen, PermissionRename   | fc FileContext_Remote, oldPath, newPath string, tough bool        | ErrMsg                           | 内测  | rpc->远程重命名（文件、文件夹）                |
| RemoveRemoteFileContext                    | FileContext_Remote                   | PermissionLink, PermissionOpen, PermissionRemove   | fc FileContext_Remote, path string                                | ErrMsg                           | 内测  | rpc->远程删除（会尽可能的删除路径里的内容）          |
| MkDirRemoteFileContext                     | FileContext_Remote                   | PermissionLink, PermissionOpen, PermissionMkDir    | fc FileContext_Remote, path string                                | ErrMsg                           | 内测  | rpc->远程创建文件夹                      |
| ReadRemoteFileContext                      | FileContext_Remote                   | PermissionLink, PermissionOpen, PermissionRead     | fc FileContext_Remote, path string, offset int64, limit int       | Data,  ErrMsg                    | 内测  | rpc->远程读取文件                       |
| ScanCFCInfoRemoteFileContext               | FileContext_Remote                   |                                                    | fc FileContext_Remote, localPath string, ignoreErr bool           | Data ScanCFCInfoResp , ErrMsg    | 内测  | 扫描路径下的下载任务文件，将结果反馈                |
| InitLocalFileContext                       |                                      | PermissionLink, PermissionOpen                     |                                                                   | FileContext                      | 内测  | loc->本地建立一个操作文件的对象                |
| OpenLocalFileContext                       | FileContext_Local                    | PermissionLink, PermissionOpen                     | fc FileContext_Local, path string                                 | FileContext                      | 内测  | loc->本地打开，并返回和当前路径路径列表            |
| CopyLocalFileContext                       | FileContext_Local                    | PermissionLink, PermissionOpen, PermissionCopy     | fc FileContext_Local, oldPath, newPath string, tough bool         | ErrMsg                           | 内测  | loc->本地复制（文件、文件夹）                 |
| MoveLocalFileContext                       | FileContext_Local                    | PermissionLink, PermissionOpen, PermissionMove     | fc FileContext_Local, oldPath, newPath string, tough bool         | ErrMsg                           | 内测  | loc->本地剪切（文件、文件夹）                 |
| RenameLocalFileContext                     | FileContext_Local                    | PermissionLink, PermissionOpen, PermissionRename   | fc FileContext_Local, oldPath, newPath string, tough bool         | ErrMsg                           | 内测  | loc->本地重命名（文件、文件夹）                |
| RemoveLocalFileContext                     | FileContext_Local                    | PermissionLink, PermissionOpen, PermissionRemove   | fc FileContext_Local, path string                                 | ErrMsg                           | 内测  | loc->本地删除（会尽可能的删除路径里的内容）          |
| MkDirLocalFileContext                      | FileContext_Local                    | PermissionLink, PermissionOpen, PermissionMkDir    | fc FileContext_Local, path string                                 | ErrMsg                           | 内测  | loc->本地创建文件夹                      |
| ReadLocalFileContext                       | FileContext_Local                    | PermissionLink, PermissionOpen, PermissionRead     | fc FileContext_Local, path string, offset int64, limit int        | Data,  ErrMsg                    | 内测  | loc->本地读取文件                       |
| CloseFileContext                           | FileContext_Remote/FileContext_Local |                                                    | fc FileContext_Remote/FileContext_Local                           |                                  | 内测  | 关闭操作文件对象                          |
| FreeString                                 |                                      |                                                    | string                                                            |                                  | 内测  | 释放指针                              |

- struct view
  - FileContext 文件对象
````
type FileContext struct {
	Id       int64 //CFC_FileContext_t
	Current  string
	Config   *ConfigInfo
	Err      string
	FileList []FileInfo
}
````
  - ConfigInfo 配置
````
type ConfigInfo struct {
	DeviceName     string //local is nil
	DeviceType     string
	RootSize       int64
	MaxRootSize    int64
	RootSizeErr    string //root计算错误时会有
	PermissionList []string
}  
````
  - FileInfo 文件信息
````
 type FileInfo struct {
	Name    string
	Size    string
	RawSize int64
	Mode    string
	ModTime string
	IsDir   bool
} 
````
  - RemoteStatus 远程状态
````
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
````
  - TransmissionFileTaskInfoProgress 下载进度
````
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
````
  - ScanCFCInfoResp 扫描结果
````
type ScanCFCInfoResp struct {
	DownInfoPath string
	UpInfoPath   string
	ErrMsg       string
}
````

- 报错一览（大部分）

| 值                                               | 备注                                                                                                           |
|-------------------------------------------------|--------------------------------------------------------------------------------------------------------------|
| 自定义报错                                           | 大概率会出现的报错                                                                                                    |
| path range out                                  | 超出root                                                                                                       |
| unexpected device type                          | 意外的设备类型                                                                                                      |
| compute storage exception                       | 计算存储发生意外                                                                                                     |
| exceed max root path size                       | 操作会超出root大小                                                                                                  |
| missing permission                              | 缺少权限                                                                                                         |
| timeout                                         | 超时                                                                                                           |
| os: unexpected                                  | 意外的os错误                                                                                                      |
| io: unexpected                                  | 意外的io错误                                                                                                      |
| fc is not remote                                | 不是远程对象                                                                                                       |
| serverName is inconsistency                     | 服务端名字不一致                                                                                                     |
| file is modified                                | 文件被修改了                                                                                                       |
| file hash does not match. File transfer failed  | hash校验不一致，文件传输失败                                                                                             |
| unexpected transmission type                    | 意外的传输类型                                                                                                      |
| 系统报错                                            | 系统的报错，意思看字面                                                                                                  |
| syntax error in pattern                         | ErrBadPattern indicates a pattern was malformed.                                                             |
| file already exists                             |                                                                                                              |
| file does not exist                             |                                                                                                              |
| file already closed                             |                                                                                                              |
| invalid argument                                | ErrInvalid indicates an invalid argument. Methods on File will return this error when the receiver is nil.   |
| permission denied                               |                                                                                                              |
| i/o timeout                                     |                                                                                                              |
| file type does not support deadline             |                                                                                                              |
| ErrProcessDone indicates a Process has finished |                                                                                                              |
| short write                                     | ErrShortWrite means that a write accepted fewer bytes than requested but failed to return an explicit error. |
| EOF                                             |                                                                                                              |
| unexpected EOF                                  |                                                                                                              |
| multiple Read calls return no data or error     |                                                                                                              |
| short buffer                                    |                                                                                                              |
| io: read/write on closed pipe                   |                                                                                                              |



- 配置文件字段说明
````
{
  "Ct": {
    "IP": "ctIP，为127.0.0.1时会在本地起中转服务",
    "Port": "ct端口",
    "Key": "ct连接密钥"
  },
  "Config": {
    "DeviceName": "server注册名",
    "DeviceType": "DeviceTypeServer(server)/DeviceTypeClient(client)",
    "Root": "服务端root路径(server)",
    "MaxRootSize": root限制空间最大大小(server),
    "Key": "服务约定密钥",
    "PermissionList": [可提供的使用权限]
  }
}
````