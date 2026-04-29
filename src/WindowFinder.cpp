#include "WindowFinder.h"
#include <psapi.h>
#include <algorithm>
#include <vector>

#pragma comment(lib, "psapi.lib")

namespace
{
    const int WINDOW_TITLE_BUFFER_SIZE = 256;
}

struct FindWindowData
{
    std::wstring targetTitle;
    bool partialMatch;
    HWND foundWindow;
};

HWND WindowFinder::FindWindowByTitle(const std::wstring& title, bool partialMatch)
{
    if (title.empty())
        return nullptr;

    FindWindowData data;
    data.targetTitle = title;
    data.partialMatch = partialMatch;
    data.foundWindow = nullptr;

    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&data));

    return data.foundWindow;
}

BOOL CALLBACK WindowFinder::EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    FindWindowData* data = reinterpret_cast<FindWindowData*>(lParam);

    if (!IsWindowVisible(hwnd))
        return TRUE;

    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    if (!(style & WS_OVERLAPPEDWINDOW) && !(style & WS_POPUP))
        return TRUE;

    wchar_t windowTitle[WINDOW_TITLE_BUFFER_SIZE];
    GetWindowTextW(hwnd, windowTitle, sizeof(windowTitle) / sizeof(wchar_t));

    std::wstring title(windowTitle);
    std::wstring target = data->targetTitle;

    std::transform(title.begin(), title.end(), title.begin(), ::towlower);
    std::transform(target.begin(), target.end(), target.begin(), ::towlower);

    if (data->partialMatch)
    {
        if (title.find(target) != std::wstring::npos)
        {
            data->foundWindow = hwnd;
            return FALSE;
        }
    }
    else
    {
        if (title == target)
        {
            data->foundWindow = hwnd;
            return FALSE;
        }
    }

    return TRUE;
}

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

        LONG style = GetWindowLong(hwnd, GWL_STYLE);
        if (!(style & WS_OVERLAPPEDWINDOW) && !(style & WS_POPUP))
            return TRUE;

        windows.push_back(hwnd);
        return TRUE;
    }, reinterpret_cast<LPARAM>(&windows));

    for (HWND hwnd : windows)
    {
        DWORD processId = GetProcessIdFromWindow(hwnd);
        std::wstring name = GetProcessNameFromId(processId);
        
        std::transform(name.begin(), name.end(), name.begin(), ::towlower);

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
