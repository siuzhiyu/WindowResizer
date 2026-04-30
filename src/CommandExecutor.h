#pragma once

#include "CmdParser.h"

class CommandExecutor
{
public:
    static bool Execute(const CommandLineOptions& options);
    static HWND LaunchAndWaitForWindow(const std::wstring& exePath, const CommandLineOptions& options, int timeoutMs);
    static HWND FindWindowForProcess(DWORD processId, int timeoutMs);
    static void SetWindowSize(HWND hwnd, int width, int height);
    static void CenterWindow(HWND hwnd);
    static void ToggleTitleBar(HWND hwnd, bool hide);
    static void MaximizeWindow(HWND hwnd);

private:
    static bool IsWindowValid(HWND hwnd);
    static BOOL CALLBACK EnumWindowsForProcess(HWND hwnd, LPARAM lParam);
};
