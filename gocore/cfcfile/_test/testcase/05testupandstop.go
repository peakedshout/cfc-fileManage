package testcase

import (
	"fmt"
	"github.com/peakedshout/go-CFC/loger"
	"path"
	"time"
)

func Test05UpAndStop() {
	loger.SetLoggerLevel(loger.LogLevelError)
	ctx := NewCtx()
	defer ctx.CloseAll()
	op, err := ctx.fc.NewTaskUpFile(path.Join(clientpath, upFileName), path.Join("", upFileName), true, 0)
	errCheck(err)
	go func() {
		for {
			p, err := ctx.fc.GetTransmissionTaskProgress(op)
			fmt.Println(p, err)
			time.Sleep(100 * time.Millisecond)
		}
	}()
	go func() {
		time.Sleep(1 * time.Second)
		ctx.fc.StopTaskTransmissionFile(op)
	}()
	err = ctx.fc.StartTaskUpFile(op, false)
	fmt.Println(err)
	time.Sleep(1 * time.Second)
	err = ctx.fc.StartTaskUpFile(op, false)
	errCheck(err)
	time.Sleep(1 * time.Second)
}
