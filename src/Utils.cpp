#include "Utils.h"
#include "App.h"
#include <windows.h>
#include <cstdio>

void ConvertToUTF8(const wchar_t* src, char* dst, int dstSize)
{
    if (src == nullptr || dst == nullptr || dstSize <= 0) { if (dst && dstSize > 0) dst[0] = '\0'; return; }
    WideCharToMultiByte(CP_UTF8, 0, src, -1, dst, dstSize, nullptr, nullptr);
}

std::string GetInstallPath()
{
    char szPath[MAX_PATH];
    if (GetModuleFileNameA(NULL, szPath, MAX_PATH)) { std::string path = szPath; size_t pos = path.find_last_of('\\'); if (pos != std::string::npos) return path.substr(0, pos); }
    return "";
}

std::string GetAppDataPath()
{
    char szPath[MAX_PATH];
    if (GetEnvironmentVariableA("APPDATA", szPath, MAX_PATH)) { std::string path = std::string(szPath) + "\\" + APP_DATA_DIR; CreateDirectoryA(path.c_str(), NULL); return path; }
    return "";
}

std::string GetSettingsFilePath() { std::string appDataPath = GetAppDataPath(); return appDataPath.empty() ? "settings.ini" : appDataPath + "\\settings.ini"; }
