@echo off
cd /d "%~dp0assets\Client"
if not exist "KnightOnLine.exe" (
	echo ERROR: KnightOnLine.exe not found. Build it first with build_client_Release_Win32.bat
	pause
	exit /b 1
)
start "" "KnightOnLine.exe"
