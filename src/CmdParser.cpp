#include "CmdParser.h"
#include "App.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <windows.h>

static bool IsExePath(const std::wstring& path)
{
    if (path.empty()) return false;
    size_t dotPos = path.find_last_of(L'.');
    if (dotPos == std::wstring::npos) return false;
    std::wstring ext = path.substr(dotPos);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
    return ext == L".exe";
}

CommandLineOptions CmdParser::Parse(int argc, wchar_t* argv[])
{
    CommandLineOptions options;
    options.isValid = false;

    if (argc < 2)
    {
        return options;
    }

    for (int i = 1; i < argc; ++i)
    {
        std::wstring arg = argv[i];

        if (arg == L"-h" || arg == L"--help")
        {
            options.help = true;
            options.isValid = true;
            return options;
        }

        if (arg == L"-center" || arg == L"-c")
        {
            options.centerWindow = true;
            continue;
        }

        if (arg == L"-hidetitle" || arg == L"-notitle" || arg == L"-ht")
        {
            options.hideTitleBar = true;
            continue;
        }

        if (arg == L"-showtitle" || arg == L"-st")
        {
            options.showTitleBar = true;
            continue;
        }

        if (arg == L"-maximize" || arg == L"-max" || arg == L"-m")
        {
            options.maximize = true;
            continue;
        }

        if (arg == L"-restore")
        {
            options.restore = true;
            continue;
        }

        if (arg == L"-nowait")
        {
            options.waitForWindow = false;
            continue;
        }

        if (arg == L"-wait" || arg == L"-w")
        {
            if (i + 1 < argc)
            {
                try
                {
                    options.waitTimeout = std::stoi(argv[++i]);
                }
                catch (...)
                {
                    options.waitTimeout = 200;
                }
            }
            continue;
        }

        if (arg == L"-p" || arg == L"--process")
        {
            if (i + 1 < argc)
            {
                options.targetProcessName = argv[++i];
            }
            continue;
        }

        if (arg == L"-t" || arg == L"--title")
        {
            if (i + 1 < argc)
            {
                options.targetWindowTitle = argv[++i];
            }
            continue;
        }

        if (arg == L"-size" || arg == L"-res")
        {
            if (i + 1 < argc)
            {
                try
                {
                    options.width = std::stoi(argv[++i]);
                    if (i + 1 < argc && argv[i + 1][0] != L'-')
                    {
                        options.height = std::stoi(argv[++i]);
                    }
                }
                catch (...)
                {
                }
            }
            continue;
        }

        if (arg == L"-scale")
        {
            if (i + 1 < argc)
            {
                try
                {
                    options.scale = std::stof(argv[++i]) / 100.0f;
                }
                catch (...)
                {
                }
            }
            continue;
        }

        if (arg[0] == L'-')
        {
            std::wstring preset = arg.substr(1);
            IsResolutionPreset(preset, options.width, options.height);
        }
        else
        {
            if (options.targetExePath.empty() && IsExePath(arg))
            {
                options.targetExePath = arg;
            }
            else if (options.targetWindowTitle.empty())
            {
                options.targetWindowTitle = arg;
            }
            else if (options.width == 0 && options.height == 0)
            {
                ParseResolution(arg, options.width, options.height);
            }
        }
    }

    if (!options.targetExePath.empty() || !options.targetWindowTitle.empty() || !options.targetProcessName.empty())
    {
        options.isValid = true;
    }

    return options;
}

bool CmdParser::ParseResolution(const std::wstring& arg, int& width, int& height)
{
    size_t xPos = arg.find(L'x');
    if (xPos != std::wstring::npos)
    {
        try
        {
            width = std::stoi(arg.substr(0, xPos));
            height = std::stoi(arg.substr(xPos + 1));
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
    
    return IsResolutionPreset(arg, width, height);
}

bool CmdParser::IsResolutionPreset(const std::wstring& preset, int& width, int& height)
{
    for (int i = 0; i < g_arrResolutionsCount; ++i)
    {
        const ResolutionPreset& res = g_arrResolutions[i];
        std::wstring name(res.name, res.name + strlen(res.name));
        std::transform(name.begin(), name.end(), name.begin(), ::towlower);
        
        std::wstring presetLower = preset;
        std::transform(presetLower.begin(), presetLower.end(), presetLower.begin(), ::towlower);
        
        if (presetLower == name || preset == std::to_wstring(res.width) + L"x" + std::to_wstring(res.height))
        {
            width = res.width;
            height = res.height;
            return true;
        }
    }
    
    return false;
}

static void WriteToConsole(const wchar_t* text)
{
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdout != INVALID_HANDLE_VALUE)
    {
        DWORD written;
        WriteConsoleW(hStdout, text, (DWORD)wcslen(text), &written, NULL);
    }
}

void CmdParser::PrintUsage()
{
    WriteToConsole(L"WindowResizer-imgui.exe [选项]\n\n");
    WriteToConsole(L"命令行模式用法:\n");
    WriteToConsole(L"  WindowResizer-imgui.exe [目标程序路径] [-分辨率] [-功能选项]\n");
    WriteToConsole(L"  WindowResizer-imgui.exe [窗口标题] [-分辨率] [-功能选项]\n");
    WriteToConsole(L"  WindowResizer-imgui.exe -p [进程名] [-分辨率] [-功能选项]\n\n");
    WriteToConsole(L"参数说明:\n");
    WriteToConsole(L"  [目标程序路径]          直接启动程序并调整其窗口\n");
    WriteToConsole(L"  [窗口标题]              指定要调整的已运行窗口标题（可以是部分标题）\n");
    WriteToConsole(L"  -p, --process [进程名]   指定目标进程名（如 notepad.exe）\n");
    WriteToConsole(L"                         应用场景:批处理脚本、开机自动调整已运行窗口\n");
    WriteToConsole(L"  -t, --title [窗口标题]   指定目标窗口标题（与直接写标题功能相同）\n\n");
    WriteToConsole(L"等待窗口选项:\n");
    WriteToConsole(L"  -wait, -w [毫秒]        等待窗口出现的超时时间（默认3000ms）\n");
    WriteToConsole(L"  -nowait                 无限等待窗口出现（按 Ctrl+C 取消）\n\n");
    WriteToConsole(L"分辨率选项:\n");
    WriteToConsole(L"  -size [宽] [高]         设置窗口尺寸，如 -size 1920 1080\n");
    WriteToConsole(L"  -scale [百分比]         按比例缩放窗口，如 -scale 120（放大到120%）\n\n");
    WriteToConsole(L"功能选项:\n");
    WriteToConsole(L"  -center, -c            将窗口居中显示\n");
    WriteToConsole(L"  -hidetitle, -notitle, -ht  隐藏窗口标题栏\n");
    WriteToConsole(L"  -showtitle, -st        显示窗口标题栏\n");
    WriteToConsole(L"  -maximize, -max, -m    最大化窗口\n");
    WriteToConsole(L"  -h, --help             显示此帮助信息\n\n");
    WriteToConsole(L"示例:\n");
    WriteToConsole(L"  WindowResizer-imgui.exe \"G:\\Game\\WinKawaks.exe\" -size 800 500 -c -ht\n");
    WriteToConsole(L"  WindowResizer-imgui.exe \"记事本\" -size 1920 1080 -st\n");
    WriteToConsole(L"  WindowResizer-imgui.exe -p notepad.exe -scale 150\n");
    WriteToConsole(L"  WindowResizer-imgui.exe \"Google Chrome\" -scale 120 -c\n");
    WriteToConsole(L"\n");
    
}
