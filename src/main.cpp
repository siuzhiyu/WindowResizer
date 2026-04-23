// Window Resizer - ImGui + Win32 DirectX9 版本
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include <d3d9.h>
#include <tchar.h>
#include <string>
#include <vector>
#include <cstdio>
#include <cstdint>
#include <windows.h>

// DirectX9 全局变量
static LPDIRECT3D9              g_pD3D = nullptr;
static LPDIRECT3DDEVICE9        g_pd3dDevice = nullptr;
static bool                     g_DeviceLost = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

// 窗口数据结构
struct ResolutionPreset
{
    std::string name;
    int width;
    int height;
    std::string ratio;
};

// 应用程序全局状态
HWND g_hTargetWindow = nullptr;
wchar_t g_szWindowTitle[256] = L"";
char g_szWindowTitleUTF8[512] = "";
int g_nCurWidth = 0, g_nCurHeight = 0;
int g_nInitialWidth = 0, g_nInitialHeight = 0;
int g_nCurrentWidth = 0, g_nCurrentHeight = 0;
int g_nScaleBaseWidth = 0, g_nScaleBaseHeight = 0;
int g_nModeSelect = 0;
int g_nSelectedResolution = 6;
int g_nScaleIndex = 2;
float g_fCustomScale = 1.0f;
int g_nWidth = 800, g_nHeight = 600;
bool g_bLockRatio = false;
double g_dLockedAspectRatio = 0.0;
bool g_bHasAspectRatio = false;
bool g_bClickedSetPercentage = false;
bool g_bIsSetMaxSize = false;
bool g_bFindingWindow = false;
bool g_bShowAbout = false;
bool g_bShowHelp = false;

// 字体指针
ImFont* g_pFontDefault = nullptr;
ImFont* g_pFontSmall = nullptr;
ImFont* g_pFontLarge = nullptr;

std::vector<ResolutionPreset> g_arrResolutions;
std::vector<std::string> g_arrScaleOptions = { "0.5x", "0.75x", "1x", "1.25x", "1.5x", "1.75x", "2x", "2.5x", "3x" };
std::vector<float> g_arrScaleValues = { 0.5f, 0.75f, 1.0f, 1.25f, 1.5f, 1.75f, 2.0f, 2.5f, 3.0f };

// 函数声明
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void InitAppData();
void UpdateSizeShow(int width, int height);
bool IsWindowValid();
void ApplySelectedMode();
void CenterWindow();
void MaximizeWindow();
void RestoreWindow();
void FindWindowCallback();
void CheckWindowValidity();
void ConvertToUTF8(const wchar_t* src, char* dst, int dstSize);

void RenderUI()
{
    ImGuiIO& io = ImGui::GetIO();
    
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y));
    
    if (ImGui::Begin("Window Resizer", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse))
    {
        // 顶部按钮区域
        ImGui::PushFont(g_pFontSmall);
        if (ImGui::Button("使用说明", ImVec2(60, 20)))
            g_bShowHelp = true;
        ImGui::SameLine();
        if (ImGui::Button("关于", ImVec2(30, 20)))
            g_bShowAbout = true;
        ImGui::PopFont();
        ImGui::Spacing();

        ImGui::BeginChild("MainContent", ImVec2(0, 0), false);
        {
            // 左侧主面板
            ImGui::BeginChild("LeftMain", ImVec2(310, 0), true);
            {
                // 开始查找窗口按钮
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.65f, 0.45f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.75f, 0.55f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.08f, 0.55f, 0.35f, 1.0f));
                if (ImGui::Button("开始查找窗口", ImVec2(-1, 38)))
                    FindWindowCallback();
                ImGui::PopStyleColor(3);
                
                if (g_bFindingWindow)
                {
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "请点击目标窗口...");
                }

                // 窗口信息区域
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f), "窗口信息");
                ImGui::Separator();
                ImGui::Spacing();
                
                char szHandle[48];
                sprintf_s(szHandle, 48, "句柄: 0x%08llX", (unsigned long long)(uintptr_t)g_hTargetWindow);
                ImGui::TextUnformatted(szHandle);
                ImGui::Text("标题: %s", g_szWindowTitleUTF8);
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("%s", g_szWindowTitleUTF8);
                }
                ImGui::Text("宽度: %-6d             高度: %d", g_nCurWidth, g_nCurHeight);

                // 调整大小区域
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f), "调整大小");
                ImGui::Separator();
                ImGui::Spacing();
                
                // 常用分辨率模块
                ImGui::BeginGroup();
                ImGui::RadioButton("常用分辨率", &g_nModeSelect, 0);
                ImGui::SameLine();
                ImGui::SetCursorPosX(120);
                ImGui::PushItemWidth(180);
                ImGui::BeginDisabled(g_nModeSelect != 0);
                const char* combo_preview = g_arrResolutions.size() > 0 ? g_arrResolutions[g_nSelectedResolution].name.c_str() : "";
                if (ImGui::BeginCombo("##ResolutionCombo", combo_preview))
                {
                    for (int i = 0; i < (int)g_arrResolutions.size(); i++)
                    {
                        bool is_selected = g_nSelectedResolution == i;
                        if (ImGui::Selectable(g_arrResolutions[i].name.c_str(), is_selected))
                            g_nSelectedResolution = i;
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                ImGui::PopItemWidth();
                ImGui::EndDisabled();
                ImGui::EndGroup();

                // 自定义宽高模块
                ImGui::Spacing();
                ImGui::RadioButton("自定义宽高", &g_nModeSelect, 1);
                ImGui::BeginDisabled(g_nModeSelect != 1);
                
                ImGui::Text("                   宽:");
                ImGui::SameLine();
                ImGui::SetCursorPosX(120);
                ImGui::PushItemWidth(180);
                if (ImGui::InputInt("##WidthInput", &g_nWidth))
                {
                    if (g_bLockRatio && g_bHasAspectRatio && g_dLockedAspectRatio > 0.0)
                        g_nHeight = (int)(g_nWidth / g_dLockedAspectRatio);
                }
                ImGui::PopItemWidth();
                ImGui::Spacing();
                
                ImGui::Text("                   高:");
                ImGui::SameLine();
                ImGui::SetCursorPosX(120);
                ImGui::PushItemWidth(180);
                if (ImGui::InputInt("##HeightInput", &g_nHeight))
                {
                    if (g_bLockRatio && g_bHasAspectRatio && g_dLockedAspectRatio > 0.0)
                        g_nWidth = (int)(g_nHeight * g_dLockedAspectRatio);
                }
                ImGui::PopItemWidth();
                ImGui::Spacing();
                
                ImGui::SameLine();
                ImGui::SetCursorPosX(120);
                if (ImGui::Checkbox("锁定比例", &g_bLockRatio))
                {
                    if (g_bLockRatio)
                    {
                        if (g_nWidth > 0 && g_nHeight > 0)
                        {
                            g_dLockedAspectRatio = (double)g_nWidth / (double)g_nHeight;
                            g_bHasAspectRatio = true;
                        }
                    }
                    else
                    {
                        g_dLockedAspectRatio = 0.0;
                        g_bHasAspectRatio = false;
                    }
                }
                ImGui::EndDisabled();

                // 比例缩放模块
                ImGui::Spacing();
                ImGui::BeginGroup();
                ImGui::RadioButton("比例缩放", &g_nModeSelect, 2);
                ImGui::SameLine();
                ImGui::SetCursorPosX(120);
                ImGui::PushItemWidth(180);
                ImGui::BeginDisabled(g_nModeSelect != 2);
                if (ImGui::BeginCombo("##ScaleCombo", g_arrScaleOptions[g_nScaleIndex].c_str()))
                {
                    for (int i = 0; i < (int)g_arrScaleOptions.size(); i++)
                    {
                        bool is_selected = g_nScaleIndex == i;
                        if (ImGui::Selectable(g_arrScaleOptions[i].c_str(), is_selected))
                            g_nScaleIndex = i;
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                ImGui::PopItemWidth();
                ImGui::EndDisabled();
                ImGui::EndGroup();

                // 输入比例模块
                ImGui::Spacing();
                ImGui::BeginGroup();
                ImGui::RadioButton("输入比例:", &g_nModeSelect, 3);
                ImGui::SameLine();
                ImGui::SetCursorPosX(120);
                ImGui::PushItemWidth(180);
                ImGui::BeginDisabled(g_nModeSelect != 3);
                float percentage = g_fCustomScale * 100.0f;
                if (ImGui::InputFloat("##PercentInput", &percentage, 5.0f, 10.0f, "%.1f %%"))
                {
                    g_fCustomScale = percentage / 100.0f;
                }
                ImGui::PopItemWidth();
                ImGui::EndDisabled();
                ImGui::EndGroup();

                // 应用设置按钮
                ImGui::Dummy(ImVec2(0, 20));
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.22f, 0.58f, 0.9f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.32f, 0.68f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.18f, 0.48f, 0.8f, 1.0f));
                if (ImGui::Button("应用设置", ImVec2(-1, 42)))
                    ApplySelectedMode();
                ImGui::PopStyleColor(3);
            }
            ImGui::EndChild();

            // 右侧快捷功能面板
            ImGui::SameLine();
            ImGui::BeginChild("RightQuick", ImVec2(0, 0), true);
            {
                ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f), "快捷功能");
                ImGui::Separator();
                ImGui::Spacing();
                
                if (ImGui::Button("强制最大化", ImVec2(-1, 38)))
                    MaximizeWindow();
                ImGui::Spacing();
                
                if (ImGui::Button("屏幕居中", ImVec2(-1, 38)))
                    CenterWindow();
                ImGui::Spacing();
                
                if (ImGui::Button("还原初始状态", ImVec2(-1, 38)))
                    RestoreWindow();
            }
            ImGui::EndChild();
        }
        ImGui::EndChild();
    }
    ImGui::End();

    // 关于弹窗
    if (g_bShowAbout)
    {
        ImGui::OpenPopup("关于");
        if (ImGui::BeginPopupModal("关于", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
        {
            ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f), "Window Resizer (ImGui 版本)");
            ImGui::Separator();
            ImGui::TextWrapped("快速调整任何窗口大小的小工具。");
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.8f, 0.5f, 0.2f, 1.0f), "原始项目信息:");
            ImGui::BulletText("原作者: inkuang");
            //ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.1f, 0.1f, 0.3f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.1f, 0.1f, 0.5f));
            if (ImGui::Button("GitHub: https://github.com/inkuang/WindowResizer"))
            {
                ShellExecuteA(NULL, "open", "https://github.com/inkuang/WindowResizer", NULL, NULL, SW_SHOWNORMAL);
            }
            ImGui::PopStyleColor(4);
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.4f, 0.7f, 0.3f, 1.0f), "ImGui版本信息:");
            ImGui::BulletText("基于ImGui重写界面");
            ImGui::BulletText("功能大致与原版一致");
            ImGui::Spacing();
            ImGui::BulletText("作者: siuzhiyu");
            //ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.1f, 0.1f, 0.3f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.1f, 0.1f, 0.5f));
            if (ImGui::Button("GitHub: https://github.com/siuzhiyu/WindowResizer"))
            {
                ShellExecuteA(NULL, "open", "https://github.com/siuzhiyu/WindowResizer", NULL, NULL, SW_SHOWNORMAL);
            }
            ImGui::PopStyleColor(4);
            ImGui::Separator();
            if (ImGui::Button("关闭", ImVec2(120, 0)))
            {
                g_bShowAbout = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    // 使用说明弹窗
    if (g_bShowHelp)
    {
        ImGui::OpenPopup("使用说明");
        if (ImGui::BeginPopupModal("使用说明", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
        {
            ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f), "基本操作步骤");
            ImGui::Separator();
            ImGui::BulletText("1. 点击界面中的\"开始查找窗口\"按钮");
            ImGui::BulletText("2. 点击需要调整的目标窗口");
            ImGui::BulletText("3. 在界面中选择合适的调整模式");
            ImGui::BulletText("4. 点击\"应用设置\"按钮完成窗口调整");
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f), "调整大小逻辑说明");
            ImGui::Separator();
            ImGui::TextWrapped("本软件采用双重分辨率变量机制：");
            ImGui::BulletText("初始分辨率：首次查找窗口时记录，永久保存");
            ImGui::BulletText("当前分辨率：基于初始分辨率，随调整操作实时更新");
            ImGui::BulletText("比例缩放：以当前分辨率为基准进行比例计算");
            ImGui::BulletText("还原操作：始终恢复到初始分辨率");
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f), "快捷功能");
            ImGui::Separator();
            ImGui::BulletText("强制最大化：将目标窗口最大化显示");
            ImGui::BulletText("屏幕居中：将目标窗口移至屏幕中央");
            ImGui::BulletText("还原初始状态：恢复窗口至初始分辨率");
            ImGui::Separator();
            if (ImGui::Button("关闭", ImVec2(120, 0)))
            {
                g_bShowHelp = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
}

void InitAppData()
{
    g_arrResolutions.clear();
    ResolutionPreset preset;
    preset.name = "640 × 480 — 4:3"; preset.width = 640; preset.height = 480; preset.ratio = "4:3"; g_arrResolutions.push_back(preset);
    preset.name = "800 × 600 — 4:3"; preset.width = 800; preset.height = 600; preset.ratio = "4:3"; g_arrResolutions.push_back(preset);
    preset.name = "1024 × 768 — 4:3"; preset.width = 1024; preset.height = 768; preset.ratio = "4:3"; g_arrResolutions.push_back(preset);
    preset.name = "1280 × 720 — 16:9"; preset.width = 1280; preset.height = 720; preset.ratio = "16:9"; g_arrResolutions.push_back(preset);
    preset.name = "1600 × 900 — 16:9"; preset.width = 1600; preset.height = 900; preset.ratio = "16:9"; g_arrResolutions.push_back(preset);
    preset.name = "1920 × 1080 — 16:9"; preset.width = 1920; preset.height = 1080; preset.ratio = "16:9"; g_arrResolutions.push_back(preset);
    preset.name = "2048 × 1152 — 16:9"; preset.width = 2048; preset.height = 1152; preset.ratio = "16:9"; g_arrResolutions.push_back(preset);
    preset.name = "2560 × 1440 — 16:9"; preset.width = 2560; preset.height = 1440; preset.ratio = "16:9"; g_arrResolutions.push_back(preset);
    preset.name = "3840 × 2160 — 16:9"; preset.width = 3840; preset.height = 2160; preset.ratio = "16:9"; g_arrResolutions.push_back(preset);
    preset.name = "5120 × 2800 — 16:9"; preset.width = 5120; preset.height = 2800; preset.ratio = "16:9"; g_arrResolutions.push_back(preset);
    preset.name = "7680 × 4320 — 16:9"; preset.width = 7680; preset.height = 4320; preset.ratio = "16:9"; g_arrResolutions.push_back(preset);
    preset.name = "1280 × 800 — 16:10"; preset.width = 1280; preset.height = 800; preset.ratio = "16:10"; g_arrResolutions.push_back(preset);
    preset.name = "1440 × 900 — 16:10"; preset.width = 1440; preset.height = 900; preset.ratio = "16:10"; g_arrResolutions.push_back(preset);
    preset.name = "1920 × 1200 — 16:10"; preset.width = 1920; preset.height = 1200; preset.ratio = "16:10"; g_arrResolutions.push_back(preset);
    preset.name = "2560 × 1600 — 16:10"; preset.width = 2560; preset.height = 1600; preset.ratio = "16:10"; g_arrResolutions.push_back(preset);
    preset.name = "3840 × 2400 — 16:10"; preset.width = 3840; preset.height = 2400; preset.ratio = "16:10"; g_arrResolutions.push_back(preset);
    preset.name = "2048 × 1080 — 17:9"; preset.width = 2048; preset.height = 1080; preset.ratio = "17:9"; g_arrResolutions.push_back(preset);
    preset.name = "4096 × 2160 — 17:9"; preset.width = 4096; preset.height = 2160; preset.ratio = "17:9"; g_arrResolutions.push_back(preset);
    preset.name = "2048 × 858 — 1.85:1"; preset.width = 2048; preset.height = 858; preset.ratio = "1.85:1"; g_arrResolutions.push_back(preset);
    preset.name = "1998 × 1080 — 2.39:1"; preset.width = 1998; preset.height = 1080; preset.ratio = "2.39:1"; g_arrResolutions.push_back(preset);
    preset.name = "2560 × 1080 — 21:9"; preset.width = 2560; preset.height = 1080; preset.ratio = "21:9"; g_arrResolutions.push_back(preset);
    preset.name = "3440 × 1440 — 21:9"; preset.width = 3440; preset.height = 1440; preset.ratio = "21:9"; g_arrResolutions.push_back(preset);
    preset.name = "3840 × 1600 — 21:9"; preset.width = 3840; preset.height = 1600; preset.ratio = "21:9"; g_arrResolutions.push_back(preset);
    preset.name = "5120 × 2160 — 21:9"; preset.width = 5120; preset.height = 2160; preset.ratio = "21:9"; g_arrResolutions.push_back(preset);
    preset.name = "3840 × 1080 — 32:9"; preset.width = 3840; preset.height = 1080; preset.ratio = "32:9"; g_arrResolutions.push_back(preset);
    preset.name = "5120 × 1440 — 32:9"; preset.width = 5120; preset.height = 1440; preset.ratio = "32:9"; g_arrResolutions.push_back(preset);
    preset.name = "7680 × 2160 — 32:9"; preset.width = 7680; preset.height = 2160; preset.ratio = "32:9"; g_arrResolutions.push_back(preset);
}

void UpdateSizeShow(int width, int height)
{
    g_nCurWidth = width;
    g_nCurHeight = height;
    g_nCurrentWidth = width;
    g_nCurrentHeight = height;
}

bool IsWindowValid()
{
    if (g_hTargetWindow == nullptr)
        return false;
    if (!IsWindow(g_hTargetWindow))
        return false;
    return true;
}

void FindWindowCallback()
{
    g_bFindingWindow = true;
}

void ConvertToUTF8(const wchar_t* src, char* dst, int dstSize)
{
    if (src == nullptr || dst == nullptr || dstSize <= 0)
    {
        if (dst != nullptr && dstSize > 0)
            dst[0] = '\0';
        return;
    }
    WideCharToMultiByte(CP_UTF8, 0, src, -1, dst, dstSize, nullptr, nullptr);
}

void CheckWindowValidity()
{
    if (g_bFindingWindow)
    {
        HWND pWnd = GetForegroundWindow();
        if (pWnd != NULL && pWnd != GetActiveWindow())
        {
            g_hTargetWindow = pWnd;
            if (GetWindowTextW(pWnd, g_szWindowTitle, 255))
            {
                ConvertToUTF8(g_szWindowTitle, g_szWindowTitleUTF8, 512);
            }
            RECT rect;
            GetWindowRect(pWnd, &rect);
            int detectedWidth = rect.right - rect.left;
            int detectedHeight = rect.bottom - rect.top;
            g_nInitialWidth = detectedWidth;
            g_nInitialHeight = detectedHeight;
            g_nCurrentWidth = detectedWidth;
            g_nCurrentHeight = detectedHeight;
            g_nScaleBaseWidth = 0;
            g_nScaleBaseHeight = 0;
            g_nWidth = detectedWidth;
            g_nHeight = detectedHeight;
            UpdateSizeShow(detectedWidth, detectedHeight);
            g_bClickedSetPercentage = false;
            g_bIsSetMaxSize = false;
            g_bFindingWindow = false;
        }
    }
    else if (g_hTargetWindow != nullptr && !IsWindow(g_hTargetWindow))
    {
        g_hTargetWindow = nullptr;
        g_szWindowTitle[0] = L'\0';
        g_szWindowTitleUTF8[0] = '\0';
        g_nCurWidth = 0;
        g_nCurHeight = 0;
        g_nInitialWidth = 0;
        g_nInitialHeight = 0;
        g_nCurrentWidth = 0;
        g_nCurrentHeight = 0;
        g_nScaleBaseWidth = 0;
        g_nScaleBaseHeight = 0;
        g_nWidth = 0;
        g_nHeight = 0;
        g_bClickedSetPercentage = false;
        g_bIsSetMaxSize = false;
    }
}

void ApplySelectedMode()
{
    if (!IsWindowValid())
        return;

    int nWidth = 0, nHeight = 0;
    switch (g_nModeSelect)
    {
    case 0:
        if (g_nSelectedResolution >= 0 && g_nSelectedResolution < (int)g_arrResolutions.size())
        {
            nWidth = g_arrResolutions[g_nSelectedResolution].width;
            nHeight = g_arrResolutions[g_nSelectedResolution].height;
            g_nCurrentWidth = nWidth;
            g_nCurrentHeight = nHeight;
        }
        g_nScaleBaseWidth = 0;
        g_nScaleBaseHeight = 0;
        break;
    case 1:
        nWidth = g_nWidth;
        nHeight = g_nHeight;
        if (nWidth <= 0 || nHeight <= 0)
            return;
        g_nCurrentWidth = nWidth;
        g_nCurrentHeight = nHeight;
        g_nScaleBaseWidth = 0;
        g_nScaleBaseHeight = 0;
        break;
    case 2:
        if (g_nScaleBaseWidth == 0 || g_nScaleBaseHeight == 0)
        {
            g_nScaleBaseWidth = g_nCurrentWidth;
            g_nScaleBaseHeight = g_nCurrentHeight;
        }
        nWidth = (int)(g_nScaleBaseWidth * g_arrScaleValues[g_nScaleIndex]);
        nHeight = (int)(g_nScaleBaseHeight * g_arrScaleValues[g_nScaleIndex]);
        g_nCurrentWidth = nWidth;
        g_nCurrentHeight = nHeight;
        break;
    case 3:
        if (g_nScaleBaseWidth == 0 || g_nScaleBaseHeight == 0)
        {
            g_nScaleBaseWidth = g_nCurrentWidth;
            g_nScaleBaseHeight = g_nCurrentHeight;
        }
        nWidth = (int)(g_nScaleBaseWidth * g_fCustomScale);
        nHeight = (int)(g_nScaleBaseHeight * g_fCustomScale);
        g_nCurrentWidth = nWidth;
        g_nCurrentHeight = nHeight;
        break;
    }

    if (nWidth > 0 && nHeight > 0)
    {
        SetWindowPos(g_hTargetWindow, NULL, 0, 0, nWidth, nHeight, SWP_NOZORDER | SWP_NOMOVE);
        UpdateSizeShow(nWidth, nHeight);
    }
}

void CenterWindow()
{
    if (!IsWindowValid())
        return;
    RECT windowRect;
    GetWindowRect(g_hTargetWindow, &windowRect);

    HMONITOR hMonitor = MonitorFromWindow(g_hTargetWindow, MONITOR_DEFAULTTONEAREST);
    MONITORINFO monitorInfo = { sizeof(monitorInfo) };
    GetMonitorInfo(hMonitor, &monitorInfo);

    int nX = monitorInfo.rcWork.left + ((monitorInfo.rcWork.right - monitorInfo.rcWork.left) - (windowRect.right - windowRect.left)) / 2;
    int nY = monitorInfo.rcWork.top + ((monitorInfo.rcWork.bottom - monitorInfo.rcWork.top) - (windowRect.bottom - windowRect.top)) / 2;
    SetWindowPos(g_hTargetWindow, NULL, nX, nY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

void MaximizeWindow()
{
    if (!IsWindowValid())
        return;
    if (IsZoomed(g_hTargetWindow))
        return;
    ShowWindow(g_hTargetWindow, SW_SHOWMAXIMIZED);
    g_bIsSetMaxSize = true;
    g_bClickedSetPercentage = false;
}

void RestoreWindow()
{
    if (!IsWindowValid())
        return;
    if (g_bIsSetMaxSize)
        ShowWindow(g_hTargetWindow, SW_RESTORE);
    SetWindowPos(g_hTargetWindow, NULL, 0, 0, g_nInitialWidth, g_nInitialHeight, SWP_NOZORDER | SWP_NOMOVE);
    g_nCurrentWidth = g_nInitialWidth;
    g_nCurrentHeight = g_nInitialHeight;
    g_nScaleBaseWidth = 0;
    g_nScaleBaseHeight = 0;
    g_nWidth = g_nInitialWidth;
    g_nHeight = g_nInitialHeight;
    g_nModeSelect = 0;
    RECT rect;
    GetWindowRect(g_hTargetWindow, &rect);
    UpdateSizeShow(rect.right - rect.left, rect.bottom - rect.top);
    g_bClickedSetPercentage = false;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    ImGui_ImplWin32_EnableDpiAwareness();
    float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Window Resizer", nullptr };
    ::RegisterClassExW(&wc);

    RECT rect = { 0, 0, 545, 535 };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    int windowWidth = rect.right - rect.left;
    int windowHeight = rect.bottom - rect.top;
    
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int posX = (screenWidth - windowWidth) / 2;
    int posY = (screenHeight - windowHeight) / 2;
    
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Window Resizer v1.0", WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, posX, posY,
        windowWidth, windowHeight, nullptr, nullptr, wc.hInstance, nullptr);

    HICON hIcon = LoadIconW(wc.hInstance, MAKEINTRESOURCEW(101));
    if (hIcon)
    {
        SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        SendMessageW(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    }

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImFontConfig font_cfg;
    font_cfg.OversampleH = 2;
    font_cfg.OversampleV = 1;
    static const ImWchar chinese_ranges[] = { 0x0020, 0xFFFF, 0 };
    
    const char* fontPaths[] = {
        "C:\\Windows\\Fonts\\msyh.ttc",
        "C:\\Windows\\Fonts\\msyhbd.ttc",
        "C:\\Windows\\Fonts\\simhei.ttf",
        "C:\\Windows\\Fonts\\simsun.ttc",
        "C:\\Windows\\Fonts\\micross.ttf",
        nullptr
    };
    
    const char* selectedFont = nullptr;
    for (int i = 0; fontPaths[i] != nullptr; i++)
    {
        if (GetFileAttributesA(fontPaths[i]) != INVALID_FILE_ATTRIBUTES)
        {
            selectedFont = fontPaths[i];
            break;
        }
    }
    
    if (selectedFont)
    {
        g_pFontDefault = io.Fonts->AddFontFromFileTTF(selectedFont, 16.0f * main_scale, &font_cfg, chinese_ranges);
        g_pFontSmall = io.Fonts->AddFontFromFileTTF(selectedFont, 12.0f * main_scale, &font_cfg, chinese_ranges);
        g_pFontLarge = io.Fonts->AddFontFromFileTTF(selectedFont, 20.0f * main_scale, &font_cfg, chinese_ranges);
    }
    else
    {
        g_pFontDefault = io.Fonts->AddFontDefault(&font_cfg);
        g_pFontSmall = io.Fonts->AddFontDefault(&font_cfg);
        g_pFontLarge = io.Fonts->AddFontDefault(&font_cfg);
    }

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;
    style.WindowRounding = 8.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.FramePadding = ImVec2(8, 4);
    style.ItemSpacing = ImVec2(8, 4);
    style.WindowPadding = ImVec2(12, 12);

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    InitAppData();

    bool done = false;
    ImVec4 clear_color = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);

    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        if (g_DeviceLost)
        {
            HRESULT hr = g_pd3dDevice->TestCooperativeLevel();
            if (hr == D3DERR_DEVICELOST)
            {
                ::Sleep(10);
                continue;
            }
            if (hr == D3DERR_DEVICENOTRESET)
                ResetDevice();
            g_DeviceLost = false;
        }

        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            g_d3dpp.BackBufferWidth = g_ResizeWidth;
            g_d3dpp.BackBufferHeight = g_ResizeHeight;
            g_ResizeWidth = g_ResizeHeight = 0;
            ResetDevice();
        }

        CheckWindowValidity();

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        RenderUI();

        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x*clear_color.w*255.0f), (int)(clear_color.y*clear_color.w*255.0f), (int)(clear_color.z*clear_color.w*255.0f), (int)(clear_color.w*255.0f));
        g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        HRESULT result = g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);
        if (result == D3DERR_DEVICELOST)
            g_DeviceLost = true;
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

bool CreateDeviceD3D(HWND hWnd)
{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
        return false;

    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;

    return true;
}

void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = nullptr; }
}

void ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam);
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
