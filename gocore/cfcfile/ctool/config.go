package ctool

import "github.com/peakedshout/go-CFC/_hook-tcp/config"

type ServerConfig struct {
	Ct      CtConfig             `json:"Ct"`
	Config  BaseConfig           `json:"Config"`
	Setting config.SettingConfig `json:"Setting"`
}
type BaseConfig struct {
	DeviceName  string       `json:"DeviceName"`
	DeviceType  string       `json:"DeviceType"`
	UserConfigs []UserConfig `json:"UserConfigs"`
}
type UserConfig struct {
	UserName       string   `json:"UserName"`
	UserKey        string   `json:"UserKey"`
	Root           string   `json:"Root"`
	MaxRootSize    int64    `json:"MaxRootSize"`
	PermissionList []string `json:"PermissionList"`
}
type CtConfig struct {
	Addr string `json:"Addr"`
	Key  string `json:"Key"`
}
