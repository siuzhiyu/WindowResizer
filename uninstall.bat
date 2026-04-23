@echo off
setlocal enabledelayedexpansion

:: ========================================
:: Language Detection & Setup
:: ========================================

:: Detect system language
for /f "tokens=3" %%a in ('reg query "HKCU\Control Panel\Desktop" /v PreferredUILanguages 2^>nul') do set "SYS_LANG=%%a"
if not defined SYS_LANG (
    for /f "tokens=3" %%a in ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Nls\Language" /v InstallLanguage 2^>nul') do set "LANG_CODE=%%a"
)

:: Check if language is Chinese (zh-CN, zh-TW, zh-HK, etc.)
set "IS_CHINESE=0"
if defined SYS_LANG (
    echo !SYS_LANG! | findstr /i "zh-" >nul 2>&1
    if !errorlevel! equ 0 set "IS_CHINESE=1"
)

:: If registry check failed, try language code method
if "!IS_CHINESE!"=="0" (
    if "!LANG_CODE!"=="0804" set "IS_CHINESE=1"
    if "!LANG_CODE!"=="0404" set "IS_CHINESE=1"
    if "!LANG_CODE!"=="0C04" set "IS_CHINESE=1"
    if "!LANG_CODE!"=="1404" set "IS_CHINESE=1"
)

:: Set code page based on language
if "!IS_CHINESE!"=="1" (
    chcp 936 >nul 2>&1
) else (
    chcp 437 >nul 2>&1
)

:: Define language-specific texts
if "!IS_CHINESE!"=="1" (
    set "TITLE=WindowResizer-imgui 卸载工具"
    set "DETECTING=正在检测"
    set "CONFIG_FOUND=[配置文件] 找到"
    set "VERSION_INFO=[版本信息]"
    set "INSTALL_PATH_LABEL=[安装路径]"
    set "EXE_LABEL=[可执行文件]"
    set "SEARCHING=[搜索] 未找到路径，正在搜索常见位置..."
    set "FOUND=[找到]"
    set "WARNING_PATH=[警告] 记录的安装路径不存在"
    set "DETECTION_RESULTS=检测结果"
    set "EXE_PATH_LABEL=可执行文件"
    set "INSTALL_DIR_LABEL=安装目录"
    set "ERROR_NOT_FOUND=[错误] 未找到软件可执行文件"
    set "CONFIG_FILE_LABEL=配置文件"
    set "CONFIG_NOT_EXISTS=不存在"
    set "DELETE_OPTIONS=删除选项"
    set "OPT1=1. 仅删除配置文件"
    set "OPT2=2. 仅删除软件"
    set "OPT3=3. 完全删除(配置文件 + 软件)"
    set "OPT4=4. 退出"
    set "SELECT_PROMPT=请选择选项 (1-4)"
    set "ERROR_INVALID=[错误] 无效的选择"
    set "DELETING_CONFIG=正在删除配置文件..."
    set "CONFIRM_CONFIG_DEL=确认要删除配置文件"
    set "CONFIRM_ALL_DEL=确认要完全删除配置文件及软件吗"
    set "SUCCESS_CONFIG_DEL=[成功] 配置文件已删除"
    set "FAILED_CONFIG_DEL=[失败] 配置文件删除失败"
    set "SUCCESS_CONFIG_DIR_DEL=[成功] 配置目录已删除"
    set "CANCELLED=[取消] 操作已取消"
    set "INFO_CONFIG_NOT_EXIST=[提示] 配置文件不存在"
    set "DELETING_SOFTWARE=正在删除软件..."
    set "CONFIRM_SOFTWARE_DEL=确认要删除软件"
    set "SUCCESS_SOFTWARE_DEL=[成功] 软件已删除"
    set "FAILED_SOFTWARE_DEL=[失败] 软件删除失败"
    set "SUCCESS_SOFTWARE_DIR_DEL=[成功] 安装目录已删除"
    set "ERROR_SOFTWARE_NOT_FOUND=[错误] 软件可执行文件未找到"
    set "DELETING_ALL=正在执行完全删除..."
    set "BACK_TO_MENU=操作完成，按任意键返回主菜单..."
    set "TEMP_PATH_LABEL=[缓存] 使用临时记录的安装路径"
) else (
    set "TITLE=WindowResizer-imgui Uninstall Tool"
    set "DETECTING=Detecting"
    set "CONFIG_FOUND=[Config] Found"
    set "VERSION_INFO=[Version]"
    set "INSTALL_PATH_LABEL=[Install Path]"
    set "EXE_LABEL=[Executable]"
    set "SEARCHING=[Search] Path not found, searching common locations..."
    set "FOUND=[Found]"
    set "WARNING_PATH=[Warning] Recorded install path does not exist"
    set "DETECTION_RESULTS=Detection Results"
    set "EXE_PATH_LABEL=Executable"
    set "INSTALL_DIR_LABEL=Install Dir"
    set "ERROR_NOT_FOUND=[Error] Software executable not found"
    set "CONFIG_FILE_LABEL=Config File"
    set "CONFIG_NOT_EXISTS=Not exists"
    set "DELETE_OPTIONS=Delete Options"
    set "OPT1=1. Delete config file only"
    set "OPT2=2. Delete software only"
    set "OPT3=3. Delete all (config + software)"
    set "OPT4=4. Exit"
    set "SELECT_PROMPT=Select option (1-4)"
    set "ERROR_INVALID=[Error] Invalid selection"
    set "DELETING_CONFIG=Deleting config file..."
    set "CONFIRM_CONFIG_DEL=Confirm delete config"
    set "CONFIRM_ALL_DEL=Confirm complete deletion of config and software"
    set "SUCCESS_CONFIG_DEL=[Success] Config deleted"
    set "FAILED_CONFIG_DEL=[Failed] Config deletion failed"
    set "SUCCESS_CONFIG_DIR_DEL=[Success] Config directory deleted"
    set "CANCELLED=[Cancelled] Operation cancelled"
    set "INFO_CONFIG_NOT_EXIST=[Info] Config file does not exist"
    set "DELETING_SOFTWARE=Deleting software..."
    set "CONFIRM_SOFTWARE_DEL=Confirm delete software"
    set "SUCCESS_SOFTWARE_DEL=[Success] Software deleted"
    set "FAILED_SOFTWARE_DEL=[Failed] Software deletion failed"
    set "SUCCESS_SOFTWARE_DIR_DEL=[Success] Install directory deleted"
    set "ERROR_SOFTWARE_NOT_FOUND=[Error] Software executable not found"
    set "DELETING_ALL=Performing complete deletion..."
    set "BACK_TO_MENU=Operation completed, press any key to return to main menu..."
    set "TEMP_PATH_LABEL=[Cache] Using temporary recorded install path"
)

title !TITLE!

:: ========================================
:: Software Information
:: ========================================
set "SOFTWARE_NAME=WindowResizer-imgui"
set "APPDATA_PATH=%APPDATA%\%SOFTWARE_NAME%"
set "SETTINGS_FILE=%APPDATA_PATH%\settings.ini"
set "TEMP_FILE=%TEMP%\%SOFTWARE_NAME%_uninstall_temp.txt"

:: ========================================
:: Main Menu Loop
:: ========================================
:main_menu
cls
echo ========================================
echo   !TITLE!
echo ========================================
echo.

:: Initialize variables
set "INSTALL_PATH="
set "EXE_PATH="
set "VERSION="

echo !DETECTING! %SOFTWARE_NAME%...
echo.

:: ========================================
:: Try config file first
:: ========================================
if exist "%SETTINGS_FILE%" (
    echo !CONFIG_FOUND!: %SETTINGS_FILE%
    
    :: Read version
    for /f "tokens=2 delims==" %%a in ('findstr /b "Version=" "%SETTINGS_FILE%"') do (
        set "VERSION_RAW=%%a"
    )
    
    if defined VERSION_RAW (
        set /a "VERSION_MAJOR=!VERSION_RAW!/100"
        set /a "VERSION_MINOR=!VERSION_RAW!%%100"
        echo !VERSION_INFO! v!VERSION_MAJOR!.!VERSION_MINOR!
    )
    
    :: Read install path
    for /f "tokens=1,* delims==" %%a in ('findstr /b "InstallPath=" "%SETTINGS_FILE%" 2^>nul') do (
        set "INSTALL_PATH=%%b"
    )
    
    if defined INSTALL_PATH (
        :: Remove leading/trailing spaces
        for /f "tokens=* delims= " %%a in ("!INSTALL_PATH!") do set "INSTALL_PATH=%%a"
        
        :: Verify path exists
        if exist "!INSTALL_PATH!" (
            :: Find executable
            for %%f in ("!INSTALL_PATH!\*.exe") do (
                set "fname=%%~nxf"
                echo !fname! | findstr /i "%SOFTWARE_NAME%" >nul
                if !errorlevel! equ 0 (
                    set "EXE_PATH=%%f"
                    echo !INSTALL_PATH_LABEL! !INSTALL_PATH!
                    echo !EXE_LABEL! !EXE_PATH!
                    goto :found_exe
                )
            )
        ) else (
            echo !WARNING_PATH!
            set "INSTALL_PATH="
        )
    )
)

:found_exe
:: ========================================
:: Try temp file if config doesn't have it
:: ========================================
if not defined EXE_PATH (
    if exist "%TEMP_FILE%" (
        echo !TEMP_PATH_LABEL!
        for /f "tokens=1,* delims==" %%a in ('findstr /b "InstallPath=" "%TEMP_FILE%" 2^>nul') do (
            set "INSTALL_PATH=%%b"
        )
        for /f "tokens=1,* delims==" %%a in ('findstr /b "ExePath=" "%TEMP_FILE%" 2^>nul') do (
            set "EXE_PATH=%%b"
        )
        :: Verify paths still exist
        if defined INSTALL_PATH (
            if exist "!INSTALL_PATH!" (
                if exist "!EXE_PATH!" (
                    echo !INSTALL_PATH_LABEL! !INSTALL_PATH!
                    echo !EXE_LABEL! !EXE_PATH!
                    goto :temp_found
                )
            )
        )
        :: Paths invalid, clear them
        set "INSTALL_PATH="
        set "EXE_PATH="
    )
)

:temp_found
:: ========================================
:: Search common locations if still not found
:: ========================================
if not defined EXE_PATH (
    echo !SEARCHING!
    
    for %%p in (
        "%ProgramFiles%\%SOFTWARE_NAME%"
        "%ProgramFiles(x86)%\%SOFTWARE_NAME%"
        "%USERPROFILE%\Desktop"
        "%USERPROFILE%\Downloads"
        "%USERPROFILE%\Documents"
    ) do (
        if exist "%%p" (
            for %%f in ("%%p\*.exe") do (
                set "fname=%%~nxf"
                echo !fname! | findstr /i "%SOFTWARE_NAME%" >nul
                if !errorlevel! equ 0 (
                    set "EXE_PATH=%%f"
                    set "INSTALL_PATH=%%p"
                    echo !FOUND! !EXE_PATH!
                    goto :search_done
                )
            )
        )
    )
)

:search_done
echo.

:: ========================================
:: Save installation info to temp file for future use
:: ========================================
if defined EXE_PATH (
    echo InstallPath=%INSTALL_PATH% > "%TEMP_FILE%"
    echo ExePath=%EXE_PATH% >> "%TEMP_FILE%"
)

:: ========================================
:: Display detection results
:: ========================================
echo ========================================
echo !DETECTION_RESULTS!:
echo ========================================
if defined EXE_PATH (
    echo !EXE_PATH_LABEL!: !EXE_PATH!
    echo !INSTALL_DIR_LABEL!: !INSTALL_PATH!
) else (
    echo !ERROR_NOT_FOUND!
)

if exist "%SETTINGS_FILE%" (
    echo !CONFIG_FILE_LABEL!: %SETTINGS_FILE%
) else (
    echo !CONFIG_FILE_LABEL!: !CONFIG_NOT_EXISTS!
)
echo.

:: ========================================
:: Display menu options
:: ========================================
echo ========================================
echo !DELETE_OPTIONS!:
echo ========================================
echo !OPT1!
echo !OPT2!
echo !OPT3!
echo !OPT4!
echo.

set /p CHOICE="!SELECT_PROMPT!: "

if "%CHOICE%"=="1" goto :delete_config
if "%CHOICE%"=="2" goto :delete_software
if "%CHOICE%"=="3" goto :delete_all
if "%CHOICE%"=="4" goto :exit_program
echo.
echo !ERROR_INVALID!
echo !BACK_TO_MENU!
pause >nul
goto :main_menu

:: ========================================
:: Delete config only
:: ========================================
:delete_config
echo.
echo !DELETING_CONFIG!
call :delete_config_files_with_confirm
echo.
echo !BACK_TO_MENU!
pause >nul
goto :main_menu

:: ========================================
:: Delete software only
:: ========================================
:delete_software
echo.
echo !DELETING_SOFTWARE!
if not defined EXE_PATH (
    echo !ERROR_SOFTWARE_NOT_FOUND!
    echo.
    echo !BACK_TO_MENU!
    pause >nul
    goto :main_menu
)
call :delete_software_files_with_confirm
echo.
echo !BACK_TO_MENU!
pause >nul
goto :main_menu

:: ========================================
:: Delete all (single confirmation)
:: ========================================
:delete_all
echo.
echo !DELETING_ALL!
echo.

set /p CONFIRM_ALL="!CONFIRM_ALL_DEL!? (Y/N): "
if /i "!CONFIRM_ALL!"=="Y" (
    call :delete_config_files_silent
    call :delete_software_files_silent
    :: Clean up temp file after complete deletion
    if exist "%TEMP_FILE%" del /f /q "%TEMP_FILE%" >nul 2>&1
) else (
    echo !CANCELLED!
)
echo.
echo !BACK_TO_MENU!
pause >nul
goto :main_menu

:: ========================================
:: Exit program
:: ========================================
:exit_program
echo.
echo !CANCELLED!
:: Clean up temp file
if exist "%TEMP_FILE%" del /f /q "%TEMP_FILE%" >nul 2>&1
echo.
pause
exit /b 0

:: ========================================
:: Delete config files (with confirmation)
:: ========================================
:delete_config_files_with_confirm
if exist "%SETTINGS_FILE%" (
    set /p CONFIRM="!CONFIRM_CONFIG_DEL! '%SETTINGS_FILE%'? (Y/N): "
    if /i "!CONFIRM!"=="Y" (
        call :delete_config_files_silent
    ) else (
        echo !CANCELLED!
    )
) else (
    echo !INFO_CONFIG_NOT_EXIST!
)
goto :eof

:: ========================================
:: Delete config files (silent, no confirmation)
:: ========================================
:delete_config_files_silent
if exist "%SETTINGS_FILE%" (
    del /f /q "%SETTINGS_FILE%" >nul 2>&1
    if exist "%SETTINGS_FILE%" (
        echo !FAILED_CONFIG_DEL!
    ) else (
        echo !SUCCESS_CONFIG_DEL!
        :: Check if directory is empty, then delete it
        dir "%APPDATA_PATH%" /b | findstr . >nul 2>&1
        if errorlevel 1 (
            rd "%APPDATA_PATH%" >nul 2>&1
            if not exist "%APPDATA_PATH%" (
                echo !SUCCESS_CONFIG_DIR_DEL!
            )
        )
    )
) else (
    echo !INFO_CONFIG_NOT_EXIST!
)
goto :eof

:: ========================================
:: Delete software files (with confirmation)
:: ========================================
:delete_software_files_with_confirm
if defined EXE_PATH (
    set /p CONFIRM="!CONFIRM_SOFTWARE_DEL! '%EXE_PATH%'? (Y/N): "
    if /i "!CONFIRM!"=="Y" (
        call :delete_software_files_silent
    ) else (
        echo !CANCELLED!
    )
) else (
    echo !ERROR_SOFTWARE_NOT_FOUND!
)
goto :eof

:: ========================================
:: Delete software files (silent, no confirmation)
:: ========================================
:delete_software_files_silent
if defined EXE_PATH (
    del /f /q "%EXE_PATH%" >nul 2>&1
    if exist "%EXE_PATH%" (
        echo !FAILED_SOFTWARE_DEL!
    ) else (
        echo !SUCCESS_SOFTWARE_DEL!
        :: Check if directory is empty, then delete it
        if defined INSTALL_PATH (
            dir "%INSTALL_PATH%" /b | findstr . >nul 2>&1
            if errorlevel 1 (
                rd "%INSTALL_PATH%" >nul 2>&1
                if not exist "%INSTALL_PATH%" (
                    echo !SUCCESS_SOFTWARE_DIR_DEL!
                )
            )
        )
    )
) else (
    echo !ERROR_SOFTWARE_NOT_FOUND!
)
goto :eof
