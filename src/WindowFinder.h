#pragma once

#include <windows.h>
#include <string>
#include <vector>

class WindowFinder
{
public:
    static HWND FindWindowByProcessName(const std::wstring& processName);
    
private:
    static DWORD GetProcessIdFromWindow(HWND hwnd);
    static std::wstring GetProcessNameFromId(DWORD processId);
};
