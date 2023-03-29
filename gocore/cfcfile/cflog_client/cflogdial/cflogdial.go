package cflogdial

import (
	"io"
	"net"
	"os"
)

func DialLog(port int) {
	raddr := &net.TCPAddr{
		IP:   net.IP{127, 0, 0, 1},
		Port: port,
		Zone: "",
	}
	conn, err := net.DialTCP("tcp", nil, raddr)
	if err != nil {
		panic(err)
	}
	defer conn.Close()
	io.Copy(os.Stdout, conn)
}
