#include "process_modules.h"

std::string GetFileNameFromPath(const std::string& path) {
	size_t pos = path.find_last_of("\\/");
	return path.substr(pos + 1);
}

HRESULT GetProcessModulesSnapshot(DWORD dwProcessId, OUT std::vector<std::string>& moduleNames) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, dwProcessId);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return ERROR_INVALID_HANDLE;
    }

    MODULEENTRY32 me32{};
    me32.dwSize = sizeof(MODULEENTRY32);

    if (Module32First(hSnapshot, &me32)) {
        do {
            char szModuleName[MAX_PATH]{0};
            size_t dummy{};
            wcstombs_s(&dummy, szModuleName, me32.szModule, sizeof(szModuleName));
            moduleNames.push_back(szModuleName);
        } while (Module32Next(hSnapshot, &me32));
    }
    else {
        return ERROR_INVALID_HANDLE;
    }

    CloseHandle(hSnapshot);
    return ERROR_SUCCESS;
}

HRESULT GetProcessModulesWinApi(DWORD dwProcessId, OUT std::vector<std::string>& moduleNames) {
    HMODULE hMods[1024]{0};
    DWORD cbNeeded{};

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId);
    if (!hProcess) {
        return ERROR_ACCESS_DENIED;
    }

    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
        for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); ++i) {
            char szModName[MAX_PATH];
            if (GetModuleFileNameExA(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(char))) {
                moduleNames.push_back(GetFileNameFromPath(szModName));
            }
        }
    }
    else {
        return ERROR_INVALID_HANDLE;
    }
    return ERROR_SUCCESS;
}
