@echo off
setlocal
rem ###########################################################################
rem Configure, build, test, and run 8080 program on Windows.
rem This script was generated with the help of AI (duck.ai with model GPT-6 Luna).
rem The accompanying transcript documents the GenAI interaction that led to this
rem script:
rem https://drive.google.com/file/d/1zb7rj9H_c__12t-J0Ur3wHisPcyrhMm5/view?usp=drive_link
rem ###########################################################################

rem Collect arguments for 8080; recognize -e as the test option.
set "RUNEXM=0"
set "HELP=0"
set "APP_ARGS="

:parse_args
if "%~1"=="" goto args_done
if "%~1"=="-e" goto option_e
if "%~1"=="-h" goto option_h


rem Treat non-options as arguments to 8080 program.
set APP_ARGS=%APP_ARGS% "%~1"
shift
goto parse_args

:option_e
set "RUNEXM=1"
shift
goto parse_args

:option_h
set "HELP=1"
shift
goto parse_args

:args_done

if "%HELP%"=="1" (
    echo Usage: build [8080_args] [-opts]
    echo Options:
    echo   -e: Run 8080EXM test. Skipped by default ^(takes ~30s to run^)
    echo   -h: Get usage info
    exit /b 0
)

rem Step 1: Configure
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 exit /b 1

rem Step 2: Build
cmake --build build --config Release --parallel
if errorlevel 1 exit /b 1

rem Step 3: Test
if "%RUNEXM%"=="1" (
    ctest --test-dir build --build-config Release --output-on-failure
) else (
    ctest --test-dir build --build-config Release --output-on-failure -E 8080EXM
)
if errorlevel 1 exit /b 1

rem Step 4: Run
echo Starting game...
build\Release\8080.exe %APP_ARGS%