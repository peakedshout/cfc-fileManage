package cfile

// 更新root信息
func (ci *ConfigInfo) updateRootSize() error {
	size, err := CalculateSize(ci.Root)
	if err != nil {
		return ErrComputeStorage
	}
	ci.RootSize = size
	if ci.RootSize > ci.MaxRootSize {
		return ErrExceedMaxRootSize
	}
	return nil
}

// 检查权限
func (ci *ConfigInfo) checkPermission(permissions ...[]string) bool {
	permissionMap := make(map[string]bool)
	for _, one := range ci.PermissionList {
		permissionMap[one] = true
	}

	for _, one := range permissions {
		result := false
		for _, two := range one {
			if !permissionMap[two] {
				result = true
				break
			}
		}
		if result {
			continue
		}
		return true
	}
	return false
}
