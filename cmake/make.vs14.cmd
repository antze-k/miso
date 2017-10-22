@echo off
set SCRIPT=%~0
for %%F in ("%SCRIPT%") do set SCRIPT_DIR=%%~dpF

set BUILD_DIR=%SCRIPT_DIR%vs14
set SRC_DIR=%SCRIPT_DIR%

echo Creating build directory %BUILD_DIR%
rmdir %BUILD_DIR% /S /Q
mkdir %BUILD_DIR%
pushd %BUILD_DIR%

cmake -G "Visual Studio 14 2015" %SRC_DIR%

popd
