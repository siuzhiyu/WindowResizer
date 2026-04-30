@echo off
setlocal

set "WINDOW_RESIZER=D:\ WindowResizer\project\Release\WindowResizer-imgui.exe"
set "GAME_EXE=O:\SteamLibrary\steamapps\common\BlackMythWukong\b1.exe"

:: 启动游戏
start "" "%GAME_EXE%"

:: 等待游戏窗口出现
timeout /t 20 /nobreak >nul

:: 使用进程名调整窗口
"%WINDOW_RESIZER%" -p b1.exe -size 2560 1440 -center -hidetitle
pause
::endlocal