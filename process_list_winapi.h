#pragma once

#include <Windows.h>
#include <tlhelp32.h>

#include <vector>
#include <tuple>
#include <string>

HRESULT GetProcessListWinApi(OUT std::vector<std::tuple<std::string, DWORD>>& processNamePidPairs);
