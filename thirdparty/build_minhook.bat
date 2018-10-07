cd minhook
git apply ..\minhook.patch
msbuild build\VC15\MinHookVC15.sln /t:libMinHook /p:Configuration=Debug /p:Platform=Win32
msbuild build\VC15\MinHookVC15.sln /t:libMinHook /p:Configuration=Debug /p:Platform=x64
msbuild build\VC15\MinHookVC15.sln /t:libMinHook /p:Configuration=Release /p:Platform=Win32
msbuild build\VC15\MinHookVC15.sln /t:libMinHook /p:Configuration=Release /p:Platform=x64
cd ..
