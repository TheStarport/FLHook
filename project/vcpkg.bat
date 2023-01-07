xcopy /s /Y "..\FLHookSDK\vcpkg.json" ".."
call "../vcpkg/bootstrap-vcpkg.bat -disableMetrics"
start /WAIT ../vcpkg/vcpkg integrate install
pause