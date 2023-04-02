package cfile

import (
	"errors"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"runtime/debug"
)

var (
	ErrLoginUserErr = errors.New("login user name or key is err")

	ErrRootPathRangeOut          = errors.New("path range out") //root path range out
	ErrDeviceType                = errors.New("unexpected device type")
	ErrComputeStorage            = errors.New("compute storage exception")
	ErrExceedMaxRootSize         = errors.New("exceed max root path size")
	ErrMissingPermission         = errors.New("missing permission")
	ErrTimeout                   = errors.New("timeout")
	ErrOsUnexpected              = errors.New("os: unexpected")
	ErrIoUnexpected              = errors.New("io: unexpected")
	ErrBadPattern                = filepath.ErrBadPattern
	ErrFcNotRemote               = errors.New("fc is not remote")
	ErrServerNameIsInconsistency = errors.New("serverName is inconsistency")
	ErrFileIsModified            = errors.New("file is modified")
	ErrFileHashCheckFailed       = errors.New("file hash does not match. File transfer failed")
	ErrTransmissionType          = errors.New("unexpected transmission type")
	ErrTransmissionStop          = errors.New("transmission is stop")

	ErrFileContextIsNotFound = errors.New("file context is not found")
)

var osErrList = []error{os.ErrExist, os.ErrNotExist, os.ErrClosed, os.ErrInvalid, os.ErrPermission, os.ErrDeadlineExceeded, os.ErrNoDeadline, os.ErrProcessDone}

func osIsErr(err error) error {
	if err == nil {
		return nil
	}
	for _, one := range osErrList {
		if errors.Is(err, one) {
			return one
		}
	}
	fmt.Println("errOs:", err, string(debug.Stack()))
	return ErrOsUnexpected
}

var ioErrList = []error{io.ErrShortWrite, io.EOF, io.ErrUnexpectedEOF, io.ErrNoProgress, io.ErrShortBuffer, io.ErrClosedPipe}

func ioIsErr(err error) error {
	if err == nil {
		return nil
	}
	for _, one := range ioErrList {
		if errors.Is(err, one) {
			return one
		}
	}
	fmt.Println("errIs:", err, string(debug.Stack()))
	return ErrIoUnexpected
}
