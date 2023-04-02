***
# cfc-fileManage
###### *一个基于go-CFC开发的文件传输工具，目标是实现私人网盘。*
***
### 简体中文
***
## 什么是cfc-fileManage？
- 这是一个使用了go-CFC为底层工具包所开发的项目，如果你想了解go-CFC是什么，你可以点击[这里](https://github.com/peakedshout/go-CFC)，查看相关信息
- 它只是在底层工具包上完成了业务逻辑，最主要的功能是文件管理，支持上传下载文件，并对传输通信进行加密，并且具有权限校验及容量限制等功能
- 如果你只是想远程访问某个内网的设备文件，选择它，是个不错的选择
***
## 为什么会出现这玩意？
- 众所周知，网络有公网和内网，两个内网的设备想要传输数据有些麻烦，更何况是跨越复杂的网络环境
- go-CFC解决了内网间通信的问题，显然，如果你用过go-CFC，你会发现它只是个底层工具，就算是配套编译的hook-tcp,那也缺少了实体的客户端和服务端，而这，cfc-fileManage就是基于go-CFC的一个文件管理应用的实现
- 如果你想，你甚至可以把它弄成私人网盘，它的目的也是如此
***
## 私人网盘？
- 是的，cfc-fileManage的目标就是这个
- 对于我们这些贫民来说，从公网上获取一台服务器（大概率是vps），拥有公网ip就十分可贵了，而添加云盘存储的价格是另外计算的，它很昂贵，并且它不是永久的，过期如果不去续费将会被销毁
- 而你从网上购物获取的硬盘，是相对便宜和划算的，无论是固体还是机械，并且是永久的，（或许应该不是分期付款。。。）
- 并且它的计算大多都是在本地机器运行，公网的代理服务器只是转发数据罢了，所以就算是丐版的服务器也是可以运行的
***
## 怎么使用？
- 准备工具：一台具有公网ip能访问的服务器，一台作为服务端的设备，一台作为客户端的设备
  - 这个公网服务器建议是带宽大的毕竟好，因为go-CFC它只是代理流量，如果是个小水管，那你将会很难受
  - 目前客户端只有win amd64的qt页面，后续会完善的
- 安装代理服务（公网服务器）
  - 因为采用的是go-CFC协议，可以在[这里](https://github.com/peakedshout/go-CFC)将hook-tcp的服务端安装在公网服务器上，之后它会代理一切执行go-CFC协议的应用
- 安装服务端（服务端设备）
  - 填写config文件，[这里](./gocore/cfcfile/config/config.json)获取
  - PermissionList填写：PermissionLink     | 许可连接//基本   | PermissionOpen     | 许可访问文件//基本 | PermissionUpload   | 许可上传       | PermissionCopy     | 许可复制       | PermissionMove     | 许可剪切       | PermissionRename   | 许可重命名      | PermissionMkDir    | 许可创建文件夹    | PermissionRemove   | 许可删除       | PermissionDownload | 许可下载       | PermissionRead     | 许可读取
  - 运行cfc_fileManageApp_server -c config.json
- 运行客户端（qt）
  - 运行exe，填写如下信息：