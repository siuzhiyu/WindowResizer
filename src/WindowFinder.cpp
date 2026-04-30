#include "WindowFinder.h"
#include <psapi.h>
#include <algorithm>
#include <vector>

#pragma comment(lib, "psapi.lib")

HWND WindowFinder::FindWindowByProcessName(const std::wstring& processName)
{
    if (processName.empty())
        return nullptr;

    std::wstring targetProcessName = processName;
    if (targetProcessName.find(L'.') == std::wstring::npos)
    {
        targetProcessName += L".exe";
    }

    std::transform(targetProcessName.begin(), targetProcessName.end(), targetProcessName.begin(), ::towlower);

    std::vector<HWND> windows;
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        auto& windows = *reinterpret_cast<std::vector<HWND>*>(lParam);
        
        if (!IsWindowVisible(hwnd))
            return TRUE;

        // 排除控制台和终端窗口
        wchar_t className[256];
        GetClassNameW(hwnd, className, sizeof(className) / sizeof(wchar_t));
        if (wcscmp(className, L"ConsoleWindowClass") == 0 ||
            wcscmp(className, L"Windows.UI.Xaml.Hosting.DesktopWindowXamlSource") == 0 ||
            wcscmp(className, L"WindowsForms10.Window.8.app.0.378734a") == 0)
            return TRUE;

        // 只检查基本窗口样式，不限制窗口类型
        LONG style = GetWindowLong(hwnd, GWL_STYLE);
        if (!(style & WS_VISIBLE))
            return TRUE;

        windows.push_back(hwnd);
        return TRUE;
    }, reinterpret_cast<LPARAM>(&windows));

    for (HWND hwnd : windows)
    {
        DWORD processId = GetProcessIdFromWindow(hwnd);
        std::wstring name = GetProcessNameFromId(processId);
        
        std::transform(name.begin(), name.end(), name.begin(), ::towlower);

        // 只支持严格的进程名匹配
        if (name == targetProcessName)
        {
            return hwnd;
        }
    }

    return nullptr;
}

DWORD WindowFinder::GetProcessIdFromWindow(HWND hwnd)
{
    DWORD processId = 0;
    GetWindowThreadProcessId(hwnd, &processId);
    return processId;
}

std::wstring WindowFinder::GetProcessNameFromId(DWORD processId)
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (!hProcess)
        return L"";

    wchar_t processName[MAX_PATH];
    DWORD size = sizeof(processName);

    if (QueryFullProcessImageNameW(hProcess, 0, processName, &size))
    {
        std::wstring fullPath(processName);
        size_t lastBackslash = fullPath.find_last_of(L'\\');
        if (lastBackslash != std::wstring::npos)
        {
            CloseHandle(hProcess);
            return fullPath.substr(lastBackslash + 1);
        }
    }

    CloseHandle(hProcess);
    return L"";
}
