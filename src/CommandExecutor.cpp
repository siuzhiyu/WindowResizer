#include "CommandExecutor.h"
#include "WindowFinder.h"
#include "CmdParser.h"
#include "WindowManager.h"
#include <iostream>
#include <process.h>
#include <cstdio>
#include <sstream>
#include <vector>

namespace
{
    const int DEFAULT_WAIT_TIMEOUT_MS = 3000;
    const int WINDOW_POLL_INTERVAL_MS = 100;
    const int LOG_ATTEMPT_INTERVAL = 5;
    const int WINDOW_TITLE_BUFFER_SIZE = 256;
}

// 辅助函数：使用WriteConsoleW输出（避免编码问题）
static void WriteOut(const std::wstring& text)
{
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdout != INVALID_HANDLE_VALUE)
    {
        DWORD written;
        WriteConsoleW(hStdout, text.c_str(), (DWORD)text.length(), &written, NULL);
    }
}

static void WriteErr(const std::wstring& text)
{
    HANDLE hStderr = GetStdHandle(STD_ERROR_HANDLE);
    if (hStderr != INVALID_HANDLE_VALUE)
    {
        DWORD written;
        WriteConsoleW(hStderr, text.c_str(), (DWORD)text.length(), &written, NULL);
    }
}

static void WriteBilingualOut(const wchar_t* zh, const wchar_t* en)
{
    std::wstringstream ss;
    if (g_cmdLang == Language::Chinese) ss << zh << std::endl;
    else ss << en << std::endl;
    WriteOut(ss.str());
}

static void WriteBilingualErr(const wchar_t* zh, const wchar_t* en)
{
    std::wstringstream ss;
    if (g_cmdLang == Language::Chinese) ss << zh << std::endl;
    else ss << en << std::endl;
    WriteErr(ss.str());
}

struct WaitForWindowData
{
    DWORD processId;
    HWND foundWindow;
    bool found;
};

bool CommandExecutor::Execute(const CommandLineOptions& options)
{
    if (!options.isValid)
        return false;

    HWND targetWindow = nullptr;

    if (!options.targetExePath.empty())
    {
        if (options.waitForWindow)
        {
            std::wstring zh = L"正在启动程序并等待窗口... 超时: " + std::to_wstring(options.waitTimeout) + L"ms";
            std::wstring en = L"Launching program and waiting for window... timeout: " + std::to_wstring(options.waitTimeout) + L"ms";
            WriteBilingualOut(zh.c_str(), en.c_str());
            targetWindow = LaunchAndWaitForWindow(options.targetExePath, options, options.waitTimeout);
            if (!targetWindow)
            {
                WriteBilingualErr(L"错误：启动程序后未找到窗口", L"Error: window not found after launch");
                return false;
            }
        }
        else
        {
            WriteBilingualOut(L"正在启动程序并无限等待窗口... (-nowait 模式)", L"Launching program and infinite waiting... (-nowait mode)");
            targetWindow = LaunchAndWaitForWindow(options.targetExePath, options, -1);
            if (!targetWindow)
            {
                WriteBilingualErr(L"错误：启动程序后未找到窗口", L"Error: window not found after launch");
                return false;
            }
        }
    }
    else if (!options.targetProcessName.empty())
    {
        std::wstring zh = L"正在通过进程名查找窗口: " + options.targetProcessName;
        std::wstring en = L"Finding window by process name: " + options.targetProcessName;
        WriteBilingualOut(zh.c_str(), en.c_str());
        targetWindow = WindowFinder::FindWindowByProcessName(options.targetProcessName);
    }
    else if (!options.targetWindowTitle.empty())
    {
        std::wstring zh = L"正在通过窗口标题查找: " + options.targetWindowTitle;
        std::wstring en = L"Finding window by title: " + options.targetWindowTitle;
        WriteBilingualOut(zh.c_str(), en.c_str());
        targetWindow = WindowFinder::FindWindowByTitle(options.targetWindowTitle);
    }

    if (!targetWindow || !IsWindowValid(targetWindow))
    {
        WriteBilingualErr(L"错误：未找到目标窗口", L"Error: target window not found");
        return false;
    }
    
    WriteBilingualOut(L"找到目标窗口", L"Target window found");

    if (options.width > 0 && options.height > 0)
    {
        std::wstring zh = L"设置窗口大小: " + std::to_wstring(options.width) + L"x" + std::to_wstring(options.height);
        std::wstring en = L"Setting window size: " + std::to_wstring(options.width) + L"x" + std::to_wstring(options.height);
        WriteBilingualOut(zh.c_str(), en.c_str());
        SetWindowSize(targetWindow, options.width, options.height);
    }

    if (options.scale > 0.0f)
    {
        RECT rect;
        GetWindowRect(targetWindow, &rect);
        int currentWidth = rect.right - rect.left;
        int currentHeight = rect.bottom - rect.top;
        int newWidth = (int)(currentWidth * options.scale);
        int newHeight = (int)(currentHeight * options.scale);
        std::wstring zh = L"缩放窗口: " + std::to_wstring(currentWidth) + L"x" + std::to_wstring(currentHeight) + L" -> " + std::to_wstring(newWidth) + L"x" + std::to_wstring(newHeight);
        std::wstring en = L"Scaling window: " + std::to_wstring(currentWidth) + L"x" + std::to_wstring(currentHeight) + L" -> " + std::to_wstring(newWidth) + L"x" + std::to_wstring(newHeight);
        WriteBilingualOut(zh.c_str(), en.c_str());
        SetWindowSize(targetWindow, newWidth, newHeight);
    }

    if (options.centerWindow)
    {
        WriteBilingualOut(L"居中窗口", L"Centering window");
        CenterWindow(targetWindow);
    }

    if (options.hideTitleBar)
    {
        WriteBilingualOut(L"隐藏标题栏", L"Hiding title bar");
        ToggleTitleBar(targetWindow, true);
    }

    if (options.showTitleBar)
    {
        WriteBilingualOut(L"显示标题栏", L"Showing title bar");
        ToggleTitleBar(targetWindow, false);
    }

    if (options.maximize)
    {
        WriteBilingualOut(L"最大化窗口", L"Maximizing window");
        MaximizeWindow(targetWindow);
    }

    WriteBilingualOut(L"操作完成", L"Operation completed");
    return true;
}

HWND CommandExecutor::LaunchAndWaitForWindow(const std::wstring& exePath, const CommandLineOptions& options, int timeoutMs)
{
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };

    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOWNORMAL;

    std::wstring cmdLine = L"\"" + exePath + L"\"";
    if (!options.targetExeArgs.empty())
    {
        cmdLine += L" " + options.targetExeArgs;
    }
    std::vector<wchar_t> cmdLineBuffer(cmdLine.begin(), cmdLine.end());
    cmdLineBuffer.push_back(L'\0');

    std::wstring workDir;
    size_t lastSlash = exePath.find_last_of(L'\\');
    if (lastSlash != std::wstring::npos)
    {
        workDir = exePath.substr(0, lastSlash);
    }

    BOOL success = CreateProcessW(
        exePath.empty() ? NULL : exePath.c_str(),
        cmdLineBuffer.data(),
        NULL,
        NULL,
        FALSE,
        CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP,
        NULL,
        workDir.empty() ? NULL : workDir.c_str(),
        &si,
        &pi);
    if (!success)
    {
        std::wstring zh = L"错误：无法启动程序 " + exePath;
        std::wstring en = L"Error: failed to launch " + exePath;
        WriteBilingualErr(zh.c_str(), en.c_str());
        return nullptr;
    }

    CloseHandle(pi.hThread);

    HWND hwnd = FindWindowForProcess(pi.dwProcessId, timeoutMs);

    CloseHandle(pi.hProcess);

    return hwnd;
}

HWND CommandExecutor::FindWindowForProcess(DWORD processId, int timeoutMs)
{
    WaitForWindowData data;
    data.processId = processId;
    data.foundWindow = nullptr;
    data.found = false;

    DWORD startTime = GetTickCount();
    int attempts = 0;
    bool infiniteWait = (timeoutMs < 0);

    while (!data.found && (infiniteWait || (GetTickCount() - startTime < (DWORD)timeoutMs)))
    {
        data.foundWindow = nullptr;
        EnumWindows(EnumWindowsForProcess, reinterpret_cast<LPARAM>(&data));

        if (data.found)
            break;

        attempts++;
        if (attempts % LOG_ATTEMPT_INTERVAL == 0)
        {
            if (infiniteWait)
            {
                std::wstring zh = L"正在等待窗口... (尝试 " + std::to_wstring(attempts) + L", 按 Ctrl+C 取消)";
                std::wstring en = L"Waiting for window... (attempt " + std::to_wstring(attempts) + L", Ctrl+C to cancel)";
                WriteBilingualOut(zh.c_str(), en.c_str());
            }
            else
            {
                std::wstring zh = L"正在等待窗口... (尝试 " + std::to_wstring(attempts) + L")";
                std::wstring en = L"Waiting for window... (attempt " + std::to_wstring(attempts) + L")";
                WriteBilingualOut(zh.c_str(), en.c_str());
            }
        }
        Sleep(WINDOW_POLL_INTERVAL_MS);
    }

    return data.foundWindow;
}

BOOL CALLBACK CommandExecutor::EnumWindowsForProcess(HWND hwnd, LPARAM lParam)
{
    WaitForWindowData* pData = reinterpret_cast<WaitForWindowData*>(lParam);

    if (!IsWindowVisible(hwnd))
        return TRUE;

    DWORD windowProcessId = 0;
    GetWindowThreadProcessId(hwnd, &windowProcessId);

    if (windowProcessId == pData->processId)
    {
        LONG style = GetWindowLong(hwnd, GWL_STYLE);
        if ((style & WS_OVERLAPPEDWINDOW) || (style & WS_POPUP))
        {
            pData->foundWindow = hwnd;
            pData->found = true;
            return FALSE;
        }
    }

    return TRUE;
}

bool CommandExecutor::IsWindowValid(HWND hwnd)
{
    return hwnd != nullptr && IsWindow(hwnd) && IsWindowVisible(hwnd);
}

void CommandExecutor::SetWindowSize(HWND hwnd, int width, int height)
{
    if (!IsWindowValid(hwnd))
        return;

    SetWindowPos(hwnd, NULL, 0, 0, width, height, SWP_NOZORDER | SWP_NOMOVE);
}

void CommandExecutor::CenterWindow(HWND hwnd)
{
    if (!IsWindowValid(hwnd))
        return;

    RECT windowRect;
    GetWindowRect(hwnd, &windowRect);

    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO monitorInfo = { sizeof(monitorInfo) };
    GetMonitorInfo(hMonitor, &monitorInfo);

    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;

    int nX = monitorInfo.rcWork.left + ((monitorInfo.rcWork.right - monitorInfo.rcWork.left) - windowWidth) / 2;
    int nY = monitorInfo.rcWork.top + ((monitorInfo.rcWork.bottom - monitorInfo.rcWork.top) - windowHeight) / 2;

    SetWindowPos(hwnd, NULL, nX, nY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

void CommandExecutor::ToggleTitleBar(HWND hwnd, bool hide)
{
    if (!IsWindowValid(hwnd))
        return;

    LONG currentStyle = GetWindowLong(hwnd, GWL_STYLE);

    if (hide)
    {
        LONG newStyle = currentStyle & ~(WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
        SetWindowLong(hwnd, GWL_STYLE, newStyle);
    }
    else
    {
        LONG newStyle = currentStyle | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
        SetWindowLong(hwnd, GWL_STYLE, newStyle);
    }

    SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
}

void CommandExecutor::MaximizeWindow(HWND hwnd)
{
    if (!IsWindowValid(hwnd))
        return;

    if (!IsZoomed(hwnd))
    {
        ShowWindow(hwnd, SW_SHOWMAXIMIZED);
    }
}
