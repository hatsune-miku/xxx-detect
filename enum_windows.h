#pragma once

#include <functional>

using EnumWindowCallback = std::function<void (HWND, const std::string&, const std::string&)>;

void EnumChildWindows(int maxDepth, EnumWindowCallback callback);
