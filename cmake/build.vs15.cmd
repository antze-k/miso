@echo off
set SCRIPT=%~0
for %%F in ("%SCRIPT%") do set SCRIPT_DIR=%%~dpF

set BUILD_DIR=%SCRIPT_DIR%vs15
set SRC_DIR=%SCRIPT_DIR%

echo Creating build directory %BUILD_DIR%
mkdir %BUILD_DIR%
pushd %BUILD_DIR%

cmake -G "Visual Studio 15 2017" %SRC_DIR%
cmake --build . --config MinSizeRel

popd
