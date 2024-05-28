#pragma once

#include <Windows.h>

#include <vector>
#include <tuple>
#include <string>

HRESULT GetProcessListTaskList(OUT std::vector<std::tuple<std::string, DWORD>>& processNamePidPairs);
