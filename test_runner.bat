@echo off
REM Test runner script for Data Cleaning Toolkit

echo Data Cleaning Toolkit - Test Runner
echo ====================================

if "%1"=="" goto help
if "%1"=="unit" goto unit_tests
if "%1"=="e2e" goto e2e_tests
if "%1"=="all" goto all_tests

:help
echo.
echo Usage:
echo   test_runner.bat unit    - Run C++ unit tests
echo   test_runner.bat e2e     - Run Playwright E2E tests
echo   test_runner.bat all     - Run all tests
echo.
goto end

:unit_tests
echo.
echo Running C++ Unit Tests...
echo -------------------------
if not exist build-tests (
    echo Creating build directory for tests...
    mkdir build-tests
)
cd build-tests
cmake .. -DBUILD_TESTS=ON -DCMAKE_TOOLCHAIN_FILE="../vcpkg/scripts/buildsystems/vcpkg.cmake"
if errorlevel 1 goto error
cmake --build .
if errorlevel 1 goto error
echo.
echo Running unit tests...
.\runUnitTests.exe
cd ..
goto end

:e2e_tests
echo.
echo Running E2E Tests...
echo --------------------
cd tests\e2e
npm test
cd ..\..
goto end

:all_tests
call :unit_tests
call :e2e_tests
goto end

:error
echo Error occurred during test setup/build
exit /b 1

:end