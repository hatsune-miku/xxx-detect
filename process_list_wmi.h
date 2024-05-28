#pragma once

#include <Windows.h>

#include <vector>
#include <tuple>
#include <string>

HRESULT GetProcessListWmi(OUT std::vector<std::tuple<std::string, DWORD>>& processNamePidPairs);
