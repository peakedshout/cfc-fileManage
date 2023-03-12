package memory

import (
	"github.com/peakedshout/go-CFC/tool"
	"sync"
	"sync/atomic"
)

var gMemory *memory
var gFMemory *FMemory

type memory struct {
	next int64
	m    sync.Map
}

func Init() {
	gMemory = &memory{
		next: 1,
		m:    sync.Map{},
	}
}

func Set(a any) int64 {
	id := atomic.AddInt64(&gMemory.next, 1)
	if id == 0 {
		id = atomic.AddInt64(&gMemory.next, 1)
	}
	gMemory.m.Store(id, a)
	return id
}
func Get(id int64) (a any, ok bool) {
	a, ok = gMemory.m.Load(id)
	return
}
func Del(id int64) {
	gMemory.m.Delete(id)
}

type SMemory struct {
	m sync.Map
}

func InitS() *SMemory {
	return &SMemory{m: sync.Map{}}
}
func (s *SMemory) Set(a any) string {
	id := tool.NewId(1)
	s.m.Store(id, a)
	return id
}
func (s *SMemory) Get(id string) (a any, ok bool) {
	a, ok = s.m.Load(id)
	return
}
func (s *SMemory) Del(id string) {
	s.m.Delete(id)
}

type FMemory struct {
	m sync.Map
}

func InitF() *FMemory {
	return &FMemory{m: sync.Map{}}
}
func InitGF() {
	gFMemory = InitF()
}
func (f *FMemory) Set(fn func()) string {
	id := tool.NewId(1)
	f.m.Store(id, fn)
	return id
}
func (f *FMemory) SetOnly(id string, fn func()) {
	f.m.Store(id, fn)
}
func (f *FMemory) Get(id string) (fn func(), ok bool) {
	a, ok := f.m.Load(id)
	if !ok {
		return nil, ok
	}
	return a.(func()), ok
}
func (f *FMemory) GetRunDel(id string) (ok bool) {
	a, ok := f.m.Load(id)
	if ok {
		a.(func())()
		f.m.Delete(id)
	}
	return ok
}
func (f *FMemory) Del(id string) {
	f.m.Delete(id)
}
func FSet(fn func()) string {
	return gFMemory.Set(fn)
}
func FSetOnly(id string, fn func()) {
	gFMemory.SetOnly(id, fn)
}
func FGet(id string) (fn func(), ok bool) {
	return gFMemory.Get(id)
}
func FGetRunDel(id string) (ok bool) {
	return gFMemory.GetRunDel(id)
}
func FDel(id string) {
	gFMemory.Del(id)
}
