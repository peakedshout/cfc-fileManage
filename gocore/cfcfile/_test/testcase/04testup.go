package testcase

import (
	"fmt"
	"github.com/peakedshout/go-CFC/loger"
	"path"
	"time"
)

func Test04Up() {
	loger.SetLoggerLevel(loger.LogLevelError)
	ctx := NewCtx()
	defer ctx.CloseAll()
	op, err := ctx.fc.NewTaskUpFile(path.Join(clientpath, upFileName), path.Join("", upFileName), true, 0)
	errCheck(err)
	r := true
	defer func() { r = false }()
	go func() {
		for r {
			p, err := ctx.fc.GetTransmissionTaskProgress(op)
			fmt.Println(p, err)
			time.Sleep(100 * time.Millisecond)
		}
	}()
	err = ctx.fc.StartTaskUpFile(op, false)
	errCheck(err)
	time.Sleep(2 * time.Second)
}
