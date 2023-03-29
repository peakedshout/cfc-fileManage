package main

import (
	"flag"
	"github.com/peakedshout/cfc-fileManage/cflog_client/cflogdial"
)

func main() {
	p := flag.Int("p", 0, "log port")
	flag.Parse()
	if *p <= 0 || *p > 65535 {
		panic("port is ?")
	}
	cflogdial.DialLog(*p)
}
