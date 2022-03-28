@echo off

echo Prepare directories...
set script_dir=%~dp0
set root_dir=%script_dir%..\..
set src_dir=%root_dir%\src
set obj_dir=%root_dir%\obj
set bin_dir=%root_dir%\windows\webview
mkdir "%obj_dir%\oidc-prompt"
mkdir "%bin_dir%"

echo Looking for vswhere.exe...
set "vswhere=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%vswhere%" set "vswhere=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%vswhere%" (
	echo ERROR: Failed to find vswhere.exe
	exit /b 1
)
echo Found %vswhere%

echo Looking for VC...
for /f "usebackq tokens=*" %%i in (`"%vswhere%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
  set vc_dir=%%i
)
if not exist "%vc_dir%\Common7\Tools\vsdevcmd.bat" (
	echo ERROR: Failed to find VC tools x86/x64
	exit /b 1
)
echo Found %vc_dir%

call "%vc_dir%\Common7\Tools\vsdevcmd.bat" -arch=x64 -host_arch=x64

echo Building objects
cl /c %src_dir%\oidc-prompt\webview.cc /I"%root_dir%\windows\webview-build\include" /std:c++17 /EHsc /Fo"%obj_dir%\oidc-prompt\webview.obj" || exit /b 1
cl /c %src_dir%\oidc-prompt\oidc_webview.c /I"%src_dir%" /Fo"%obj_dir%\oidc-prompt\oidc_webview.obj" || exit /b 1
	
	
echo Linking oidc-webview.exe (x64)
link %obj_dir%\oidc-prompt\oidc_webview.obj %obj_dir%\oidc-prompt\webview.obj %script_dir%\WebView2Loader.dll.lib /OUT:"%bin_dir%\oidc-webview.exe" || exit /b 1


