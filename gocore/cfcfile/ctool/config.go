package ctool

type ServerConfig struct {
	Ct     CtConfig   `json:"Ct"`
	Config BaseConfig `json:"Config"`
}
type BaseConfig struct {
	DeviceName     string   `json:"DeviceName"`
	DeviceType     string   `json:"DeviceType"`
	Root           string   `json:"Root"`
	MaxRootSize    int64    `json:"MaxRootSize"`
	Key            string   `json:"Key"`
	PermissionList []string `json:"PermissionList"`
}
type CtConfig struct {
	Addr string `json:"Addr"`
	Key  string `json:"Key"`
}
