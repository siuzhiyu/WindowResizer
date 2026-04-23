#include "LanguageManager.h"

LanguageManager& LanguageManager::GetInstance()
{
    static LanguageManager instance;
    return instance;
}

LanguageManager::LanguageManager() : m_currentLangCode("en-US")
{
    LoadStrings();
}

void LanguageManager::SetLanguage(const std::string& langCode)
{
    m_currentLangCode = langCode;
    LoadStrings();
}

std::string LanguageManager::GetCurrentLanguage() const
{
    return m_currentLangCode;
}

const char* LanguageManager::Get(const std::string& key) const
{
    auto it = m_strings.find(key);
    if (it != m_strings.end()) return it->second.c_str();
    return "";
}

void LanguageManager::LoadStrings()
{
    if (m_currentLangCode == "zh-CN")
        LoadChinese();
    else
        LoadEnglish();
}

void LanguageManager::LoadChinese()
{
    m_strings.clear();
    m_strings["settings"] = "设置";
    m_strings["admin_mode"] = "管理员模式";
    m_strings["run_as_admin"] = "以管理员权限运行";
    m_strings["theme_settings"] = "主题设置";
    m_strings["light_theme"] = "浅色主题";
    m_strings["dark_theme"] = "暗色主题";
    m_strings["follow_system"] = "随系统切换";
    m_strings["about"] = "关于";
    m_strings["reset_settings"] = "重置设置";
    m_strings["confirm_reset"] = "确认重置";
    m_strings["confirm_reset_text"] = "确定要将所有设置恢复为默认值吗？";
    m_strings["confirm"] = "确定";
    m_strings["cancel"] = "取消";
    m_strings["help"] = "使用说明";
    m_strings["start_find_window"] = "开始查找窗口";
    m_strings["click_target_window"] = "请点击目标窗口...";
    m_strings["window_info"] = "窗口信息";
    m_strings["handle"] = "句柄";
    m_strings["title"] = "标题";
    m_strings["width"] = "宽度";
    m_strings["height"] = "高度";
    m_strings["resize"] = "调整大小";
    m_strings["resolution"] = "常用分辨率";
    m_strings["custom_size"] = "自定义宽高";
    m_strings["custom_width_label"] = "宽:";
    m_strings["custom_height_label"] = "高:";
    m_strings["lock_ratio"] = "锁定比例";
    m_strings["scale"] = "比例缩放";
    m_strings["input_scale"] = "输入比例:";
    m_strings["apply_settings"] = "应用设置";
    m_strings["quick_actions"] = "快捷功能";
    m_strings["restore_initial"] = "还原初始状态";
    m_strings["force_maximize"] = "强制最大化";
    m_strings["center_screen"] = "屏幕居中";
    m_strings["hide_titlebar"] = "隐藏标题栏";
    m_strings["show_titlebar"] = "显示标题栏";
    m_strings["about_title"] = "Window Resizer (ImGui 版本)";
    m_strings["about_desc"] = "快速调整任何窗口大小的小工具。";
    m_strings["original_project"] = "原始项目信息:";
    m_strings["original_author"] = "原作者: inkuang";
    m_strings["imgui_version_info"] = "ImGui版本信息:";
    m_strings["imgui_rewritten"] = "基于ImGui重写界面";
    m_strings["imgui_same_features"] = "功能大致与原版一致";
    m_strings["imgui_author"] = "作者: siuzhiyu";
    m_strings["close"] = "关闭";
    m_strings["help_title"] = "基本操作步骤";
    m_strings["help_step1"] = "1. 点击界面中的\"开始查找窗口\"按钮";
    m_strings["help_step2"] = "2. 点击需要调整的目标窗口";
    m_strings["help_step3"] = "3. 在界面中选择合适的调整模式";
    m_strings["help_step4"] = "4. 点击\"应用设置\"按钮完成窗口调整";
    m_strings["help_logic_title"] = "调整大小逻辑说明";
    m_strings["help_logic_desc"] = "本软件采用双重分辨率变量机制:";
    m_strings["help_logic_1"] = "初始分辨率: 首次查找窗口时记录，永久保存";
    m_strings["help_logic_2"] = "当前分辨率: 基于初始分辨率，随调整操作实时更新";
    m_strings["help_logic_3"] = "比例缩放: 以当前分辨率为基准进行比例计算";
    m_strings["help_logic_4"] = "还原操作: 始终恢复到初始分辨率";
    m_strings["language"] = "语言";
}

void LanguageManager::LoadEnglish()
{
    m_strings.clear();
    m_strings["settings"] = "Settings";
    m_strings["admin_mode"] = "Admin Mode";
    m_strings["run_as_admin"] = "Run as Administrator";
    m_strings["theme_settings"] = "Theme Settings";
    m_strings["light_theme"] = "Light Theme";
    m_strings["dark_theme"] = "Dark Theme";
    m_strings["follow_system"] = "Follow System";
    m_strings["about"] = "About";
    m_strings["reset_settings"] = "Reset Settings";
    m_strings["confirm_reset"] = "Confirm Reset";
    m_strings["confirm_reset_text"] = "Are you sure you want to reset all settings to default?";
    m_strings["confirm"] = "Confirm";
    m_strings["cancel"] = "Cancel";
    m_strings["help"] = "Help";
    m_strings["start_find_window"] = "Start Find Window";
    m_strings["click_target_window"] = "Please click the target window...";
    m_strings["window_info"] = "Window Info";
    m_strings["handle"] = "Handle";
    m_strings["title"] = "Title";
    m_strings["width"] = "Width";
    m_strings["height"] = "Height";
    m_strings["resize"] = "Resize";
    m_strings["resolution"] = "Resolution";
    m_strings["custom_size"] = "Custom Size";
    m_strings["custom_width_label"] = "W:";
    m_strings["custom_height_label"] = "H:";
    m_strings["lock_ratio"] = "Lock Ratio";
    m_strings["scale"] = "Scale";
    m_strings["input_scale"] = "Input Scale:";
    m_strings["apply_settings"] = "Apply Settings";
    m_strings["quick_actions"] = "Quick Actions";
    m_strings["restore_initial"] = "Restore Initial";
    m_strings["force_maximize"] = "Force Maximize";
    m_strings["center_screen"] = "Center on Screen";
    m_strings["hide_titlebar"] = "Hide Title Bar";
    m_strings["show_titlebar"] = "Show Title Bar";
    m_strings["about_title"] = "Window Resizer (ImGui Version)";
    m_strings["about_desc"] = "A small tool to quickly resize any window.";
    m_strings["original_project"] = "Original Project Info:";
    m_strings["original_author"] = "Original Author: inkuang";
    m_strings["imgui_version_info"] = "ImGui Version Info:";
    m_strings["imgui_rewritten"] = "UI rewritten with ImGui";
    m_strings["imgui_same_features"] = "Features similar to original";
    m_strings["imgui_author"] = "Author: siuzhiyu";
    m_strings["close"] = "Close";
    m_strings["help_title"] = "Basic Steps";
    m_strings["help_step1"] = "1. Click \"Start Find Window\" button";
    m_strings["help_step2"] = "2. Click the target window you want to resize";
    m_strings["help_step3"] = "3. Select a suitable resize mode";
    m_strings["help_step4"] = "4. Click \"Apply Settings\" to resize the window";
    m_strings["help_logic_title"] = "Resize Logic Explanation";
    m_strings["help_logic_desc"] = "This software uses a dual resolution variable mechanism:";
    m_strings["help_logic_1"] = "Initial Resolution: Recorded when first finding window, saved permanently";
    m_strings["help_logic_2"] = "Current Resolution: Based on initial resolution, updated in real-time";
    m_strings["help_logic_3"] = "Scale Resize: Calculated based on current resolution";
    m_strings["help_logic_4"] = "Restore: Always restores to initial resolution";
    m_strings["language"] = "Language";
    m_strings["ui_scale"] = "UI Scale";
    m_strings["ui_scale_desc"] = "Adjust UI element size";
}
