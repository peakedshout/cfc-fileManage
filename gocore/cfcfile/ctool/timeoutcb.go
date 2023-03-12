package ctool

import (
	"errors"
	"sync"
	"time"
)

type TimeoutCb struct {
	id   string
	wait chan error
	d    time.Duration
	fn   func()
}

func NewTimeoutCb(timeout time.Duration, fn func(), id string) *TimeoutCb {
	tcb := &TimeoutCb{
		id:   id,
		wait: make(chan error, 1),
		d:    timeout,
		fn:   fn,
	}
	return tcb
}
func (tcb *TimeoutCb) Wait() error {
	select {
	case e := <-tcb.wait:
		tcb.fn()
		return e
	case <-time.After(tcb.d):
		return errors.New("timeout")
	}
}
func (tcb *TimeoutCb) Set(id string, err error) {
	if tcb.id != id {
		return
	}
	tcb.wait <- err
}

type TimeoutCbV2 struct {
	id   string
	wait chan error
	d    time.Duration
	fn   func() error
	lock sync.Mutex
}

func NewTimeoutCbV2() *TimeoutCbV2 {
	tcb := &TimeoutCbV2{
		wait: make(chan error, 1),
	}
	return tcb
}
func (tcb *TimeoutCbV2) Reset(id string, timeout time.Duration) {
	tcb.lock.Lock()
	defer tcb.lock.Unlock()
	tcb.id = id
	tcb.d = timeout
}
func (tcb *TimeoutCbV2) Wait() error {
	select {
	case e := <-tcb.wait:
		if tcb.fn != nil && e == nil {
			e = tcb.fn()
		}
		return e
	case <-time.After(tcb.d):
		return errors.New("timeout")
	}
}
func (tcb *TimeoutCbV2) SetCb(id string, fn func() error) {
	tcb.lock.Lock()
	defer tcb.lock.Unlock()
	if tcb.id != id {
		return
	}
	tcb.fn = fn
	tcb.wait <- nil
}
