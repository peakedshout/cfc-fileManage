package testcase

import (
	"github.com/peakedshout/go-CFC/loger"
)

func Test01Init() {
	loger.SetLoggerLevel(loger.LogLevelError)
	ctx := NewCtx()
	defer ctx.CloseAll()
	ctx.fc.OpenRemote("./")
	if ctx.fc.Err != "" {
		loger.SetLogError(ctx.fc.Err)
	}
}
