#include "process_list_tasklist.h"

HRESULT GetProcessListTaskList(OUT std::vector<std::tuple<std::string, DWORD>>& processNamePidPairs) {
    FILE* pipe = _popen("tasklist /FO csv /NH", "r");
    if (!pipe) {
        return ERROR_PIPE_BUSY;
    }

    char buffer[260 + 10];

    processNamePidPairs.clear();
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        char* pPnSuffixPosition = strstr(buffer, "\",");
        if (!pPnSuffixPosition) {
            return ERROR_ILLEGAL_CHARACTER;
        }
        
        char* pPidPrefixPosition = pPnSuffixPosition + 2;
        char* pPidSuffixPosition = strstr(pPidPrefixPosition, "\",");
        if (!pPidSuffixPosition) {
            return ERROR_ILLEGAL_CHARACTER;
        }

        *pPnSuffixPosition = 0;

        // Skip the first quote
        *(pPidSuffixPosition++) = 0;
        
        // Skip the first quote
        std::string szProcessName = buffer + 1;
        int pid = atoi(pPidPrefixPosition);

        processNamePidPairs.push_back(std::make_tuple(szProcessName, pid));
    }

    // Close the pipe
    _pclose(pipe);
    return ERROR_SUCCESS;
}
