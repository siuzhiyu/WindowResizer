#include "App.h"
#include "Config.h"
#include "Utils.h"
#include "Settings.h"
#include "Theme.h"
#include "DirectXManager.h"
#include "WindowManager.h"
#include "LanguageManager.h"
#include "FontConfig.h"
#include "CmdParser.h"
#include "CommandExecutor.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <d3d11.h>
#include <dxgi1_2.h>
#include <tchar.h>
#include <cstdio>
#include <windows.h>
#include <shellapi.h>

// 资源定义
#define FONT_RESOURCE 102

AppState g_appState;
HWND g_hMainWindow = nullptr;
HWND g_hViewportWindow = nullptr;

bool RunAsAdmin()
{
    char szPath[MAX_PATH];
    if (GetModuleFileNameA(NULL, szPath, MAX_PATH))
    {
        SHELLEXECUTEINFOA sei = {sizeof(sei), SEE_MASK_NOCLOSEPROCESS, NULL, "runas", szPath, NULL, NULL, SW_SHOWNORMAL};
        return ShellExecuteExA(&sei) != FALSE;
    }
    return false;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    int argc;
    wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    CommandLineOptions cmdOptions = CmdParser::Parse(argc, argv);

    bool hasConsole = false;
    if (cmdOptions.help || cmdOptions.isValid)
    {
        // 尝试附加到父控制台
        if (!AttachConsole(ATTACH_PARENT_PROCESS))
        {
            // 如果没有父控制台，创建新控制台
            AllocConsole();
        }
        hasConsole = true;
    }

    if (cmdOptions.help)
    {
        CmdParser::PrintUsage();
        LocalFree(argv);
        if (hasConsole)
        {
            Sleep(CONSOLE_FLUSH_DELAY_MS); // 短暂等待确保输出完成
            FreeConsole();
        }
        return 0;
    }

    if (cmdOptions.isValid)
    {
        bool success = CommandExecutor::Execute(cmdOptions);
        LocalFree(argv);
        if (hasConsole)
        {
            Sleep(CONSOLE_FLUSH_DELAY_MS); // 短暂等待确保输出完成
            FreeConsole();
        }
        return success ? 0 : 1;
    }

    LocalFree(argv);

    LoadSettings();
    // 优先使用保存的语言，没有保存则检测系统语言
    if (!g_appState.strLanguage || strlen(g_appState.strLanguage) == 0)
    {
        DetectAndSetLanguage();
    }
    else
    {
        LanguageManager::GetInstance().SetLanguage(g_appState.strLanguage);
    }

    BOOL isAdmin = FALSE;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup))
    { CheckTokenMembership(NULL, AdministratorsGroup, &isAdmin); FreeSid(AdministratorsGroup); }

    if (g_appState.bAdminMode && !isAdmin)
    {
        if (RunAsAdmin())
        {
            return 0; // 成功启动管理员进程后退出当前进程
        }
    }

    ImGui_ImplWin32_EnableDpiAwareness();
    float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY));
    // 样式跟随系统 DPI 和 UIScaleDebug 调试值
    float style_scale = main_scale * DebugConfig::UIScaleDebug;
    float font_scale = main_scale * DebugConfig::DpiScaleMultiplier * DebugConfig::UIScaleDebug;

    WNDCLASSEXW wc = {sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Window Resizer", nullptr};
    ::RegisterClassExW(&wc);

    // 创建主窗口 - 使用普通窗口样式，隐藏显示
    g_hMainWindow = ::CreateWindowExW(
        0,
        wc.lpszClassName,
        L"ImGuiHost",
        WS_OVERLAPPEDWINDOW,
        0, 0, 100, 100,
        NULL, NULL, GetModuleHandle(nullptr), NULL
    );
    if (!CreateDeviceD3D(g_hMainWindow)) { CleanupDeviceD3D(); ::UnregisterClassW(wc.lpszClassName, wc.hInstance); return 1; }
    ::ShowWindow(g_hMainWindow, SW_HIDE);
    ::UpdateWindow(g_hMainWindow);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigViewportsNoAutoMerge = true;
    io.ConfigViewportsNoTaskBarIcon = false;
    io.ConfigViewportsNoDefaultParent = true;

    // 优化 ImGui 字体图集配置
    ImFontAtlas* atlas = io.Fonts;
    atlas->Flags |= ImFontAtlasFlags_NoPowerOfTwoHeight;  // 不强制使用2的幂次方高度
    atlas->Flags |= ImFontAtlasFlags_NoMouseCursors;      // 不生成鼠标光标纹理
    
    ImFontConfig font_cfg;
    font_cfg.OversampleH = 0;  // 禁用水平抗锯齿
    font_cfg.OversampleV = 0;  // 禁用垂直抗锯齿
    font_cfg.PixelSnapH = true;  // 像素对齐
    font_cfg.MergeMode = false;  // 不合并字体
    
    // 从资源中加载字体
    bool fontLoaded = false;
    HRSRC hFontResource = FindResourceW(GetModuleHandle(nullptr), MAKEINTRESOURCEW(FONT_RESOURCE), RT_RCDATA);
    if (hFontResource)
    {
        HGLOBAL hFontGlobal = LoadResource(nullptr, hFontResource);
        if (hFontGlobal)
        {
            void* pFontData = LockResource(hFontGlobal);
            if (pFontData)
            {
                DWORD fontSize = SizeofResource(nullptr, hFontResource);
                io.Fonts->AddFontFromMemoryTTF(pFontData, fontSize, DebugConfig::FontBaseSize * font_scale, &font_cfg, custom_glyph_ranges);
                fontLoaded = true;
            }
        }
    }

    // 如果内嵌字体加载失败，尝试使用系统字体"微软雅黑"
    if (!fontLoaded)
    {
        ImFont* systemFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyh.ttc", DebugConfig::FontBaseSize * font_scale, &font_cfg, custom_glyph_ranges);
        if (systemFont)
        {
            fontLoaded = true;
        }
    }

    // 如果还是失败，使用 ImGui 默认字体
    if (!fontLoaded)
    {
        io.Fonts->AddFontDefault(&font_cfg);
    }

    g_appState.bThemeChanged = true;
    UpdateTheme();

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(style_scale);  // 样式按系统 DPI 和 UIScaleDebug 调试值
    style.FontScaleDpi = main_scale * DebugConfig::DpiScaleMultiplier;  // FontScaleDpi 只按系统
    style.WindowRounding = 8.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.FramePadding = ImVec2(8, 4);
    style.ItemSpacing = ImVec2(8, 4);
    style.WindowPadding = ImVec2(12, 12);

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) style.Colors[ImGuiCol_WindowBg].w = 1.0f;

    ImGui_ImplWin32_Init(g_hMainWindow);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
    InitAppData();

    RenderLoop();

    SaveSettings();
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    ::DestroyWindow(g_hMainWindow);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 0;
}
