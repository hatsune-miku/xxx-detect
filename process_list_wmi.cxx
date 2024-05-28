#include <comdef.h>
#include <Wbemidl.h>

#include <vector>
#include <tuple>
#include <string>

#include "process_list_wmi.h"

#pragma comment(lib, "wbemuuid.lib")

HRESULT GetProcessListWmi(OUT std::vector<std::tuple<std::string, DWORD>>& processNamePidPairs) {
    HRESULT hr;

    hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        return hr;
    }

    hr = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL
    );
    if (FAILED(hr)) {
        CoUninitialize();
        return hr;
    }

    IWbemLocator* pLoc = NULL;
    hr = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc);
    if (FAILED(hr)) {
        CoUninitialize();
        return hr;
    }

    IWbemServices* pSvc = NULL;
    hr = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        NULL,
        NULL,
        0,
        NULL,
        0,
        0,
        &pSvc
    );
    if (FAILED(hr)) {
        pLoc->Release();
        CoUninitialize();
        return hr;
    }

    hr = CoSetProxyBlanket(
        pSvc,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        NULL,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE
    );
    if (FAILED(hr)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return hr;
    }

    IEnumWbemClassObject* pEnumerator = NULL;
    hr = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT ProcessId, ParentProcessId, Name FROM Win32_Process"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);
    if (FAILED(hr)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return hr;
    }

    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;

    processNamePidPairs.clear();
    while (pEnumerator) {
        hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (FAILED(hr)) {
            return hr;
        }
        if (uReturn == 0) {
            break;
        }
        std::string szProcessName;
        UINT uProcessId = 0;

        VARIANT vtProp{};
        VariantInit(&vtProp);
        hr = pclsObj->Get(L"ProcessId", 0, &vtProp, 0, 0);
        if (FAILED(hr)) {
            pSvc->Release();
            pLoc->Release();
            pEnumerator->Release();
            CoUninitialize();
            return hr;
        }
        uProcessId = vtProp.uintVal;
        VariantClear(&vtProp);

        hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
        if (FAILED(hr)) {
            pSvc->Release();
            pLoc->Release();
            pEnumerator->Release();
            CoUninitialize();
            return hr;
        }
        szProcessName = _com_util::ConvertBSTRToString(vtProp.bstrVal);
        VariantClear(&vtProp);

        pclsObj->Release();

        processNamePidPairs.push_back(std::make_tuple(szProcessName, uProcessId));
    }

    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();

    return 0;
}