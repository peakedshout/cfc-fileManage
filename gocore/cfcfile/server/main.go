package main

import (
	"cfcfile/server/serverMain"
	"flag"
)

func main() {
	cp := flag.String("c", "./config.json", "config path...")
	flag.Parse()
	serverMain.RunAll(*cp)
}
