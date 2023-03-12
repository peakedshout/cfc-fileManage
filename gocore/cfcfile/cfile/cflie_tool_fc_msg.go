package cfile

import "github.com/peakedshout/go-CFC/tool"

func SetCFMsg(id string, data any) CFCFileCMsg {
	return CFCFileCMsg{
		Id:   id,
		Data: data,
	}
}

func GetCFMsg(in any, out any) (string, error) {
	var info CFCFileCMsg
	err := tool.UnmarshalV2(in, &info)
	if err != nil {
		return "", err
	}
	err = tool.UnmarshalV2(info.Data, out)
	if err != nil {
		return "", err
	}
	return info.Id, nil
}
func GetCFMsgStep1(in any) (CFCFileCMsg, error) {
	var info CFCFileCMsg
	err := tool.UnmarshalV2(in, &info)
	return info, err
}
func (cfm *CFCFileCMsg) GetCFMsgStep2(out any) error {
	return tool.UnmarshalV2(cfm.Data, out)
}

func (cfm *CFCFileCMsg) SetCFMsg(data any) CFCFileCMsg {
	return CFCFileCMsg{
		Id:   cfm.Id,
		Data: data,
	}
}
