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

const int CONSOLE_FLUSH_DELAY_MS = 50;

struct CommandLineOptions
{
    std::wstring targetExePath;
    std::wstring targetExeArgs;
    std::wstring targetProcessName;
    int width = 0;
    int height = 0;
    float scale = 0.0f;
    bool centerWindow = false;
    bool hideTitleBar = false;
    bool showTitleBar = false;
    bool maximize = false;
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
