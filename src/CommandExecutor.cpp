#include "CommandExecutor.h"
#include "WindowFinder.h"
#include <iostream>
#include <process.h>

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
            std::wcout << L"正在启动程序并等待窗口... 超时: " << options.waitTimeout << L"ms\n";
            targetWindow = LaunchAndWaitForWindow(options.targetExePath, options.waitTimeout);
            if (!targetWindow)
            {
                std::wcerr << L"错误：启动程序后未找到窗口\n";
                return false;
            }
        }
        else
        {
            std::wcout << L"正在启动程序并无限等待窗口... (-nowait 模式)\n";
            targetWindow = LaunchAndWaitForWindow(options.targetExePath, -1);
            if (!targetWindow)
            {
                std::wcerr << L"错误：启动程序后未找到窗口\n";
                return false;
            }
        }
    }
    else if (!options.targetProcessName.empty())
    {
        std::wcout << L"正在通过进程名查找窗口: " << options.targetProcessName << L"\n";
        targetWindow = WindowFinder::FindWindowByProcessName(options.targetProcessName);
    }
    else if (!options.targetWindowTitle.empty())
    {
        std::wcout << L"正在通过窗口标题查找: " << options.targetWindowTitle << L"\n";
        targetWindow = WindowFinder::FindWindowByTitle(options.targetWindowTitle);
    }

    if (!targetWindow || !IsWindowValid(targetWindow))
    {
        std::wcerr << L"错误：未找到目标窗口\n";
        return false;
    }
    
    std::wcout << L"找到目标窗口\n";

    if (options.width > 0 && options.height > 0)
    {
        std::wcout << L"设置窗口大小: " << options.width << L"x" << options.height << L"\n";
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
        std::wcout << L"缩放窗口: " << currentWidth << L"x" << currentHeight << L" -> " << newWidth << L"x" << newHeight << L"\n";
        SetWindowSize(targetWindow, newWidth, newHeight);
    }

    if (options.centerWindow)
    {
        std::wcout << L"居中窗口\n";
        CenterWindow(targetWindow);
    }

    if (options.hideTitleBar)
    {
        std::wcout << L"隐藏标题栏\n";
        ToggleTitleBar(targetWindow, true);
    }

    if (options.showTitleBar)
    {
        std::wcout << L"显示标题栏\n";
        ToggleTitleBar(targetWindow, false);
    }

    if (options.maximize)
    {
        std::wcout << L"最大化窗口\n";
        MaximizeWindow(targetWindow);
    }

    std::wcout << L"操作完成\n";
    return true;
}

HWND CommandExecutor::LaunchAndWaitForWindow(const std::wstring& exePath, int timeoutMs)
{
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };

    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOWNORMAL;

    std::wstring cmdLine = L"\"" + exePath + L"\"";

    std::wstring workDir;
    size_t lastSlash = exePath.find_last_of(L'\\');
    if (lastSlash != std::wstring::npos)
    {
        workDir = exePath.substr(0, lastSlash);
    }

    if (!CreateProcessW(
        exePath.empty() ? NULL : exePath.c_str(),
        &cmdLine[0],
        NULL,
        NULL,
        FALSE,
        CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP,
        NULL,
        workDir.empty() ? NULL : workDir.c_str(),
        &si,
        &pi))
    {
        std::wcerr << L"错误：无法启动程序 " << exePath << L"\n";
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
        if (attempts % 5 == 0)
        {
            if (infiniteWait)
            {
                std::wcout << L"正在等待窗口... (尝试 " << attempts << L", 按 Ctrl+C 取消)\n";
            }
            else
            {
                std::wcout << L"正在等待窗口... (尝试 " << attempts << L")\n";
            }
        }
        Sleep(100);
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

void CommandExecutor::RestoreWindow(HWND hwnd)
{
    if (!IsWindowValid(hwnd))
        return;

    ShowWindow(hwnd, SW_RESTORE);
}
