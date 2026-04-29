#pragma once

#include <windows.h>
#include <string>
#include <vector>

class WindowFinder
{
public:
    static HWND FindWindowByTitle(const std::wstring& title, bool partialMatch = true);
    static HWND FindWindowByProcessName(const std::wstring& processName);
    
private:
    static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
    static DWORD GetProcessIdFromWindow(HWND hwnd);
    static std::wstring GetProcessNameFromId(DWORD processId);
};
