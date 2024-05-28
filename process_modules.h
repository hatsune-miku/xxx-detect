#pragma once

#include <Windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <vector>
#include <string>

HRESULT GetProcessModulesSnapshot(DWORD dwProcessId, OUT std::vector<std::string>& moduleNames);
HRESULT GetProcessModulesWinApi(DWORD dwProcessId, OUT std::vector<std::string>& moduleNames);
