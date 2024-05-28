#include "obs_detect.h"

#include <iostream>
#include <chrono>

int main(int, const char**) {
	std::cout << "Initializing...\n";

	if (FAILED(ObsDetectInitialize())) {
		std::cout << "Failed to initialize OBS detection.\n";
		return 1;
	}

	std::cout << "Initialized. Press any key to start detection...\n";
	system("pause > nul");
	
	std::cout << "Detecting...\n";
	auto startTime = std::chrono::high_resolution_clock::now();

	BOOL bSecurityCheckFailed = DidAnyModuleAddressChange();
	auto results = std::make_tuple(FALSE, FALSE);
	std::vector<std::tuple<std::string, DWORD>> processList;

	BOOL bWinApiListSucceeded = SUCCEEDED(GetProcessListWinApi(OUT processList));
	BOOL bWinApiCheckSnapshotSucceeded = SUCCEEDED(CheckProcessAndModules(processList, GetProcessModulesSnapshot, results));
	BOOL bProcessCheckFailedWinApi = std::get<0>(results);
	BOOL bWinApiModuleCheckFailedSnapshot = std::get<1>(results);
	BOOL bWinApiCheckWinApiSucceeded = SUCCEEDED(CheckProcessAndModules(processList, GetProcessModulesWinApi, results));
	BOOL bWinApiModuleCheckFailedWinApi = std::get<1>(results);

	BOOL bWmiListSucceeded = SUCCEEDED(GetProcessListWmi(OUT processList));
	BOOL bWmiCheckSnapshotSucceeded = SUCCEEDED(CheckProcessAndModules(processList, GetProcessModulesSnapshot, results));
	BOOL bProcessCheckFailedWmi = std::get<0>(results);
	BOOL bWmiModuleCheckFailedSnapshot = std::get<1>(results);
	BOOL bWmiCheckWinApiSucceeded = SUCCEEDED(CheckProcessAndModules(processList, GetProcessModulesWinApi, results));
	BOOL bWmiModuleCheckFailedWinApi = std::get<1>(results);

	BOOL bTaskListListSucceeded = SUCCEEDED(GetProcessListTaskList(OUT processList));
	BOOL bTaskListCheckSnapshotSucceed = SUCCEEDED(CheckProcessAndModules(processList, GetProcessModulesSnapshot, results));
	BOOL bProcessCheckFailedTaskList = std::get<0>(results);
	BOOL bTaskListModuleCheckFailedSnapshot = std::get<1>(results);
	BOOL bTaskListCheckWinApiSucceeded = SUCCEEDED(CheckProcessAndModules(processList, GetProcessModulesWinApi, results));
	BOOL bTaskListModuleCheckFailedWinApi = std::get<1>(results);

	BOOL bWindowCheckFailed = CheckObsChildWindow();

	// 只要其中一项为阳性，就可以实锤了
	BOOL bIsFinalResultPositive = 
		bSecurityCheckFailed 
		|| bProcessCheckFailedWinApi 
		|| bWinApiModuleCheckFailedSnapshot 
		|| bWinApiModuleCheckFailedWinApi
		|| bProcessCheckFailedWmi 
		|| bWmiModuleCheckFailedSnapshot
		|| bWmiModuleCheckFailedWinApi 
		|| bProcessCheckFailedTaskList 
		|| bTaskListModuleCheckFailedSnapshot 
		|| bTaskListModuleCheckFailedWinApi
		|| bWindowCheckFailed;

	auto timeElapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();

	std::cout << "* Security Check            : " << (bSecurityCheckFailed ? "FAILED" : "PASSED") << "\n";

	std::cout << "\n* ==== Process List Method: WinApi ====\n";
	std::cout << "* API Call #1               : " << (bWinApiListSucceeded ? "SUCCESS" : "FAILED") << "\n";
	std::cout << "* API Call #2               : " << (bWinApiCheckSnapshotSucceeded ? "SUCCESS" : "FAILED") << "\n";
	std::cout << "* API Call #3               : " << (bWinApiCheckWinApiSucceeded ? "SUCCESS" : "FAILED") << "\n";
	if (bWinApiListSucceeded && bWinApiCheckSnapshotSucceeded && bWinApiCheckWinApiSucceeded) {
		std::cout << "* Process Check             : " << (bProcessCheckFailedWinApi ? "[!] POSITIVE" : "[-] NEGATIVE") << "\n";
		std::cout << "* Module Scan (Snapshot)    : " << (bWinApiModuleCheckFailedSnapshot ? "[!] POSITIVE" : "[-] NEGATIVE") << "\n";
		std::cout << "* Module Scan (OpenProcess) : " << (bWinApiModuleCheckFailedWinApi ? "[!] POSITIVE" : "[-] NEGATIVE") << "\n";
	}

	std::cout << "\n* ==== Process List Method: WMI ====\n";
	std::cout << "* API Call #1               : " << (bWmiListSucceeded ? "SUCCESS" : "FAILED") << "\n";
	std::cout << "* API Call #2               : " << (bWmiCheckSnapshotSucceeded ? "SUCCESS" : "FAILED") << "\n";
	std::cout << "* API Call #3               : " << (bWmiCheckWinApiSucceeded ? "SUCCESS" : "FAILED") << "\n";
	if (bWmiListSucceeded && bWmiCheckSnapshotSucceeded && bWmiCheckWinApiSucceeded) {
		std::cout << "* Process Check             : " << (bProcessCheckFailedWmi ? "[!] POSITIVE" : "[-] NEGATIVE") << "\n";
		std::cout << "* Module Scan (Snapshot)    : " << (bWmiModuleCheckFailedSnapshot ? "[!] POSITIVE" : "[-] NEGATIVE") << "\n";
		std::cout << "* Module Scan (OpenProcess) : " << (bWmiModuleCheckFailedWinApi ? "[!] POSITIVE" : "[-] NEGATIVE") << "\n";
	}

	std::cout << "\n* ==== Process List Method: TaskList ====\n";
	std::cout << "* API Call #1               : " << (bTaskListListSucceeded ? "SUCCESS" : "FAILED") << "\n";
	std::cout << "* API Call #2               : " << (bTaskListCheckSnapshotSucceed ? "SUCCESS" : "FAILED") << "\n";
	std::cout << "* API Call #3               : " << (bTaskListCheckWinApiSucceeded ? "SUCCESS" : "FAILED") << "\n";
	if (bTaskListListSucceeded && bTaskListCheckSnapshotSucceed && bTaskListCheckWinApiSucceeded) {
		std::cout << "* Process Check             : " << (bProcessCheckFailedTaskList ? "[!] POSITIVE" : "[-] NEGATIVE") << "\n";
		std::cout << "* Module Scan (Snapshot)    : " << (bTaskListModuleCheckFailedSnapshot ? "[!] POSITIVE" : "[-] NEGATIVE") << "\n";
		std::cout << "* Module Scan (OpenProcess) : " << (bTaskListModuleCheckFailedWinApi ? "[!] POSITIVE" : "[-] NEGATIVE") << "\n";
	}

	std::cout << "\n* ==== Window & Child Window ====\n";
	std::cout << "* Scan Result               : " << (bWindowCheckFailed ? "[!] POSITIVE" : "[-] NEGATIVE") << "\n";

	std::cout << "\n* ==== Overall ====\n";
	std::cout << "* Final Result              : " << (bIsFinalResultPositive ? "[!] POSITIVE" : "[-] NEGATIVE") << "\n";

	std::cout << "\n* Time Elapsed: " << timeElapsedMillis << "ms\n";
	std::cout << "Press any key to exit...\n";
	system("pause > nul");
	return 0;
}
