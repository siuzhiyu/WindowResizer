# WindowResizer-ImGui

A tool to quickly resize any window with a beautiful ImGui-based interface.

窗口调整工具，基于 ImGui 构建的精美界面。

---

## Features / 功能特性

- Quick window resizing with common resolution presets
- 常用分辨率预设，快速调整窗口大小
- Custom width/height input with aspect ratio lock
- 自定义宽高输入，支持锁定比例
- Scale windows by percentage
- 按比例缩放窗口
- Theme switching system
- 主题切换系统
- Configuration management with auto-save
- 配置管理，自动保存设置
- Uninstall tool for clean removal
- 卸载清理工具，确保完全移除程序
- Single file executable, no dependencies
- 单文件可执行程序，无需依赖

---

## Notice / 注意！

1. **Windows that cannot be resized**：Try enabling "Run as Administrator" in settings to gain sufficient permissions to resize system-level windows.

2. **Configuration file location**：The program will generate a `settings.ini` configuration file in the `%APPDATA%\WindowResizer-imgui` directory. This file is used to save:
   - User interface settings (theme, language, etc.)
   - Software window position records
   - Program run parameters
   - Other user preferences

3. **Clean removal methods**：
   - **Manual removal**：
     1. Delete the program executable `WindowResizer-imgui.exe`
     2. Delete the configuration directory `%APPDATA%\WindowResizer-imgui`
   - **Using batch tool**：Run the `uninstall.bat` script in the project root directory and follow the prompts to select deletion options
---
1. **遇到不能调整大小的窗口**：可以尝试在设置里勾选「启用管理员运行」选项，以获取足够的权限调整系统级窗口。

2. **配置文件位置**：程序运行后会在 `%APPDATA%\WindowResizer-imgui` 目录生成 `settings.ini` 配置文件。该文件主要用于保存：
   - 用户界面设置（主题、语言等）
   - 软件窗口位置记录
   - 程序运行参数
   - 其他用户偏好设置

3. **无残留删除方法**：
   - **手动删除**：
     1. 删除程序可执行文件 `WindowResizer-imgui.exe`
     2. 删除配置文件目录 `%APPDATA%\WindowResizer-imgui`
   - **使用批处理工具**：运行项目根目录下的 `uninstall.bat` 脚本，按照提示选择删除选项

---

## Screenshot / 程序截图

![Screenshot](screenshot.png)

## Build / 编译

### Requirements / 环境要求

- Visual Studio 2022 (v143 toolset)
- Windows SDK
- DirectX 11 SDK (usually included with Windows SDK / 通常已包含在 Windows SDK 中)
- ImGui Docking Branch
- LXGWWenKaiLite font for Chinese support

### Build Steps / 编译步骤

1. Open `WindowResizer-imgui.sln` in Visual Studio
2. Select `Release | x64` configuration
3. Build solution
---
1. 在 Visual Studio 中打开 `WindowResizer-imgui.sln`
2. 选择 `Release | x64` 配置
3. 生成解决方案

## Usage / 使用方法

1. Run `WindowResizer-imgui.exe`
2. Click "开始查找窗口" button
3. Click on the target window you want to resize
4. Select resize mode and click "应用设置"
---
1. 运行 `WindowResizer-imgui.exe`
2. 点击"开始查找窗口"按钮
3. 点击要调整的目标窗口
4. 选择调整模式并点击"应用设置"

## Project Structure / 项目结构

```
WindowResizer-ImGui/
├── src/                                      # Main application code / 主程序代码
│   ├── main.cpp
│   ├── App.h
│   ├── Config.cpp/h
│   ├── DirectXManager.cpp/h
│   ├── FontConfig.cpp/h
│   ├── LanguageManager.cpp/h
│   ├── RenderUI.cpp/h
│   ├── Settings.cpp/h
│   ├── Theme.cpp/h
│   ├── Utils.cpp/h
│   └── WindowManager.cpp/h
├── imgui/
│   ├── backends/
│   │   ├── imgui_impl_dx11.cpp/h             # DirectX11 backend / DirectX11 后端
│   │   └── imgui_impl_win32.cpp/h            # Win32 backend / Win32 后端
│   └── imgui*.cpp/h                          # ImGui core files / ImGui 核心文件
├── project/
│   ├── LXGWWenKaiLite-Regular.ttf            # Font file / 字体文件
│   ├── WindowResizer-imgui.vcxproj           # VS project file / VS 项目文件
│   └── WindowResizer-imgui.vcxproj.filters   # VS project filters / VS 项目过滤器
├── res/
│   ├── WindowResizer.ico                     # Application icon / 应用图标
│   └── resource.rc                           # Resource file / 资源文件
├── WindowResizer-imgui.sln                   # VS solution file / VS 解决方案文件
├── LICENSE                                   # License file / 许可证文件
├── README.md                                 # This file / 本文件
├── screenshot.png                            # Screenshot / 程序截图
└── uninstall.bat                             # Uninstall script / 卸载脚本
```

## License / 许可证

This project is licensed under the MIT License.

本项目采用 MIT 许可证。

Copyright (c) 2019 inkuang  
Modified by siuzhiyu 2026

See the [LICENSE](LICENSE) file for details.

详见 [LICENSE](LICENSE) 文件。

## Credits / 致谢

- Original author: inkuang (https://github.com/inkuang/WindowResizer)
- ImGui: Omar Cornut (https://github.com/ocornut/imgui)
- ImGui Docking Branch: Enhanced UI with docking and viewport support

