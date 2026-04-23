# dev001 分支与 main 分支对比分析

## 1. 核心技术变更

### 1.1 图形API升级
- **从DirectX 9升级到DirectX 11**
  - main分支：使用DirectX 9进行渲染
  - dev001分支：升级到DirectX 11，提供更好的性能和稳定性
  - 相关文件：`src/main.cpp`、`src/DirectXManager.cpp`

### 1.2 ImGui功能增强
- **添加Docking和Viewports支持**
  - 启用`ImGuiConfigFlags_DockingEnable`和`ImGuiConfigFlags_ViewportsEnable`
  - 提供更灵活的UI布局和多窗口支持
- **字体系统优化**
  - 添加对`LXGWWenKaiLite-Regular.ttf`字体的支持
  - 优化字体渲染设置，如禁用过度采样、启用像素对齐
  - 相关文件：`src/main.cpp`、`src/FontConfig.cpp`

## 2. 功能新增

### 2.1 窗口管理功能
- **窗口居中**：`CenterWindow()` 函数实现窗口在当前显示器居中
- **窗口最大化**：`MaximizeWindow()` 函数实现窗口最大化
- **窗口还原**：`RestoreWindow()` 函数还原窗口到初始大小
- **标题栏切换**：`ToggleTitleBar()` 函数可隐藏/显示窗口标题栏
- **多种调整模式**：
  - 直接设置宽高
  - 按预设比例缩放
  - 按自定义比例缩放
  - 相关文件：`src/WindowManager.cpp`

### 2.2 配置管理
- **新增配置系统**：`Config.cpp` 和 `Config.h` 实现配置的保存和加载
- **设置管理**：`Settings.cpp` 和 `Settings.h` 实现应用设置的管理
- **语言管理**：`LanguageManager.cpp` 和 `LanguageManager.h` 实现多语言支持
- **主题管理**：`Theme.cpp` 和 `Theme.h` 实现主题切换功能

### 2.3 工具函数增强
- **实用工具**：`Utils.cpp` 和 `Utils.h` 提供各种辅助函数
- **DirectX管理**：`DirectXManager.cpp` 和 `DirectXManager.h` 专门管理DirectX 11相关操作

## 3. 代码结构优化

### 3.1 模块化重构
- **分离关注点**：将功能分散到多个模块，如配置、字体、语言、主题等
- **清晰的职责划分**：每个模块负责特定的功能，提高代码可维护性
- **相关文件**：`src/` 目录下的多个新文件

### 3.2 渲染循环改进
- **优化消息处理**：改进了Windows消息处理逻辑
- **性能优化**：添加了渲染条件判断，减少不必要的渲染
- **Viewport支持**：添加了对ImGui Viewport的完整支持
- **相关文件**：`src/WindowManager.cpp` 中的 `RenderLoop()` 函数

## 4. 用户体验改进

### 4.1 界面优化
- **主题系统**：实现了可切换的主题系统
- **字体优化**：使用更美观的LXGWWenKaiLite字体
- **响应式布局**：适配不同DPI和窗口大小

### 4.2 操作流程优化
- **窗口选择**：改进了窗口选择的交互方式
- **设置应用**：更直观的设置应用流程
- **状态反馈**：提供更清晰的操作状态反馈

## 5. 其他变更

### 5.1 项目配置
- **更新项目文件**：`project/WindowResizer-imgui.vcxproj` 适配DirectX 11
- **添加字体文件**：`project/LXGWWenKaiLite-Regular.ttf`

### 5.2 脚本改进
- **增强卸载脚本**：`uninstall.bat` 脚本功能更完善，支持多种卸载选项

### 5.3 文档更新
- **更新README.md**：反映项目的最新状态和功能

## 6. 总结

dev001分支相对于main分支进行了全面的升级和改进，主要包括：

1. **技术栈升级**：从DirectX 9升级到DirectX 11，提供更好的性能和稳定性
2. **功能增强**：添加了多种窗口管理功能，如居中、最大化、还原、标题栏切换等
3. **架构优化**：模块化重构，提高代码可维护性
4. **用户体验**：改进界面设计和操作流程
5. **系统集成**：更好的多语言支持和主题系统

这些变更使WindowResizer-ImGui成为一个功能更强大、性能更优、用户体验更好的窗口调整工具。