#include <map>
#include <string>
#include <algorithm>
#include <vector>
#include <tuple>
#include <iostream>

#include "obs_detect.h"
#include "process_list_tasklist.h"
#include "process_list_winapi.h"
#include "process_list_wmi.h"
#include "process_modules.h"
#include "enum_windows.h"

std::map<std::string, FARPROC> g_originFunctionAddresses{};
std::map<std::string, HMODULE> g_functionToModuleHandle{};
const std::vector<std::string> g_obsModules = {
	"obs.dll",
	"obs-browser.dll",
	"obs-ffmpeg.dll",
	"obs-filters.dll",
	"obs-outputs.dll",
};
const std::vector<std::string> g_obsProcesses = {
	"obs64.exe",
	"obs32.exe",
	"obs.exe",
};
const std::vector<std::string> g_obsWindowTitles = {
	"obs",
	"obs64",
};
const std::vector<std::string> g_obsWindowClassPatterns = {
	"Qt",
	"QWindowIcon",
};

HRESULT ObsDetectInitialize() {
	const HMODULE hKernel32 = GetModuleHandle(TEXT("kernel32.dll"));
	const HMODULE hNtdll = GetModuleHandle(TEXT("ntdll.dll"));
	const HMODULE hNtkrnlpa = GetModuleHandle(TEXT("ntdll.dll"));

	if (!hKernel32 || hNtdll || !hNtkrnlpa) {
		return ERROR_INVALID_HANDLE;
	}
	
	// 备份原始函数地址，用于 hook 检测
	// 如果后续出现了动态内存 patch，修改此处备份的值实现绕过检测，
	// 则 g_functionToModuleHandle 可以改为动态申请且定时更换内存地址
	g_functionToModuleHandle["CreateToolhelp32Snapshot"] = hKernel32;
	g_functionToModuleHandle["KiSystemService"] = hNtkrnlpa;
	g_functionToModuleHandle["NtQuerySystemInformation"] = hNtdll;
	g_functionToModuleHandle["ZwQuerySystemInformation"] = hNtdll;

	for (const auto& [szFunctionName, hModule] : g_functionToModuleHandle) {
		g_originFunctionAddresses[szFunctionName] =
			GetProcAddress(hModule, szFunctionName.c_str());
	}
	return ERROR_SUCCESS;
}

BOOL DidModuleAddressChange(const std::string& szFunctionName) {
	const HMODULE hModule = g_functionToModuleHandle[szFunctionName];
	return g_originFunctionAddresses[szFunctionName] != GetProcAddress(hModule, szFunctionName.c_str());
}

BOOL DidAnyModuleAddressChange() {
	return std::ranges::any_of(g_functionToModuleHandle, [] (const auto& pair) {
		return DidModuleAddressChange(pair.first);
	});
}

HRESULT CheckModuleLoaded(UINT uProcessId, GetProcessModulesFunc f, BOOL* pResult) {
	if (!pResult) {
		return ERROR_INVALID_HANDLE;
	}
	*pResult = FALSE;

	std::vector<std::string> moduleNames;
	if (FAILED(f(uProcessId, OUT moduleNames))) {
		return ERROR_ACCESS_DENIED;
	}

	*pResult = std::ranges::any_of(moduleNames, [](const std::string& szModulePath) {
		return std::ranges::any_of(g_obsModules, [szModulePath](const std::string& szObsModuleName) {
			return szModulePath.find(szObsModuleName) != std::string::npos;
		});
	});
	return ERROR_SUCCESS;
}

HRESULT CheckProcessAndModules(std::vector<std::tuple<std::string, DWORD>> processNameToPid, GetProcessModulesFunc mf, OUT std::tuple<BOOL, BOOL>& result) {
	BOOL bModuleMatched = FALSE;
	BOOL bProcessNameMatched = std::ranges::any_of(processNameToPid, [mf, &bModuleMatched](const auto& tuple) {
		const auto& [szProcessName, uPid] = tuple;

		BOOL bModuleTestResult = FALSE;
		CheckModuleLoaded(uPid, mf, &bModuleTestResult);
		bModuleMatched |= bModuleTestResult;

		return std::ranges::any_of(g_obsProcesses, [&szProcessName](const std::string& szObsProcess) {
			return szProcessName == szObsProcess;
		});
	});
	result = std::make_tuple(bProcessNameMatched, bModuleMatched);
	return ERROR_SUCCESS;
}

BOOL CheckObsChildWindow() {
	BOOL ret = FALSE;
	auto proc = [&](HWND hwnd, const std::string& dwWindowText, const std::string& dwClassName) {
		UNREFERENCED_PARAMETER(hwnd);
		if (std::ranges::any_of(g_obsWindowClassPatterns, [&](const std::string& pattern) {
			return std::ranges::any_of(g_obsWindowTitles, [&](const std::string& title) {
				return dwClassName.find(pattern) != std::string::npos && dwWindowText == title;
			});
		})) {
			ret = TRUE;
		}
	};
	EnumChildWindows(3, proc);
	return ret;
}
