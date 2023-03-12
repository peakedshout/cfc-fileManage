package cfile

const TransmissionTypeUp = "TransmissionTypeUp"
const TransmissionTypeDown = "TransmissionTypeDown"

const downFileSizeBuff = 1024 * 64
const upFileSizeBuff = 1024 * 64

const (
	UpStatusUploadReset      = "UpStatusUploadReset"      //上传重置
	UpStatusUploadInProgress = "UpStatusUploadInProgress" //上传进行中
	UpStatusUploadCompleted  = "UpStatusUploadCompleted"  //上传完成
	UpStatusUploadFailure    = "UpStatusUploadFailure"    //上传失败
	UpStatusUploadPause      = "UpStatusUploadPause"      //上传暂停
	UpStatusUploadFileNil    = "UpStatusUploadFileNil"    //上传文件不存在
)

const (
	DownStatusDownloadReset      = "DownStatusDownloadReset"      //下载重置
	DownStatusDownloadInProgress = "DownStatusDownloadInProgress" //下载进行中
	DownStatusDownloadCompleted  = "DownStatusDownloadCompleted"  //下载完成
	DownStatusDownloadFailure    = "DownStatusDownloadFailure"    //下载失败
	DownStatusDownloadPause      = "DownStatusDownloadPause"      //下载暂停
	DownStatusDownloadFileNil    = "DownStatusDownloadFileNil"    //下载文件不存在
)
