package testcase

import (
	"fmt"
	"github.com/peakedshout/go-CFC/loger"
	"path"
	"time"
)

func Test03DownAndStop() {
	loger.SetLoggerLevel(loger.LogLevelError)
	ctx := NewCtx()
	defer ctx.CloseAll()
	op, err := ctx.fc.NewTaskDownFile(downFileName, path.Join(clientpath, downFileName), true, 0)
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
	go func() {
		time.Sleep(1 * time.Second)
		ctx.fc.StopTaskTransmissionFile(op)
	}()
	err = ctx.fc.StartTaskDownFile(op, true)
	fmt.Println(err)
	time.Sleep(1 * time.Second)
	err = ctx.fc.StartTaskDownFile(op, false)
	errCheck(err)
	time.Sleep(1 * time.Second)
}
