xcopy /s /Y "..\FLHookSDK\vcpkg.json" ".."
../vcpkg/bootstrap-vcpkg.bat
../vcpkg/vcpkg integrate install