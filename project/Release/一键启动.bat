@echo off
setlocal

:: 定义 WindowResizer 的完整路径
set "WINDOW_RESIZER=D:\ WindowResizer\project\Release\WindowResizer-imgui.exe"

:: 定义目标程序
set "TARGET_EXE=O:\SteamLibrary\steamapps\common\BlackMythWukong\b1.exe"

:: 定义调整参数  :: 使用进程名方式
set "WR_ARGS=-p b1.exe -size 2560 1440 -center -hidetitle -nowait"

:: 定义传递参数 如果有重叠参数 那就在=后面加-- 开头用来分隔
set "TARGET_ARGS=-fileopenlog"

:: 执行
::"%WINDOW_RESIZER%" "%TARGET_EXE%" %WR_ARGS% %TARGET_ARGS%

"%TARGET_EXE%"

"%WINDOW_RESIZER%" %WR_ARGS% %TARGET_ARGS%
pause
::endlocal