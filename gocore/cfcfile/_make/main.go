package main

import (
	"fmt"
	"github.com/peakedshout/go-CFC/loger"
	"os"
	"os/exec"
	"path"
)

const (
	configPath = "../config"

	serverPath = "../server/main.go"
	clientPath = "../client/main.go"

	clientHName = "cfcfile_api2.h"
	clientHPath = "../client/cfcfile_api2.h"
)

func main() {
	checkFile()
	makeConfig()
	makeAllBin()
	makeCGO()
	logMake()
}
func logMake() {
	var err error
	logPath := "../cflog_client/main.go"
	outPath := "./asset/LogDial-win-amd64" + "/cfc_fileManageApp_logdialer.exe"
	_, err = os.Stat(logPath)
	errCheck(err)
	var cmd *exec.Cmd
	var b []byte
	cleanGoEnv()
	err = os.Setenv("CGO_ENABLED", "0")
	errCheck(err)
	err = os.Setenv("GOOS", "windows")
	errCheck(err)
	err = os.Setenv("GOARCH", "amd64")
	errCheck(err)
	cmd = exec.Command("go", "build", "-o", outPath, logPath)
	cmd.Env = append(os.Environ())
	b, err = cmd.CombinedOutput()
	fmt.Println("log", "win", "amd64", string(b), err)
	errCheck(err)
	err = os.Chmod(outPath, 0777)
	errCheck(err)
}

func errCheck(err error) {
	if err != nil {
		loger.SetLogError(err)
	}
}
func checkFile() {
	var err error
	_, err = os.Stat(clientPath)
	errCheck(err)
	_, err = os.Stat(serverPath)
	errCheck(err)
	_, err = os.Stat(path.Join(configPath, "config.json"))
	errCheck(err)
	for key, value := range goGoArchMap {
		for _, one := range value {
			err = os.MkdirAll("./asset/"+key+"-"+one, 0777)
			errCheck(err)
		}
	}
}

var goGoosMap = map[string]string{
	"win":   "windows",
	"mac":   "darwin",
	"linux": "linux",
}
var goGoArchMap = map[string][]string{
	"linux": {"386", "amd64", "arm64", "arm"},
	"mac":   {"amd64", "arm64"},
	"win":   {"386", "amd64", "arm64", "arm"},
}

var goEnvMap = map[string]string{
	"GOPATH":          os.Getenv("GOPATH"),
	"GOROOT":          os.Getenv("GOROOT"),
	"GO111MODULE":     os.Getenv("GO111MODULE"),
	"_":               os.Getenv("_"),
	"PWD":             os.Getenv("PWD"),
	"HOME":            os.Getenv("HOME"),
	"PATH":            os.Getenv("PATH"),
	"w64-mingw32-gcc": os.Getenv("w64-mingw32-gcc"),
}

func cleanGoEnv() {
	var err error
	os.Clearenv()
	for key, value := range goEnvMap {
		err = os.Setenv(key, value)
		errCheck(err)
	}
}
func makeAllBin() {
	for key, value := range goGoArchMap {
		for _, one := range value {
			makeBin(key, one)
		}
	}
}
func printEnv() {
	for _, one := range os.Environ() {
		fmt.Println(one)
	}
}

func makeBin(goos, arch string) {
	var err error
	var cmd *exec.Cmd
	var b []byte
	cleanGoEnv()
	err = os.Setenv("CGO_ENABLED", "0")
	errCheck(err)
	err = os.Setenv("GOOS", goGoosMap[goos])
	errCheck(err)
	err = os.Setenv("GOARCH", arch)

	errCheck(err)
	s := getOutPath(goos, arch)
	cmd = exec.Command("go", "build", "-o", s, serverPath)
	cmd.Env = append(os.Environ())
	b, err = cmd.CombinedOutput()
	fmt.Println("s", goos, arch, string(b), err)
	errCheck(err)
	err = os.Chmod(s, 0777)
	errCheck(err)
}

// only win x86 an=amd64
func makeCGO() {
	var err error
	var cmd *exec.Cmd
	var b []byte
	cleanGoEnv()
	err = os.Setenv("CGO_ENABLED", "1")
	errCheck(err)
	err = os.Setenv("GOOS", "windows")
	errCheck(err)
	err = os.Setenv("GOARCH", "amd64")
	errCheck(err)
	err = os.Setenv("CC", "x86_64-w64-mingw32-gcc")
	errCheck(err)
	err = os.Setenv("CXX", "x86_64-w64-mingw32-g++")
	errCheck(err)

	s := getOutPathGGO("cfcfile_api_win_amd64.dll")
	cmd = exec.Command("go", "build", "-ldflags", "-s -w", "-buildmode=c-shared", "-o", s, clientPath)
	cmd.Env = append(os.Environ())
	b, err = cmd.CombinedOutput()
	fmt.Println("cgo", "win", "amd64", string(b), err)
	errCheck(err)
	err = os.Chmod(s, 0777)
	errCheck(err)

	b1, err := os.ReadFile(clientHPath)
	errCheck(err)
	err = os.WriteFile(getOutPathGGO(clientHName), b1, 0777)
	errCheck(err)
}

func getOutPath(goos, arch string) string {
	out := ""
	out = "./asset/" + goos + "-" + arch + "/cfc_fileManageApp_server_" + goos + "-" + arch
	if goos == "win" || goos == "windows" {
		out += ".exe"
	}
	return out
}

func getOutPathGGO(p string) string {
	out := ""
	out = "./asset/CGO-" + "win" + "-" + "amd64" + "/" + p
	return out
}

var configMMap = map[string]string{
	"": "",
}

func makeConfig() {
	fn1 := func(key string) string {
		return configPath + "/config" + key + ".json"
	}
	fn2 := func(key string) string {
		return "./asset/config" + key
	}
	for key, value := range configMMap {
		b, err := os.ReadFile(fn1(value))
		errCheck(err)
		err = os.MkdirAll(fn2(key), 0777)
		errCheck(err)
		err = os.WriteFile(fn2(key)+"/config.json", b, 0777)
		errCheck(err)
	}
}
