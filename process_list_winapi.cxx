#include "process_list_winapi.h"

HRESULT GetProcessListWinApi(OUT std::vector<std::tuple<std::string, DWORD>>& processNamePidPairs) {
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return ERROR_INVALID_HANDLE;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return ERROR_ACCESS_DENIED;
    }

    processNamePidPairs.clear();
    do {
        char szExeFile[MAX_PATH]{ 0 };
        size_t dummy{};
        wcstombs_s(&dummy, szExeFile, pe32.szExeFile, sizeof(szExeFile));
        processNamePidPairs.push_back(std::make_tuple(szExeFile, pe32.th32ProcessID));
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return ERROR_SUCCESS;
}
