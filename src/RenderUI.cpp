#include "RenderUI.h"
#include "LanguageManager.h"
#include <imgui.h>
#include <windows.h>
#include <shellapi.h>
#include <cstdio>

void RenderSettingsPopup();
void RenderAboutPopup();
void RenderHelpPopup();
void RenderLeftPanel();
void RenderRightPanel();

void RenderUI()
{
    static bool open = true;
    static bool firstFrame = true;
    static bool s_dwmInitialized = false;
    static HMODULE s_hDwmapi = nullptr;
    static HRESULT(WINAPI* s_pDwmSetWindowAttribute)(HWND, DWORD, LPCVOID, DWORD) = nullptr;
    if (!s_dwmInitialized)
    {
        s_dwmInitialized = true;
        s_hDwmapi = LoadLibrary(L"dwmapi.dll");
        if (s_hDwmapi)
        {
            s_pDwmSetWindowAttribute = (decltype(s_pDwmSetWindowAttribute))GetProcAddress(s_hDwmapi, "DwmSetWindowAttribute");
        }
    }
    if (firstFrame)
    {
        firstFrame = false;
        ImVec2 windowSize = ImVec2(545, 580);
        if (g_appState.nWindowX == -1 || g_appState.nWindowY == -1)
        {
            POINT pt = {0, 0};
            HMONITOR hMonitor = ::MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
            MONITORINFO mi = {sizeof(mi)};
            if (hMonitor && ::GetMonitorInfo(hMonitor, &mi))
            {
                int workW = mi.rcWork.right - mi.rcWork.left;
                int workH = mi.rcWork.bottom - mi.rcWork.top;
                ImGui::SetNextWindowPos(ImVec2(mi.rcWork.left + (workW - windowSize.x) * 0.5f, mi.rcWork.top + (workH - windowSize.y) * 0.5f), ImGuiCond_Once);
            }
            else
            {
                ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_Once);
            }
        }
        else
        {
            ImGui::SetNextWindowPos(ImVec2((float)g_appState.nWindowX, (float)g_appState.nWindowY), ImGuiCond_Once);
        }
    }

    ImGui::SetNextWindowSize(ImVec2(545, 580), ImGuiCond_Always);

    // 设置窗口圆角样式
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 12));
    
    // 美化颜色
    if (g_appState.bIsDarkTheme)
    {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.12f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.14f, 0.14f, 0.16f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.20f, 0.20f, 0.22f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.25f, 0.25f, 0.27f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.30f, 0.30f, 0.32f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.30f, 0.30f, 0.35f, 0.50f));
        ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0.00f, 0.00f, 0.00f, 0.00f));
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.95f, 0.95f, 0.97f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.90f, 0.90f, 0.92f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.85f, 0.85f, 0.87f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.80f, 0.80f, 0.82f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.75f, 0.75f, 0.77f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.70f, 0.70f, 0.75f, 0.50f));
        ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0.00f, 0.00f, 0.00f, 0.00f));
    }

    ImGui::Begin("Window Resizer", &open,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoResize
    );

    ImGuiViewport* vp = ImGui::GetWindowViewport();
    HWND hwnd = (HWND)vp->PlatformHandle;
    g_hViewportWindow = hwnd;

    // 在第一次渲染时设置窗口圆角
    static bool s_windowRounded = false;
    if (!s_windowRounded && hwnd)
    {
        s_windowRounded = true;
        
        typedef enum DWMWINDOWATTRIBUTE {
            DWMWA_WINDOW_CORNER_PREFERENCE = 33
        } DWMWINDOWATTRIBUTE;
        
        typedef enum DWM_WINDOW_CORNER_PREFERENCE {
            DWMWCP_DEFAULT = 0,
            DWMWCP_DONOTROUND = 1,
            DWMWCP_ROUND = 2,
            DWMWCP_ROUNDSMALL = 3
        } DWM_WINDOW_CORNER_PREFERENCE;
        
        if (s_pDwmSetWindowAttribute)
        {
            DWM_WINDOW_CORNER_PREFERENCE preference = DWMWCP_ROUND;
            s_pDwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &preference, sizeof(preference));
        }
    }

    const float title_bar_height = 28.0f;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 0));

    // 标题文字往右挪一点，增加左边距
    ImGui::SetCursorPosX(16);
    ImGui::Text("Window Resizer-imgui v1.1");

    float button_width = 24;
    float button_spacing = 8; // 按钮之间的间距
    // 按钮往左挪，增加右边距
    ImGui::SameLine(ImGui::GetWindowWidth() - button_width * 2 - button_spacing - 16);

    if (ImGui::Button("_", ImVec2(button_width, button_width)))
    {
        if (hwnd != NULL)
            ::ShowWindow(hwnd, SW_MINIMIZE);
    }

    ImGui::SameLine(0, button_spacing);

    // 红色关闭按钮（调暗一点）
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.75f, 0.22f, 0.22f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.32f, 0.32f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.65f, 0.17f, 0.17f, 1.00f));
    if (ImGui::Button("X", ImVec2(button_width, button_width)))
    {
        SaveSettings();
        ::PostQuitMessage(0);
    }
    ImGui::PopStyleColor(3);

    ImGui::PopStyleVar(2);

    ImGui::Button(LANG("settings")) ? ImGui::OpenPopup("SettingsMenu") : (void)0;
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y + 5));
    RenderSettingsPopup();

    ImGui::SameLine();
    if (ImGui::Button(LANG("help"))) { g_appState.bShowHelp = true; ImGui::OpenPopup(LANG("help")); }
    ImGui::Spacing();

    ImGui::BeginChild("MainContent", ImVec2(0, 0), false);
    {
        ImGui::BeginChild("LeftMain", ImVec2(310, 0), true);
        RenderLeftPanel();
        ImGui::EndChild();

        ImGui::SameLine();
        ImGui::BeginChild("RightQuick", ImVec2(0, 0), true);
        RenderRightPanel();
        ImGui::EndChild();
    }
    ImGui::EndChild();
    ImGui::End();
    
    // 恢复样式
    ImGui::PopStyleVar(5);
    ImGui::PopStyleColor(7);

    if (g_appState.bShowAbout) RenderAboutPopup();
    if (g_appState.bShowHelp) RenderHelpPopup();
}

void RenderSettingsPopup()
{
    if (!ImGui::BeginPopup("SettingsMenu")) return;

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.5f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.6f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.4f, 0.1f, 1.0f));
    if (ImGui::Button(LANG("reset_settings"), ImVec2(-1, 0)))
        ImGui::OpenPopup("ConfirmReset");
    ImGui::PopStyleColor(3);

    if (ImGui::BeginPopupModal("ConfirmReset", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text(LANG("confirm_reset_text"));
        ImGui::Spacing();
        if (ImGui::Button(LANG("confirm"), ImVec2(120, 0))) { ResetSettings(); ImGui::CloseCurrentPopup(); ImGui::CloseCurrentPopup(); }
        ImGui::SameLine();
        if (ImGui::Button(LANG("cancel"), ImVec2(120, 0))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    ImGui::Separator();
    ImGui::Text(LANG("admin_mode"));
    if (ImGui::Checkbox(LANG("run_as_admin"), &g_appState.bAdminMode))
    {
        SaveSettings();
        BOOL isAdmin = FALSE;
        SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
        PSID AdministratorsGroup;
        if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup))
        {
            CheckTokenMembership(NULL, AdministratorsGroup, &isAdmin);
            FreeSid(AdministratorsGroup);
        }
        if (RunAsAdmin()) PostQuitMessage(0);
    }

    ImGui::Separator();
    if (ImGui::BeginMenu(LANG("theme_settings")))
    {
        if (ImGui::RadioButton(LANG("light_theme"), &g_appState.nThemeMode, 0)) g_appState.bThemeChanged = true;
        if (ImGui::RadioButton(LANG("dark_theme"), &g_appState.nThemeMode, 1)) g_appState.bThemeChanged = true;
        if (ImGui::RadioButton(LANG("follow_system"), &g_appState.nThemeMode, 2)) g_appState.bThemeChanged = true;
        ImGui::EndMenu();
    }

    std::string currentLang = LanguageManager::GetInstance().GetCurrentLanguage();
    if (ImGui::BeginMenu(LANG("language")))
    {
        if (ImGui::MenuItem("中文", nullptr, currentLang == "zh-CN"))
        {
            LanguageManager::GetInstance().SetLanguage("zh-CN");
            SaveSettings();
        }
        if (ImGui::MenuItem("English", nullptr, currentLang == "en-US"))
        {
            LanguageManager::GetInstance().SetLanguage("en-US");
            SaveSettings();
        }
        ImGui::EndMenu();
    }

    ImGui::Separator();
    if (ImGui::Button(LANG("about"), ImVec2(-1, 0))) { g_appState.bShowAbout = true; ImGui::CloseCurrentPopup(); ImGui::OpenPopup(LANG("about")); }

    ImGui::EndPopup();
}

void RenderAboutPopup()
{
    if (g_appState.bShowAbout)
    {
        if (g_hViewportWindow)
        {
            RECT rect;
            if (::GetWindowRect(g_hViewportWindow, &rect))
            {
                int winW = rect.right - rect.left;
                int winH = rect.bottom - rect.top;
                int posX = rect.left + (winW - 400) / 2;
                int posY = rect.top + (winH - 400) / 2;
                ImGui::SetNextWindowPos(ImVec2((float)posX, (float)posY), ImGuiCond_Always);
            }
        }
        ImGui::OpenPopup(LANG("about"));
    }
    if (!ImGui::BeginPopup(LANG("about"), ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)) return;

    ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f), LANG("about_title"));
    ImGui::Separator();
    ImGui::TextWrapped(LANG("about_desc"));
    ImGui::Spacing();

    ImGui::TextColored(ImVec4(0.8f, 0.5f, 0.2f, 1.0f), LANG("original_project"));
    ImGui::BulletText(LANG("original_author"));
    ImGui::Text("GitHub: ");
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));
    ImGui::Text("https://github.com/inkuang/WindowResizer");
    if (ImGui::IsItemHovered())
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        if (ImGui::IsItemClicked())
        {
            // 使用 ShellExecuteW 直接打开链接
            ShellExecuteW(NULL, L"open", L"https://github.com/inkuang/WindowResizer", NULL, NULL, SW_SHOWNORMAL);
        }
    }
    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.4f, 0.7f, 0.3f, 1.0f), LANG("imgui_version_info"));
    ImGui::BulletText(LANG("imgui_rewritten"));
    ImGui::BulletText(LANG("imgui_same_features"));
    ImGui::Spacing();
    ImGui::BulletText(LANG("imgui_author"));
    ImGui::Text("GitHub: ");
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));
    ImGui::Text("https://github.com/siuzhiyu/WindowResizer");
    if (ImGui::IsItemHovered())
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        if (ImGui::IsItemClicked())
        {
            // 使用 ShellExecuteW 直接打开链接
            ShellExecuteW(NULL, L"open", L"https://github.com/siuzhiyu/WindowResizer", NULL, NULL, SW_SHOWNORMAL);
        }
    }
    ImGui::PopStyleColor();

    ImGui::Separator();
    if (ImGui::Button(LANG("close"), ImVec2(120, 0))) { g_appState.bShowAbout = false; ImGui::CloseCurrentPopup(); }
    
    // 检查是否点击了弹窗外部
    if (ImGui::IsMouseClicked(0) && !ImGui::IsWindowHovered())
    {
        g_appState.bShowAbout = false;
        ImGui::CloseCurrentPopup();
    }
    
    ImGui::EndPopup();
}

void RenderHelpPopup()
{
    if (g_appState.bShowHelp)
    {
        if (g_hViewportWindow)
        {
            RECT rect;
            if (::GetWindowRect(g_hViewportWindow, &rect))
            {
                int winW = rect.right - rect.left;
                int winH = rect.bottom - rect.top;
                int posX = rect.left + (winW - 400) / 2;
                int posY = rect.top + (winH - 500) / 2;
                ImGui::SetNextWindowPos(ImVec2((float)posX, (float)posY), ImGuiCond_Always);
            }
        }
        ImGui::OpenPopup(LANG("help"));
    }
    if (!ImGui::BeginPopup(LANG("help"), ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)) return;

    ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f), LANG("help_title"));
    ImGui::Separator();
    ImGui::BulletText(LANG("help_step1"));
    ImGui::BulletText(LANG("help_step2"));
    ImGui::BulletText(LANG("help_step3"));
    ImGui::BulletText(LANG("help_step4"));
    ImGui::Spacing();

    ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f), LANG("help_logic_title"));
    ImGui::Separator();
    ImGui::TextWrapped(LANG("help_logic_desc"));
    ImGui::BulletText(LANG("help_logic_1"));
    ImGui::BulletText(LANG("help_logic_2"));
    ImGui::BulletText(LANG("help_logic_3"));
    ImGui::BulletText(LANG("help_logic_4"));
    ImGui::Separator();

    if (ImGui::Button(LANG("close"), ImVec2(120, 0))) { g_appState.bShowHelp = false; ImGui::CloseCurrentPopup(); }
    
    // 检查是否点击了弹窗外部
    if (ImGui::IsMouseClicked(0) && !ImGui::IsWindowHovered())
    {
        g_appState.bShowHelp = false;
        ImGui::CloseCurrentPopup();
    }
    
    ImGui::EndPopup();
}

void RenderLeftPanel()
{
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.65f, 0.45f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.75f, 0.55f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.08f, 0.55f, 0.35f, 1.0f));
    if (ImGui::Button(LANG("start_find_window"), ImVec2(-1, 38)))
        FindWindowCallback();
    ImGui::PopStyleColor(3);

    if (g_appState.bFindingWindow)
    {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.8f, 0.6f, 0.0f, 1.0f), LANG("click_target_window"));
    }

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f), LANG("window_info"));
    ImGui::Separator();
    ImGui::Spacing();

    char szHandle[48];
    sprintf_s(szHandle, 48, "%s: 0x%08llX", LANG("handle"), (unsigned long long)(uintptr_t)g_appState.hTargetWindow);
    ImGui::TextUnformatted(szHandle);

    char szTitle[512];
    sprintf_s(szTitle, 512, "%s: %s", LANG("title"), g_appState.szWindowTitleUTF8);
    ImGui::TextUnformatted(szTitle);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", g_appState.szWindowTitleUTF8);

    char szSize[128];
    sprintf_s(szSize, 128, "%s: %-6d             %s: %d", LANG("width"), g_appState.nCurWidth, LANG("height"), g_appState.nCurHeight);
    ImGui::TextUnformatted(szSize);

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f), LANG("resize"));
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::BeginGroup();
    ImGui::RadioButton(LANG("resolution"), &g_appState.nModeSelect, 0);
    ImGui::SameLine();
    ImGui::SetCursorPosX(120);
    ImGui::PushItemWidth(180);
    ImGui::BeginDisabled(g_appState.nModeSelect != 0);
    const char* combo_preview = g_appState.nSelectedResolution < g_arrResolutionsCount ? g_arrResolutions[g_appState.nSelectedResolution].name : "";
    if (ImGui::BeginCombo("##ResolutionCombo", combo_preview))
    {
        for (int i = 0; i < g_arrResolutionsCount; i++)
        {
            bool is_selected = g_appState.nSelectedResolution == i;
            if (ImGui::Selectable(g_arrResolutions[i].name, is_selected))
                g_appState.nSelectedResolution = i;
            if (is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();
    ImGui::EndDisabled();
    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::RadioButton(LANG("custom_size"), &g_appState.nModeSelect, 1);
    ImGui::BeginDisabled(g_appState.nModeSelect != 1);

    ImGui::Text("               %s", LANG("custom_width_label"));
    ImGui::SameLine();
    ImGui::SetCursorPosX(120);
    ImGui::PushItemWidth(180);
    if (ImGui::InputInt("##WidthInput", &g_appState.nWidth))
    {
        if (g_appState.bLockRatio && g_appState.bHasAspectRatio && g_appState.dLockedAspectRatio > 0.0)
            g_appState.nHeight = (int)(g_appState.nWidth / g_appState.dLockedAspectRatio);
    }
    ImGui::PopItemWidth();
    ImGui::Spacing();

    ImGui::Text("               %s", LANG("custom_height_label"));
    ImGui::SameLine();
    ImGui::SetCursorPosX(120);
    ImGui::PushItemWidth(180);
    if (ImGui::InputInt("##HeightInput", &g_appState.nHeight))
    {
        if (g_appState.bLockRatio && g_appState.bHasAspectRatio && g_appState.dLockedAspectRatio > 0.0)
            g_appState.nWidth = (int)(g_appState.nHeight * g_appState.dLockedAspectRatio);
    }
    ImGui::PopItemWidth();
    ImGui::Spacing();

    ImGui::SameLine();
    ImGui::SetCursorPosX(120);
    if (ImGui::Checkbox(LANG("lock_ratio"), &g_appState.bLockRatio))
    {
        if (g_appState.bLockRatio)
        {
            if (g_appState.nWidth > 0 && g_appState.nHeight > 0)
            {
                g_appState.dLockedAspectRatio = (double)g_appState.nWidth / (double)g_appState.nHeight;
                g_appState.bHasAspectRatio = true;
            }
        }
        else
        {
            g_appState.dLockedAspectRatio = 0.0;
            g_appState.bHasAspectRatio = false;
        }
    }
    ImGui::EndDisabled();

    ImGui::Spacing();
    ImGui::BeginGroup();
    ImGui::RadioButton(LANG("scale"), &g_appState.nModeSelect, 2);
    ImGui::SameLine();
    ImGui::SetCursorPosX(120);
    ImGui::PushItemWidth(180);
    ImGui::BeginDisabled(g_appState.nModeSelect != 2);
    if (ImGui::BeginCombo("##ScaleCombo", g_arrScaleOptions[g_appState.nScaleIndex]))
    {
        for (int i = 0; i < g_arrScaleCount; i++)
        {
            bool is_selected = g_appState.nScaleIndex == i;
            if (ImGui::Selectable(g_arrScaleOptions[i], is_selected))
                g_appState.nScaleIndex = i;
            if (is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();
    ImGui::EndDisabled();
    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::BeginGroup();
    ImGui::RadioButton(LANG("input_scale"), &g_appState.nModeSelect, 3);
    ImGui::SameLine();
    ImGui::SetCursorPosX(120);
    ImGui::PushItemWidth(180);
    ImGui::BeginDisabled(g_appState.nModeSelect != 3);
    float percentage = g_appState.fCustomScale * 100.0f;
    if (ImGui::InputFloat("##PercentInput", &percentage, 5.0f, 10.0f, "%.1f %%"))
        g_appState.fCustomScale = percentage / 100.0f;
    ImGui::PopItemWidth();
    ImGui::EndDisabled();
    ImGui::EndGroup();

    ImGui::Dummy(ImVec2(0, 20));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.22f, 0.58f, 0.9f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.32f, 0.68f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.18f, 0.48f, 0.8f, 1.0f));
    if (ImGui::Button(LANG("apply_settings"), ImVec2(-1, 42)))
        ApplySelectedMode();
    ImGui::PopStyleColor(3);
}

void RenderRightPanel()
{
    ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f), LANG("quick_actions"));
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.65f, 0.45f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.75f, 0.55f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.08f, 0.55f, 0.35f, 1.0f));
    if (ImGui::Button(LANG("restore_initial"), ImVec2(-1, 38)))
        RestoreWindow();
    ImGui::PopStyleColor(3);
    ImGui::Spacing();

    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button(LANG("force_maximize"), ImVec2(-1, 38)))
        MaximizeWindow();
    ImGui::Spacing();

    if (ImGui::Button(LANG("center_screen"), ImVec2(-1, 38)))
        CenterWindow();
    ImGui::Spacing();

    if (g_appState.bTitleBarHidden)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.6f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.7f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.5f, 0.1f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        if (ImGui::Button(LANG("show_titlebar"), ImVec2(-1, 38)))
            ToggleTitleBar();
        ImGui::PopStyleColor(4);
    }
    else
    {
        if (ImGui::Button(LANG("hide_titlebar"), ImVec2(-1, 38)))
            ToggleTitleBar();
    }
    ImGui::Spacing();
}
