#include "Settings.h"
#include "App.h"
#include "Utils.h"
#include "LanguageManager.h"
#include <windows.h>
#include <cstdio>

namespace
{
    static void SetLanguageToAppState(const char* lang)
    {
        static char langBuffer[32];
        strncpy_s(langBuffer, sizeof(langBuffer), lang, _TRUNCATE);
        g_appState.strLanguage = langBuffer;
    }
}

void MigrateSettings()
{
    std::string oldPath = GetInstallPath() + "\\settings.ini";
    if (GetFileAttributesA(oldPath.c_str()) != INVALID_FILE_ATTRIBUTES)
    {
        FILE* file = nullptr;
    if (fopen_s(&file, oldPath.c_str(), "r") == 0 && file)
        {
            char line[256];
            while (fgets(line, sizeof(line), file))
            {
                if (strstr(line, "ThemeMode=")) g_appState.nThemeMode = atoi(strchr(line, '=') + 1);
                else if (strstr(line, "AdminMode=")) g_appState.bAdminMode = atoi(strchr(line, '=') + 1);
                else if (strstr(line, "Language="))
                {
                    char* lang = strchr(line, '=') + 1;
                    if (lang)
                    {
                        size_t len = strlen(lang);
                        if (len > 0 && lang[len-1] == '\n') lang[len-1] = '\0';
                        if (len > 1 && lang[len-2] == '\r') lang[len-2] = '\0';
                        SetLanguageToAppState(lang);
                    }
                }
            }
            fclose(file);
        }
    }
}

void DetectAndSetLanguage()
{
    LANGID langId = GetUserDefaultUILanguage();
    WORD primaryLangId = PRIMARYLANGID(langId);
    LanguageManager::GetInstance().SetLanguage((primaryLangId == LANG_CHINESE) ? "zh-CN" : "en-US");
}

void SaveSettings()
{
    if (g_hViewportWindow)
    {
        RECT rect;
        if (GetWindowRect(g_hViewportWindow, &rect))
        {
            g_appState.nWindowX = rect.left;
            g_appState.nWindowY = rect.top;
        }
    }
    std::string currentLangStr = LanguageManager::GetInstance().GetCurrentLanguage();
    SetLanguageToAppState(currentLangStr.c_str());
    std::string path = GetSettingsFilePath();
    FILE* file = nullptr;
    if (fopen_s(&file, path.c_str(), "w") == 0 && file) { fprintf(file, "[Settings]\nVersion=%d\nThemeMode=%d\nAdminMode=%d\nInstallPath=%s\nWindowX=%d\nWindowY=%d\nLanguage=%s\n", CURRENT_VERSION, g_appState.nThemeMode, g_appState.bAdminMode, GetInstallPath().c_str(), g_appState.nWindowX, g_appState.nWindowY, g_appState.strLanguage); fclose(file); }
}

void LoadSettings()
{
    g_appState.nThemeMode = 2;
    g_appState.bAdminMode = false;
    g_appState.nWindowX = -1;
    g_appState.nWindowY = -1;
    g_appState.strLanguage = "";
    MigrateSettings();
    std::string path = GetSettingsFilePath();
    FILE* file = nullptr;
    if (fopen_s(&file, path.c_str(), "r") == 0 && file)
    {
        char line[256];
        while (fgets(line, sizeof(line), file))
        {
            if (strstr(line, "ThemeMode=")) g_appState.nThemeMode = atoi(strchr(line, '=') + 1);
            else if (strstr(line, "AdminMode=")) g_appState.bAdminMode = atoi(strchr(line, '=') + 1);
            else if (strstr(line, "WindowX=")) g_appState.nWindowX = atoi(strchr(line, '=') + 1);
            else if (strstr(line, "WindowY=")) g_appState.nWindowY = atoi(strchr(line, '=') + 1);
            else if (strstr(line, "Language="))
            {
                char* lang = strchr(line, '=') + 1;
                if (lang)
                {
                    size_t len = strlen(lang);
                    if (len > 0 && lang[len-1] == '\n') lang[len-1] = '\0';
                    if (len > 1 && lang[len-2] == '\r') lang[len-2] = '\0';
                    SetLanguageToAppState(lang);
                }
            }
        }
        fclose(file);
    }
}

void ResetSettings()
{
    // 重置窗口相关状态
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
    g_appState.nModeSelect = 0;
    g_appState.nSelectedResolution = 6;
    g_appState.nScaleIndex = 2;
    g_appState.fCustomScale = 1.0f;
    g_appState.nWidth = 800;
    g_appState.nHeight = 600;
    g_appState.bLockRatio = false;
    g_appState.dLockedAspectRatio = 0.0;
    g_appState.bHasAspectRatio = false;
    g_appState.bClickedSetPercentage = false;
    g_appState.bIsSetMaxSize = false;
    g_appState.bFindingWindow = false;
    g_appState.bShowAbout = false;
    g_appState.bShowHelp = false;
    g_appState.bTitleBarHidden = false;
    g_appState.originalWindowStyle = 0;
    
    // 重置其他设置
    g_appState.nThemeMode = 2;
    g_appState.bAdminMode = false;
    g_appState.strLanguage = "";
    
    std::string settingsPath = GetSettingsFilePath();
    if (GetFileAttributesA(settingsPath.c_str()) != INVALID_FILE_ATTRIBUTES) DeleteFileA(settingsPath.c_str());
    SaveSettings();
    g_appState.bThemeChanged = true;

    BOOL isAdmin = FALSE;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup))
    { CheckTokenMembership(NULL, AdministratorsGroup, &isAdmin); FreeSid(AdministratorsGroup); }
    if (isAdmin && !g_appState.bAdminMode)
    {
        char szPath[MAX_PATH];
        if (GetModuleFileNameA(NULL, szPath, MAX_PATH))
        {
            SHELLEXECUTEINFOA sei = {sizeof(sei), SEE_MASK_NOCLOSEPROCESS, NULL, "open", szPath, NULL, NULL, SW_SHOWNORMAL};
            ShellExecuteExA(&sei);
            PostQuitMessage(0);
        }
    }
}
