mkdir obj\oidc-prompt
mkdir bin

cl /c src\oidc-prompt\webview.cc /I"windows\webview-build\include" /std:c++17 /EHsc /Fo"obj\oidc-prompt\webview.obj"
cl /c src\oidc-prompt\oidc_webview.c /I"src" /Fo"obj\oidc-prompt\oidc_webview.obj"
link obj\oidc-prompt\oidc_webview.obj obj\oidc-prompt\webview.obj windows\webview-build\WebView2Loader.dll.lib /OUT:"bin\oidc-webview.exe"

cp windows/webview-build/dll/* bin\