package cfconst

const version = "cfc-fileManage-v0.1.0-beta.1"

const addr = "https://github.com/peakedshout/cfc-fileManage"

const about = `## 软件名：cfc-fileManage(client-qt)
### 版本：` + version + `
###### 这是一个免费的文件管理工具，目前还是测试版本
***
### 项目地址：[cfc-fileManage](` + addr + `)
### 作者列表：
- [peakedshout](https://github.com/peakedshout)(后端业务)
- [BUDAI-AZ](https://github.com/BUDAI-AZ)(qt页面)
***
##### 底层通信协议：[go-CFC](https://github.com/peakedshout/go-CFC)
`

func GetVersion() string {
	return version
}
func GetAddr() string {
	return addr
}
func GetAbout() string {
	return about
}
