package cflog

import (
	"github.com/peakedshout/go-CFC/complexRW"
	"github.com/peakedshout/go-CFC/loger"
	"net"
	"sync"
	"sync/atomic"
)

var gLogLn *net.TCPListener
var gLogLnLock sync.Mutex

var gLogConnLock sync.Mutex
var gLogConnMap map[complexRW.RWId]net.Conn

func Init() {
	loger.SetLoggerLevel(loger.LogLevelWarn)
	gLogConnMap = make(map[complexRW.RWId]net.Conn)
}
func Opt(level uint8, stack bool) {
	loger.SetLoggerStack(stack)
	loger.SetLoggerLevel(level)
}
func Listen(port int) error {
	gLogLnLock.Lock()
	defer gLogLnLock.Unlock()
	if gLogLn != nil && gLogLn.Addr().(*net.TCPAddr).Port == port {
		return nil
	}
	if gLogLn != nil {
		gLogLn.Close()
		gLogConnLock.Lock()
		for _, one := range gLogConnMap {
			one.Close()
		}
		gLogConnLock.Unlock()
	}
	laddr := &net.TCPAddr{
		IP:   net.IP{127, 0, 0, 1},
		Port: port,
	}
	ln, err := net.ListenTCP("tcp", laddr)
	if err != nil {
		return err
	}
	gLogLn = ln
	go func() {
		var disable atomic.Bool
		defer func() {
			disable.Store(true)
		}()
		for true {
			conn, err := ln.Accept()
			if err != nil {
				return
			}
			go handle(conn, &disable)
		}
	}()
	return nil
}
func handle(conn net.Conn, disable *atomic.Bool) {
	defer conn.Close()
	id := loger.SetLoggerCopy(conn)
	defer func() {
		loger.DelLoggerCopy(id)
	}()
	setConnM(id, conn, disable)
	defer func() {
		delConnM(id)
	}()
	var buff []byte
	for true {
		_, err := conn.Read(buff)
		if err != nil {
			return
		}
	}
}
func delConnM(id complexRW.RWId) {
	gLogConnLock.Lock()
	delete(gLogConnMap, id)
	gLogConnLock.Unlock()
}
func setConnM(id complexRW.RWId, conn net.Conn, disable *atomic.Bool) {
	gLogConnLock.Lock()
	defer gLogConnLock.Unlock()
	if disable.Load() {
		return
	}
	gLogConnMap[id] = conn
}
