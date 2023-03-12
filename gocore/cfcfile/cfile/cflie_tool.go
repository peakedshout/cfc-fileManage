package cfile

import (
	"bufio"
	"crypto/sha256"
	"encoding/hex"
	"fmt"
	"github.com/peakedshout/go-CFC/tool"
	"io"
	"io/fs"
	"os"
	"path"
	"path/filepath"
)

func MakeHash(b []byte) []byte {
	s := sha256.New()
	s.Write(b)
	return s.Sum(nil)
}

func GetFileHash(path string) (string, error) {
	f, err := os.Open(path)
	if err != nil {
		return "", osIsErr(err)
	}
	defer f.Close()
	hash := sha256.New()
	if _, err := io.Copy(hash, f); err != nil {
		return "", ioIsErr(err)
	}
	h := hash.Sum(nil)
	value := hex.EncodeToString(h)
	return value, nil
}

// deprecated
func GetFileHash2(path string) (string, error) {
	f, err := os.Open(path)
	if err != nil {
		return "", osIsErr(err)
	}
	defer f.Close()
	reader := bufio.NewReaderSize(f, 4096)
	buf := make([]byte, 4096)
	var h []byte
	for {
		n, err := reader.Read(buf)
		if err != nil && err != io.EOF {
			return "", ioIsErr(err)
		}
		if 0 == n {
			break
		}
		//if 4096 != n {
		//	break
		//}
		h = append(h, buf[:0]...)
		h = MakeHash(h)
	}
	return tool.MustBytesToBase64(h).Data, nil
}
func GetFileDetailInfo(path string, makeHash bool) (info FileDetailInfo, err error) {
	f, err := os.Stat(path)
	if err != nil {
		err = osIsErr(err)
		return
	}
	if makeHash {
		h, err := GetFileHash(path)
		if err != nil {
			return info, err
		}
		info.Hash = h
	}
	info.FileName = f.Name()
	info.Size = f.Size()
	info.ModTime = f.ModTime().String()
	return info, nil
}

func ReadSome(path string, offset int64, limit int) ([]byte, error) {
	f, err := os.Open(path)
	if err != nil {
		return nil, osIsErr(err)
	}
	defer f.Close()
	_, err = f.Seek(offset, 0)
	if err != nil {
		return nil, osIsErr(err)
	}
	//buffer := 4094

	reader := bufio.NewReaderSize(f, limit)
	buf := make([]byte, limit)
	j, err := reader.Read(buf)
	if err != nil {
		return nil, err
	}
	return buf[:j], nil
}

// 外部处理err
func MakeZeroFile(p string, size int) (err error) {
	dirPath := path.Dir(p)
	err = os.MkdirAll(dirPath, 0777)
	if err != nil {
		return err
	}
	f, err := os.Create(p)
	if err != nil {
		return err
	}
	defer func() {
		f.Close()
		if err != nil {
			os.Remove(p)
		}
	}()
	buff := 4096
	b := make([]byte, buff)
	j := size / buff
	k := size % buff
	for i := 0; i < j; i++ {
		_, err := f.Write(b)
		if err != nil {
			return err
		}
	}
	if k != 0 {
		_, err := f.Write(b[:k])
		if err != nil {
			return err
		}
	}
	return nil
}

func CopyFileOrDir(fromFile, from, to string) error {
	//复制文件的路径
	rel, err := filepath.Rel(from, fromFile)
	if err != nil {
		return ErrBadPattern
	}
	toFile := filepath.Join(to, rel)
	//创建复制文件目录
	if err = os.MkdirAll(filepath.Dir(toFile), 0777); err != nil {
		return osIsErr(err)
	}
	//读取源文件
	file, err := os.Open(fromFile)
	if err != nil {
		return osIsErr(err)
	}
	defer file.Close()
	bufReader := bufio.NewReader(file)
	// 创建复制文件用于保存
	out, err := os.Create(toFile)
	if err != nil {
		return osIsErr(err)
	}
	defer out.Close()
	// 然后将文件流和文件流对接起来
	_, err = io.Copy(out, bufReader)
	return ioIsErr(err)
}
func MoveFileOrDir(fromFile, from, to string) error {
	//复制文件的路径
	rel, err := filepath.Rel(from, fromFile)
	if err != nil {
		return ErrBadPattern
	}
	toFile := filepath.Join(to, rel)
	//创建复制文件目录
	if err = os.MkdirAll(filepath.Dir(toFile), 0777); err != nil {
		return osIsErr(err)
	}
	err = os.Rename(fromFile, toFile)
	if err != nil {
		return osIsErr(err)
	}
	return nil
}

func checkLocalFileIsNotExist(p string) bool {
	_, err := os.Stat(p)
	if !os.IsNotExist(err) {
		return false
	}
	return true
}

func checkLocalFileIsExist(p string) bool {
	_, err := os.Stat(p)
	if err != nil {
		return false
	}
	return true
}

// 字节的单位转换 保留两位小数
func FormatFileSize(fileSize int64) (size string) {
	if fileSize < 1024 {
		//return strconv.FormatInt(fileSize, 10) + "B"
		return fmt.Sprintf("%.2fB", float64(fileSize)/float64(1))
	} else if fileSize < (1024 * 1024) {
		return fmt.Sprintf("%.2fKB", float64(fileSize)/float64(1024))
	} else if fileSize < (1024 * 1024 * 1024) {
		return fmt.Sprintf("%.2fMB", float64(fileSize)/float64(1024*1024))
	} else if fileSize < (1024 * 1024 * 1024 * 1024) {
		return fmt.Sprintf("%.2fGB", float64(fileSize)/float64(1024*1024*1024))
	} else if fileSize < (1024 * 1024 * 1024 * 1024 * 1024) {
		return fmt.Sprintf("%.2fTB", float64(fileSize)/float64(1024*1024*1024*1024))
	} else { //if fileSize < (1024 * 1024 * 1024 * 1024 * 1024 * 1024)
		return fmt.Sprintf("%.2fEB", float64(fileSize)/float64(1024*1024*1024*1024*1024))
	}
}

// 计算路径大小（包含文件夹大小）
func CalculateSize(p string) (int64, error) {
	_, err := os.Stat(p)
	if err != nil {
		return 0, err
	}
	var size int64
	err = filepath.Walk(p, func(path string, info fs.FileInfo, err error) error {
		//if !info.IsDir() {
		size += info.Size()
		//}
		return err
	})
	return size, err
}

func FtoF(list []os.FileInfo) (flist []FileInfo) {
	for _, one := range list {
		flist = append(flist, FileInfo{
			Name:    one.Name(),
			Size:    FormatFileSize(one.Size()),
			RawSize: one.Size(),
			Mode:    one.Mode().String(),
			ModTime: one.ModTime().Format("2006-01-02 15:04:05"),
			IsDir:   one.IsDir(),
			//Sys:     one.Sys(),
		})
	}
	return
}
