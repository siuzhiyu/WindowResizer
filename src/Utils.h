#pragma once
#include <string>

void ConvertToUTF8(const wchar_t* src, char* dst, int dstSize);
std::string GetInstallPath();
std::string GetAppDataPath();
std::string GetSettingsFilePath();
