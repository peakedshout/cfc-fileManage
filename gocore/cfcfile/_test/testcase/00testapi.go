package testcase

import (
	"github.com/peakedshout/cfc-fileManage/cfile"
	"github.com/peakedshout/cfc-fileManage/ctool"
	"github.com/peakedshout/cfc-fileManage/memory"
	"github.com/peakedshout/cfc-fileManage/server/serverMain"
	"github.com/peakedshout/go-CFC/client"
	"github.com/peakedshout/go-CFC/loger"
	"github.com/peakedshout/go-CFC/server"
	"math/rand"
	"os"
	"path"
	"time"
)

const key1 = "6a647c0bf889419c84e461486f83d776"
const key2 = "edcbf56a029f4a3b901916a3e03dd316"

const proxyAddr = "127.0.0.1:9999"

const serverName = "testserver"
const clientName = "testclient"

var serverpath = path.Join(getPwd(), "./serverpath")
var clientpath = path.Join(getPwd(), "./clientpath")

const downFileName = "down.txt"
const upFileName = "up.txt"

var downFilePath = path.Join(serverpath, downFileName)
var upFilePath = path.Join(clientpath, upFileName)

type testCtx struct {
	ps *server.ProxyServer

	serverBox *client.DeviceBox
	clientBox *client.DeviceBox

	fc *cfile.FileContext
}

func getPwd() string {
	d, err := os.Getwd()
	errCheck(err)
	return d
}

func errCheck(err error) {
	if err != nil {
		loger.SetLogError(err)
	}
}

func newFileSize() int {
	rand.Seed(time.Now().Unix())
	return rand.Intn(90*1024*1024) + 100*1024*1024
}

func NewFilePath() {
	_, err := os.Stat("./testcase")
	errCheck(err)
	err = os.MkdirAll(serverpath, 0777)
	errCheck(err)
	err = os.MkdirAll(clientpath, 0777)
	errCheck(err)
	err = cfile.MakeZeroFile(downFilePath, newFileSize())
	errCheck(err)
	err = cfile.MakeZeroFile(upFilePath, newFileSize())
	errCheck(err)
}
func DelFilePath() {
	err := os.RemoveAll(serverpath)
	errCheck(err)
	err = os.RemoveAll(clientpath)
	errCheck(err)
}

func (ctx *testCtx) CloseAll() {
	if ctx.fc != nil {
		ctx.fc.Close()
	}
	if ctx.clientBox != nil {
		ctx.clientBox.Close()
	}
	if ctx.serverBox != nil {
		ctx.serverBox.Close()
	}
	if ctx.ps != nil {
		ctx.ps.Close()
	}
	DelFilePath()
}

func NewCtx() *testCtx {
	memory.Init()
	memory.InitGF()
	NewFilePath()
	tc := &testCtx{}
	tc.ps = server.NewProxyServer(proxyAddr, key1)
	time.Sleep(1 * time.Second)

	serverConfig := &cfile.ConfigInfo{
		DeviceName:     serverName,
		DeviceType:     cfile.DeviceTypeServer,
		Root:           serverpath,
		RootSize:       0,
		MaxRootSize:    1 * 1024 * 1024 * 1024,
		RootSizeErr:    "",
		RawKey:         key2,
		PermissionList: cfile.PermissionToAll,
	}
	proxyConfig := &ctool.CtConfig{
		Addr: proxyAddr,
		Key:  key1,
	}
	s, err := serverMain.RunMain(serverConfig, proxyConfig)
	errCheck(err)
	tc.serverBox = s
	c, err := client.LinkProxyServer(clientName, proxyAddr, key1)
	errCheck(err)
	tc.clientBox = c

	tc.fc = cfile.InitRemote(tc.clientBox, serverName, key2)
	if tc.fc.Err != "" {
		loger.SetLogError(tc.fc.Err)
	}
	return tc
}
