@echo off
setlocal
cd /d "%~dp0"

set "EDITOR=bin\godot.windows.editor.x86_64.exe"
set "PROJECT=platform\psp\demo_3d"

if not exist "%EDITOR%" (
  echo Godot PSP editor is not built yet.
  echo Run: scons platform=windows target=editor angle=no d3d12=no vulkan=no -j8
  pause
  exit /b 1
)

start "Godot 4.7 PSP" "%EDITOR%" --editor --path "%CD%\%PROJECT%"
