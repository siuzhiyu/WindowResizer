#pragma once

#include <windows.h>
#include <string>

#define SOFTWARE_NAME "WindowResizer-imgui"
#define CURRENT_VERSION 120
#define APP_DATA_DIR SOFTWARE_NAME

struct AppState
{
    HWND hTargetWindow = nullptr;
    wchar_t szWindowTitle[256] = L"";
    char szWindowTitleUTF8[512] = "";
    int nCurWidth = 0, nCurHeight = 0;
    int nInitialWidth = 0, nInitialHeight = 0;
    int nCurrentWidth = 0, nCurrentHeight = 0;
    int nScaleBaseWidth = 0, nScaleBaseHeight = 0;
    int nModeSelect = 0;
    int nSelectedResolution = 6;
    int nScaleIndex = 2;
    float fCustomScale = 1.0f;
    int nWidth = 800, nHeight = 600;
    bool bLockRatio = false;
    double dLockedAspectRatio = 0.0;
    bool bHasAspectRatio = false;
    bool bClickedSetPercentage = false;
    bool bIsSetMaxSize = false;
    bool bFindingWindow = false;
    bool bShowAbout = false;
    bool bShowHelp = false;
    bool bTitleBarHidden = false;
    bool bAdminMode = false;
    LONG originalWindowStyle = 0;
    int nThemeMode = 2;
    bool bIsDarkTheme = true;
    bool bThemeChanged = false;
    int nWindowX = -1;
    int nWindowY = -1;
    const char* strLanguage = "";
};

extern AppState g_appState;
extern HWND g_hMainWindow;
extern HWND g_hViewportWindow;

void SaveSettings();
void LoadSettings();
void ResetSettings();
bool RunAsAdmin();
void DetectAndSetLanguage();

extern const char* g_arrScaleOptions[];
extern const float g_arrScaleValues[];
extern const int g_arrScaleCount;

struct ResolutionPreset
{
    const char* name;
    int width;
    int height;
    const char* aspectRatio;
};

extern const ResolutionPreset g_arrResolutions[];
extern const int g_arrResolutionsCount;

void UpdateSizeShow(int width, int height);
bool IsWindowValid();
void FindWindowCallback();
void CheckWindowValidity();
void ApplySelectedMode();
void CenterWindow();
void MaximizeWindow();
void RestoreWindow();
void ToggleTitleBar();
void RenderUI();
