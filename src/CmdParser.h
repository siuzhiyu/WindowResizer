#pragma once

#include <windows.h>
#include <string>
#include <vector>

// 语言枚举
enum class Language
{
    English,
    Chinese
};

// 全局语言设置
extern Language g_cmdLang;

struct CommandLineOptions
{
    std::wstring targetExePath;
    std::wstring targetWindowTitle;
    std::wstring targetProcessName;
    int width = 0;
    int height = 0;
    float scale = 0.0f;
    bool centerWindow = false;
    bool hideTitleBar = false;
    bool showTitleBar = false;
    bool maximize = false;
    bool restore = false;
    bool waitForWindow = true;
    int waitTimeout = 3000;
    bool help = false;
    bool isValid = false;
};

class CmdParser
{
public:
    static CommandLineOptions Parse(int argc, wchar_t* argv[]);
    static void PrintUsage();
    
private:
    static bool ParseResolution(const std::wstring& arg, int& width, int& height);
    static void DetectLanguage();
};
