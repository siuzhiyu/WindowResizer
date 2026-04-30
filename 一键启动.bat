@echo off
setlocal

:: 定义 WindowResizer 的完整路径
set "WINDOW_RESIZER=D:\ WindowResizer\project\Release\WindowResizer-imgui.exe"

:: 定义目标程序
set "TARGET_EXE=G:\Program Files (x86)\WinKawaks KOF97Plus\WinKawaks.exe"

:: 定义调整参数
set "WR_ARGS=-size 2560 1440 -center -hidetitle -nowait"

:: 定义传递参数 如果有重叠参数 那就在=后面加-- 开头用来分隔
set "TARGET_ARGS="

:: 执行
"%WINDOW_RESIZER%" "%TARGET_EXE%" %WR_ARGS% %TARGET_ARGS%

endlocal