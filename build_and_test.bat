@echo off
setlocal

set "Configuration=%1"

if "%Configuration%"=="" (
    set "Configuration=Debug"
)

cmake --build "build" --config "%Configuration%" && ^
ctest --test-dir "build" --build-config "%Configuration%" --output-on-failure
