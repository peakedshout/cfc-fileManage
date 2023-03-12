package main

import (
	"cfcfile/_test/testcase"
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
}
