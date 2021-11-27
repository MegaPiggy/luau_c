@if exist cmake (
    @echo Sub folder found. Deleting . . . 
    @rd /S /Q cmake
)
@mkdir cmake && cd cmake
@cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=OFF -DCPR_BUILD_TESTS=OFF -DCMAKE_USE_OPENSSL=OFF -DBUILD_CPR_TESTS=OFF
@cmake --build . --target Luau.Repl.CLI --config RelWithDebInfo
@cmake --build . --target Luau.Analyze.CLI --config RelWithDebInfo

@cd ../
@mkdir bin
@copy /Y "%cd%\cmake\RelWithDebInfo\luau.exe" "%cd%\bin\luau.exe"
@copy /Y "%cd%\cmake\RelWithDebInfo\luau-analyze.exe" "%cd%\bin\luau-analyze.exe"
@copy /Y "%cd%\cmake\_deps\cpr-build\cpr\RelWithDebInfo\cpr.dll" "%cd%\bin\cpr.dll"
@copy /Y "%cd%\cmake\_deps\curl-build\lib\RelWithDebInfo\libcurl.dll" "%cd%\bin\libcurl.dll"