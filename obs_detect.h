#pragma once

#include <Windows.h>

#include "process_list_tasklist.h"
#include "process_list_winapi.h"
#include "process_list_wmi.h"

#include "process_modules.h"

typedef HRESULT (*GetProcessModulesFunc)(DWORD, OUT std::vector<std::string>&);
typedef HRESULT (*GetProcessNamePidPairsFunc)(std::vector<std::tuple<std::string, DWORD>>&);

HRESULT ObsDetectInitialize();

BOOL DidAnyModuleAddressChange();

HRESULT CheckModuleLoaded(
	UINT uProcessId, 
	GetProcessModulesFunc f, 
	BOOL* pResult
);

HRESULT CheckProcessAndModules(
	std::vector<std::tuple<std::string, DWORD>> processNameToPid,
	GetProcessModulesFunc mf,
	OUT std::tuple<BOOL, BOOL>& result
);

BOOL CheckObsChildWindow();
