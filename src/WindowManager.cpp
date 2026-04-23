#include "WindowManager.h"
#include "App.h"
#include "Config.h"
#include "Utils.h"
#include "DirectXManager.h"
#include "Theme.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <windows.h>
#include <shellapi.h>

static HHOOK s_hMouseHook = nullptr;
static HWND s_hwndHitTarget = nullptr;
static HWND s_hwndMainView = nullptr;

void SetMainViewportWindow(HWND hwnd)
{
    s_hwndMainView = hwnd;
}

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && wParam == WM_LBUTTONDOWN && g_appState.bFindingWindow)
    {
        POINT pt = ((MSLLHOOKSTRUCT*)lParam)->pt;
        HWND hHit = WindowFromPoint(pt);
        if (hHit && (!s_hwndMainView || hHit != s_hwndMainView))
        {
            s_hwndHitTarget = hHit;
        }
    }
    return CallNextHookEx(s_hMouseHook, nCode, wParam, lParam);
}

void StartMouseHook(HWND hwnd)
{
    s_hwndMainView = hwnd;
    if (!s_hMouseHook)
    {
        s_hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, GetModuleHandle(NULL), 0);
    }
}

void StopMouseHook()
{
    if (s_hMouseHook)
    {
        UnhookWindowsHookEx(s_hMouseHook);
        s_hMouseHook = nullptr;
    }
    s_hwndMainView = nullptr;
}

void InitAppData()
{
}

bool IsWindowValid()
{
    if (g_appState.hTargetWindow == nullptr) return false;
    if (!IsWindow(g_appState.hTargetWindow)) return false;
    return true;
}

void UpdateSizeShow(int width, int height)
{
    g_appState.nCurWidth = width;
    g_appState.nCurHeight = height;
    g_appState.nCurrentWidth = width;
    g_appState.nCurrentHeight = height;
}

void FindWindowCallback()
{
    g_appState.bFindingWindow = true;
    s_hwndHitTarget = nullptr;
    StartMouseHook(g_hViewportWindow);
    SetCursor(LoadCursor(NULL, IDC_CROSS));
}

void HandleWindowSelection(HWND hwndHit)
{
    if (!hwndHit || (s_hwndMainView && hwndHit == s_hwndMainView)) return;

    g_appState.hTargetWindow = hwndHit;
    g_appState.bFindingWindow = false;
    StopMouseHook();
    SetCursor(LoadCursor(NULL, IDC_ARROW));

    wchar_t title[256] = L"";
    GetWindowTextW(hwndHit, title, 256);
    wcscpy_s(g_appState.szWindowTitle, title);
    ConvertToUTF8(title, g_appState.szWindowTitleUTF8, sizeof(g_appState.szWindowTitleUTF8));

    RECT rect;
    GetWindowRect(hwndHit, &rect);
    int detectedWidth = rect.right - rect.left;
    int detectedHeight = rect.bottom - rect.top;
    g_appState.nInitialWidth = detectedWidth;
    g_appState.nInitialHeight = detectedHeight;
    g_appState.nCurrentWidth = detectedWidth;
    g_appState.nCurrentHeight = detectedHeight;
    g_appState.nCurWidth = detectedWidth;
    g_appState.nCurHeight = detectedHeight;
    g_appState.nScaleBaseWidth = 0;
    g_appState.nScaleBaseHeight = 0;
    g_appState.nWidth = detectedWidth;
    g_appState.nHeight = detectedHeight;

    LONG style = GetWindowLong(hwndHit, GWL_STYLE);
    g_appState.bTitleBarHidden = !(style & WS_CAPTION);

    if (g_appState.bTitleBarHidden)
        g_appState.originalWindowStyle = style | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
    else
        g_appState.originalWindowStyle = style;

    double aspect = (double)detectedWidth / detectedHeight;
    if (aspect > 1.0)
    {
        g_appState.dLockedAspectRatio = aspect;
        g_appState.bHasAspectRatio = true;
    }
    else
    {
        g_appState.dLockedAspectRatio = 0.0;
        g_appState.bHasAspectRatio = false;
    }

    g_appState.bClickedSetPercentage = false;
    g_appState.bIsSetMaxSize = false;

    CheckWindowValidity();
}

void CheckWindowValidity()
{
    if (g_appState.hTargetWindow != nullptr && !IsWindow(g_appState.hTargetWindow))
    {
        g_appState.hTargetWindow = nullptr;
        g_appState.szWindowTitle[0] = L'\0';
        g_appState.szWindowTitleUTF8[0] = '\0';
        g_appState.nCurWidth = 0;
        g_appState.nCurHeight = 0;
        g_appState.nInitialWidth = 0;
        g_appState.nInitialHeight = 0;
        g_appState.nCurrentWidth = 0;
        g_appState.nCurrentHeight = 0;
        g_appState.nScaleBaseWidth = 0;
        g_appState.nScaleBaseHeight = 0;
        g_appState.nWidth = 0;
        g_appState.nHeight = 0;
        g_appState.bClickedSetPercentage = false;
        g_appState.bIsSetMaxSize = false;
        g_appState.bTitleBarHidden = false;
        g_appState.originalWindowStyle = 0;
    }
}

void ApplySelectedMode()
{
    if (!IsWindowValid()) return;

    int nWidth = 0, nHeight = 0;
    switch (g_appState.nModeSelect)
    {
    case 0:
        if (g_appState.nSelectedResolution >= 0 && g_appState.nSelectedResolution < g_arrResolutionsCount)
        {
            nWidth = g_arrResolutions[g_appState.nSelectedResolution].width;
            nHeight = g_arrResolutions[g_appState.nSelectedResolution].height;
            g_appState.nCurrentWidth = nWidth;
            g_appState.nCurrentHeight = nHeight;
        }
        g_appState.nScaleBaseWidth = 0;
        g_appState.nScaleBaseHeight = 0;
        break;
    case 1:
        nWidth = g_appState.nWidth;
        nHeight = g_appState.nHeight;
        if (nWidth <= 0 || nHeight <= 0) return;
        g_appState.nCurrentWidth = nWidth;
        g_appState.nCurrentHeight = nHeight;
        g_appState.nScaleBaseWidth = 0;
        g_appState.nScaleBaseHeight = 0;
        break;
    case 2:
        if (g_appState.nScaleBaseWidth == 0 || g_appState.nScaleBaseHeight == 0)
        {
            g_appState.nScaleBaseWidth = g_appState.nCurrentWidth;
            g_appState.nScaleBaseHeight = g_appState.nCurrentHeight;
        }
        nWidth = (int)(g_appState.nScaleBaseWidth * g_arrScaleValues[g_appState.nScaleIndex]);
        nHeight = (int)(g_appState.nScaleBaseHeight * g_arrScaleValues[g_appState.nScaleIndex]);
        g_appState.nCurrentWidth = nWidth;
        g_appState.nCurrentHeight = nHeight;
        break;
    case 3:
        if (g_appState.nScaleBaseWidth == 0 || g_appState.nScaleBaseHeight == 0)
        {
            g_appState.nScaleBaseWidth = g_appState.nCurrentWidth;
            g_appState.nScaleBaseHeight = g_appState.nCurrentHeight;
        }
        nWidth = (int)(g_appState.nScaleBaseWidth * g_appState.fCustomScale);
        nHeight = (int)(g_appState.nScaleBaseHeight * g_appState.fCustomScale);
        g_appState.nCurrentWidth = nWidth;
        g_appState.nCurrentHeight = nHeight;
        break;
    }

    if (nWidth > 0 && nHeight > 0)
    {
        SetWindowPos(g_appState.hTargetWindow, NULL, 0, 0, nWidth, nHeight, SWP_NOZORDER | SWP_NOMOVE);
        UpdateSizeShow(nWidth, nHeight);
    }
}

void CenterWindow()
{
    if (!IsWindowValid()) return;
    RECT windowRect;
    GetWindowRect(g_appState.hTargetWindow, &windowRect);

    HMONITOR hMonitor = MonitorFromWindow(g_appState.hTargetWindow, MONITOR_DEFAULTTONEAREST);
    MONITORINFO monitorInfo = {sizeof(monitorInfo)};
    GetMonitorInfo(hMonitor, &monitorInfo);

    int nX = monitorInfo.rcWork.left + ((monitorInfo.rcWork.right - monitorInfo.rcWork.left) - (windowRect.right - windowRect.left)) / 2;
    int nY = monitorInfo.rcWork.top + ((monitorInfo.rcWork.bottom - monitorInfo.rcWork.top) - (windowRect.bottom - windowRect.top)) / 2;
    SetWindowPos(g_appState.hTargetWindow, NULL, nX, nY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

void MaximizeWindow()
{
    if (!IsWindowValid()) return;
    if (IsZoomed(g_appState.hTargetWindow)) return;
    ShowWindow(g_appState.hTargetWindow, SW_SHOWMAXIMIZED);
    g_appState.bIsSetMaxSize = true;
    g_appState.bClickedSetPercentage = false;
}

void RestoreWindow()
{
    if (!IsWindowValid()) return;
    if (g_appState.bIsSetMaxSize)
        ShowWindow(g_appState.hTargetWindow, SW_RESTORE);

    SetWindowPos(g_appState.hTargetWindow, NULL, 0, 0, g_appState.nInitialWidth, g_appState.nInitialHeight, SWP_NOZORDER | SWP_NOMOVE);

    if (g_appState.bTitleBarHidden && g_appState.originalWindowStyle != 0)      
    {
        bool originalHasTitleBar = (g_appState.originalWindowStyle & WS_CAPTION) != 0;
        if (originalHasTitleBar)
        {
            SetWindowLong(g_appState.hTargetWindow, GWL_STYLE, g_appState.originalWindowStyle);
            SetWindowPos(g_appState.hTargetWindow, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
            g_appState.bTitleBarHidden = false;
        }
    }

    g_appState.nCurrentWidth = g_appState.nInitialWidth;
    g_appState.nCurrentHeight = g_appState.nInitialHeight;
    g_appState.nScaleBaseWidth = 0;
    g_appState.nScaleBaseHeight = 0;
    g_appState.nWidth = g_appState.nInitialWidth;
    g_appState.nHeight = g_appState.nInitialHeight;
    g_appState.nModeSelect = 0;

    RECT rect;
    GetWindowRect(g_appState.hTargetWindow, &rect);
    UpdateSizeShow(rect.right - rect.left, rect.bottom - rect.top);
    g_appState.bClickedSetPercentage = false;
}

void ToggleTitleBar()
{
    if (!IsWindowValid()) return;

    if (!g_appState.bTitleBarHidden)
    {
        if (g_appState.originalWindowStyle == 0)
            g_appState.originalWindowStyle = GetWindowLong(g_appState.hTargetWindow, GWL_STYLE);

        LONG newStyle = g_appState.originalWindowStyle & ~(WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
        SetWindowLong(g_appState.hTargetWindow, GWL_STYLE, newStyle);
        SetWindowPos(g_appState.hTargetWindow, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
        g_appState.bTitleBarHidden = true;
    }
    else
    {
        if (g_appState.originalWindowStyle != 0)
        {
            SetWindowLong(g_appState.hTargetWindow, GWL_STYLE, g_appState.originalWindowStyle);
            SetWindowPos(g_appState.hTargetWindow, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
            g_appState.bTitleBarHidden = false;
        }
    }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;

    if (g_appState.bFindingWindow && msg == WM_LBUTTONDOWN)
    {
        int x = (int)(short)LOWORD(lParam);
        int y = (int)(short)HIWORD(lParam);
        POINT pt = {x, y};
        ClientToScreen(hWnd, &pt);
        HWND hHit = WindowFromPoint(pt);
        if (hHit && hHit != hWnd)
        {
            HandleWindowSelection(hHit);
        }
        return 0;
    }

    switch (msg)
    {
    case WM_USER + 1:
        if (s_hwndHitTarget)
        {
            HandleWindowSelection(s_hwndHitTarget);
            s_hwndHitTarget = nullptr;
        }
        return 0;
    case WM_SIZE: if (wParam != SIZE_MINIMIZED) { g_ResizeWidth = (UINT)LOWORD(lParam); g_ResizeHeight = (UINT)HIWORD(lParam); } return 0;
    case WM_SYSCOMMAND: 
        if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
        if ((wParam & 0xfff0) == SC_CLOSE) { ::PostQuitMessage(0); return 0; }
        break;
    case WM_CLOSE: ::PostQuitMessage(0); return 0;
    case WM_DESTROY: StopMouseHook(); ::PostQuitMessage(0); return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

void RenderLoop()
{
    bool done = false;
    ImVec4 clear_color = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    bool needRender = true;

    while (!done)
    {
        MSG msg;
        bool hasMessage = false;
        
        BOOL result = ::GetMessage(&msg, NULL, 0, 0);
        if (result > 0)
        {
            hasMessage = true;
            needRender = true;
            
            if (msg.message == WM_SYSCOMMAND)
            {
                WPARAM cmd = msg.wParam & 0xfff0;
                if (cmd == SC_CLOSE)
                {
                    ::PostQuitMessage(0);
                    done = true;
                    continue;
                }
            }
            
            if (msg.message == WM_QUIT)
            {
                done = true;
                continue;
            }
            
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);

            if (s_hwndHitTarget && g_appState.bFindingWindow)
            {
                HandleWindowSelection(s_hwndHitTarget);
                s_hwndHitTarget = nullptr;
            }
        }
        else if (result == 0)
        {
            done = true;
            continue;
        }
        else
        {
            done = true;
            continue;
        }

        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            if (viewport && viewport->PlatformRequestClose)
            {
                done = true;
                break;
            }
        }

        ImGuiIO& io = ImGui::GetIO();

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            if (viewport && viewport->PlatformHandle)
            {
                HWND hwnd = (HWND)viewport->PlatformHandle;
                if (::IsIconic(hwnd))
                {
                    needRender = false;
                    continue;
                }
            }
        }

        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
            needRender = true;
        }

        CheckWindowValidity();
        if (g_appState.bThemeChanged) 
        {
            UpdateTheme();
            needRender = true;
        }

        if (needRender || io.WantCaptureMouse || io.WantCaptureKeyboard || io.WantTextInput)
        {
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();
            RenderUI();
            ImGui::Render();

            const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
            g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
            g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
            }

            g_pSwapChain->Present(1, 0);
            
            needRender = false;
        }
    }
}
