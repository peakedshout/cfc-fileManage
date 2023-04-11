package cfile

import (
	"github.com/peakedshout/go-CFC/client"
	"github.com/peakedshout/go-CFC/tool"
)

func (ri *RemoteInfo) GetId() string {
	return ri.rid
}

func (ri *RemoteInfo) SetId(id string) {
	ri.rid = id
}

func (ri *RemoteInfo) SetSub(key string, sub *client.SubBox) {
	ri.subLock.Lock()
	defer ri.subLock.Unlock()
	m, ok := ri.subMap[key]
	if !ok {
		ri.subMap[key] = make(map[int]*client.SubBox)
	}
	l := len(m)
	ri.subMap[key][l] = sub
}

func (ri *RemoteInfo) DelSub(key string) {
	ri.subLock.Lock()
	defer ri.subLock.Unlock()
	delete(ri.subMap, key)
}

func (rc *readCbContext) runAndWaitCb(fn func()) error {
	fn()
	return rc.waitCb()
}

func (rc *readCbContext) waitCb() error {
	return <-rc.wait
}

// 获取sub网速
func (ri *RemoteInfo) GetSubSpeed(key ...string) [][]tool.NetworkSpeedView {
	var resp [][]tool.NetworkSpeedView
	ri.subLock.Lock()
	defer ri.subLock.Unlock()
	for _, one := range key {
		var info []tool.NetworkSpeedView
		s, ok := ri.subMap[one]
		if ok {
			for _, two := range s {
				info = append(info, two.GetNetworkSpeedView())
			}
		}
		resp = append(resp, info)
	}
	return resp
}

// 获取client及所有sub的网速
func (ri *RemoteInfo) GetAllSubSpeed() tool.NetworkSpeedView {
	ri.subLock.Lock()
	defer ri.subLock.Unlock()
	var sl []tool.NetworkSpeedView
	for _, one := range ri.subMap {
		for _, two := range one {
			sl = append(sl, two.GetAllNetworkSpeedView())
		}
	}
	sl = append(sl, ri.sub.GetAllNetworkSpeedView())
	return tool.CountAllNetworkSpeedView(sl...)
}

func (ri *RemoteInfo) SetCFMsg(data any) CFCFileCMsg {
	return SetCFMsg(ri.rid, data)
}
