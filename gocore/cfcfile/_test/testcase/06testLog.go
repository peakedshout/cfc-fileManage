package testcase

import (
	"github.com/peakedshout/cfc-fileManage/cflog"
	"github.com/peakedshout/cfc-fileManage/cflog_client/cflogdial"
	"github.com/peakedshout/go-CFC/loger"
	"time"
)

func Test06Log() {
	cflog.Init()
	cflog.Opt(loger.LogLevelDebug, false)
	err := cflog.Listen(11111)
	errCheck(err)
	go func() {
		cflogdial.DialLog(11111)
	}()
	time.Sleep(1 * time.Second)
	loger.SetLogWarn(time.Now().Nanosecond(), "dwdwdwd")
	time.Sleep(2 * time.Second)
}
