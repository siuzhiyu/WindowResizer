#pragma once

struct ResolutionPreset;

// 静态分辨率预设数组
extern const ResolutionPreset g_arrResolutions[];
extern const int g_arrResolutionsCount;

// 静态缩放选项数组
extern const char* g_arrScaleOptions[];
extern const float g_arrScaleValues[];
extern const int g_arrScaleCount;

// 调试用全局配置
namespace DebugConfig {
    const float FontBaseSize = 8.0f;
    const float DpiScaleMultiplier = 1.4f;
    const float UIScaleDebug = 1.0f;
}
