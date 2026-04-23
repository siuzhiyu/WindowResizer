#include "Theme.h"
#include "App.h"
#include "Config.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include <windows.h>
#include <winreg.h>

bool IsSystemDarkMode()
{
    BOOL isDarkMode = FALSE;
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        DWORD value; DWORD size = sizeof(value);
        if (RegQueryValueEx(hKey, L"AppsUseLightTheme", NULL, NULL, (LPBYTE)&value, &size) == ERROR_SUCCESS)
            isDarkMode = (value == 0);
        RegCloseKey(hKey);
    }
    return isDarkMode;
}

void UpdateTheme()
{
    bool shouldBeDark = (g_appState.nThemeMode == 1) ? true : (g_appState.nThemeMode == 0) ? false : IsSystemDarkMode();
    if (g_appState.bIsDarkTheme != shouldBeDark)
    {
        g_appState.bIsDarkTheme = shouldBeDark;
        if (g_appState.bIsDarkTheme) ImGui::StyleColorsDark(); else ImGui::StyleColorsLight();

        ImGuiStyle& style = ImGui::GetStyle();
        float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY));
        style.ScaleAllSizes(main_scale);
        style.FontScaleDpi = main_scale * DebugConfig::DpiScaleMultiplier;
        style.WindowRounding = 8.0f;
        style.FrameRounding = 4.0f;
        style.GrabRounding = 4.0f;
        style.FramePadding = ImVec2(8, 4);
        style.ItemSpacing = ImVec2(8, 4);
        style.WindowPadding = ImVec2(12, 12);
    }
    g_appState.bThemeChanged = false;
}
