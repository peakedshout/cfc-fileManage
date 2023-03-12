package main

import (
	"flag"
	"github.com/peakedshout/cfc-fileManage/server/serverMain"
)

func main() {
	cp := flag.String("c", "./config.json", "config path...")
	flag.Parse()
	serverMain.RunAll(*cp)
}
