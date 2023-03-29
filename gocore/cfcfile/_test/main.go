package main

import (
	"github.com/peakedshout/cfc-fileManage/_test/testcase"
	"time"
)

func main() {
	testcase.Test01Init()
	time.Sleep(1 * time.Second)
	testcase.Test02Down()
	time.Sleep(1 * time.Second)
	testcase.Test03DownAndStop()
	time.Sleep(1 * time.Second)
	testcase.Test04Up()
	time.Sleep(1 * time.Second)
	testcase.Test05UpAndStop()
	time.Sleep(1 * time.Second)
	testcase.Test06Log()
}
