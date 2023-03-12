package cfile

const (
	DeviceTypeServer = "DeviceTypeServer"
	DeviceTypeClient = "DeviceTypeClient"
)
const (
	PermissionLink     = "PermissionLink"     //许可连接//基本
	PermissionOpen     = "PermissionOpen"     //许可访问文件//基本
	PermissionUpload   = "PermissionUpload"   //许可上传
	PermissionCopy     = "PermissionCopy"     //许可复制
	PermissionMove     = "PermissionMove"     //许可剪切
	PermissionRename   = "PermissionRename"   //许可重命名
	PermissionMkDir    = "PermissionMkDir"    //许可创建文件夹
	PermissionRemove   = "PermissionRemove"   //许可删除
	PermissionDownload = "PermissionDownload" //许可下载
	PermissionRead     = "PermissionRead"     //许可读取
)

var PermissionToAll = []string{PermissionLink, PermissionOpen, PermissionUpload, PermissionCopy, PermissionMove, PermissionRename, PermissionMkDir, PermissionRemove, PermissionDownload, PermissionRead}

var PermissionToDefaultClient = []string{PermissionLink, PermissionOpen, PermissionCopy, PermissionMove, PermissionRename, PermissionMkDir, PermissionRemove, PermissionRead}

var PermissionToDefaultServer = []string{PermissionLink, PermissionOpen, PermissionUpload, PermissionCopy, PermissionMove, PermissionRename, PermissionMkDir, PermissionRemove, PermissionDownload, PermissionRead}
