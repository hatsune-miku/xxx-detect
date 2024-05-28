#include <Windows.h>

#include <iostream>
#include <vector>
#include <string>

#include "enum_windows.h"

BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam);

struct EnumData {
    int indentLevel;
    int maxDepth;
    EnumWindowCallback callback;
};

void ProcessChildWindow(HWND hwnd, int indentLevel, const EnumData* data) {
    UNREFERENCED_PARAMETER(indentLevel);

    char windowText[256];
    char className[256];

    GetWindowTextA(hwnd, windowText, sizeof(windowText) / sizeof(char));
    GetClassNameA(hwnd, className, sizeof(className) / sizeof(char));

    data->callback(hwnd, windowText, className);
}

BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam) {
    const EnumData* data = reinterpret_cast<EnumData*>(lParam);
    ProcessChildWindow(hwnd, data->indentLevel, data);

    if (data->indentLevel >= data->maxDepth) {
        return TRUE;
    }

    const EnumData newData{
        .indentLevel = data->indentLevel + 1,
        .maxDepth = data->maxDepth,
        .callback = data->callback,
    };
    EnumChildWindows(hwnd, EnumChildProc, reinterpret_cast<LPARAM>(&newData));
    return TRUE;
}

void EnumChildWindows(int maxDepth, EnumWindowCallback callback) {
    EnumData data{
        .indentLevel = 0,
        .maxDepth = maxDepth,
        .callback = callback,
    };
    EnumWindows(EnumChildProc, reinterpret_cast<LPARAM>(&data));
}
