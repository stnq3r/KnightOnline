@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"

echo ==========================================
echo  OpenKO client build (Release ^| Win32)
echo ==========================================

where git >nul 2>nul
if errorlevel 1 (
	echo ERROR: git.exe not found in PATH. Install Git for Windows.
	exit /b 1
)

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
	echo ERROR: vswhere.exe not found. Install Visual Studio 2022.
	exit /b 1
)

set "MSBUILD="
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -version 17.0 -latest -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do set "MSBUILD=%%i"

if "%MSBUILD%"=="" (
	echo ERROR: MSBuild.exe not found. Install Visual Studio 2022 with the "Desktop development with C++" workload.
	exit /b 1
)

echo Using MSBuild: %MSBUILD%
echo.

"%MSBUILD%" /m /p:Configuration=Release /p:Platform=Win32 Client.slnx
if errorlevel 1 (
	echo.
	echo ERROR: Build failed.
	exit /b 1
)

echo.
echo Copying the client exe into assets\Client ...
if not exist "bin\Release-Win32\KnightOnLine.exe" (
	echo ERROR: bin\Release-Win32\KnightOnLine.exe was not produced.
	exit /b 1
)
copy /Y "bin\Release-Win32\KnightOnLine.exe" "assets\Client\KnightOnLine.exe" >nul

echo.
echo Build complete. Client ready at: assets\Client\KnightOnLine.exe
echo Launch it with: run_client.bat
exit /b 0
