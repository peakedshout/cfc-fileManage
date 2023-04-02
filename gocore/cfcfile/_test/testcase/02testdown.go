package testcase

import (
	"fmt"
	"github.com/peakedshout/go-CFC/loger"
	"path"
	"time"
)

func Test02Down() {
	loger.SetLoggerLevel(loger.LogLevelError)
	testPrint("Test02Down")
	defer testPrint("Test02Down")
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
	err = ctx.fc.StartTaskDownFile(op, true)
	errCheck(err)
	time.Sleep(1 * time.Second)
}
