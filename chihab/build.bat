@echo off
rem script setup ----------------------------------------------------------------
cd /D "%~dp0"
setlocal
if exist C:\vc_env\msvc\setup.bat @call "C:\vc_env\msvc\setup.bat"

rem Get command-line arguments --------------------------------------------------
for %%a in (%*) do set "%%a=1"
if not "%msvc%"=="1" if not "%clang%"=="1" set msvc=1
if not "%release%"=="1" set debug=1
if "%debug%"=="1"   set release=0 && echo [debug mode]
if "%release%"=="1" set debug=0 && echo [release mode]
if "%msvc%"=="1"    set clang=0 && echo [msvc compile]
if "%clang%"=="1"   set msvc=0 && echo [clang compile]

set src_dir=.
set program=test
set bin=bin

if not exist %bin% md %bin%
if exist %bin%\%program%.* del %bin%\%program%.*

rem common options --------------------------------------------------------------
set ext=.exe
set out_path=%bin%\%program%

rem MSVC compiler options -------------------------------------------------------
set cc_options=/options:strict /EHsc /EHa- /GS- /GR- /guard:cf- /FC /c /Tc
set cc_diag=/diagnostics:caret /Zc:preprocessor /Zc:auto
rem set show_includes=/showIncludes
set cc=cl %show_includes% /nologo %cc_diag% %cc_options%
set cc_includes=/I%src_dir% /Ivendor
rem set cc_asan=/fsanitize=address
set cc_def=/DDEBUG
rem set cc_def=/DDEBUG /DNDEBUG /DRYU_NOCRT

set cc_pdb_out=/Fd:%out_path%.pdb
set cc_obj=%out_path%.obj
set cc_out=/Fo:%cc_obj% %cc_pdb_out%

rem (4090)const/volatile (4189)var init unused (5045)spectre mitigation
set cc_warn=/Wall /WX /wd4090 /wd4189 /wd5045 /wd4101

set cc_std=/experimental:c11atomics /std:clatest
set cc_opti=/Od /Oi
rem set cc_debug=/Zl
set cc_debug=%cc_debug% /Zi

set cc_flags=%cc_warn% %cc_def% %cc_debug% /Zl /Gs9999999 %cc_std% %cc_opti% %cc_includes% %cc_asan%

rem MSVC linker options ---------------------------------------------------------
set linker=link /nologo /INCREMENTAL:NO
set l_debug=/DEBUG:FULL /PDB:%out_path%.pdb /DYNAMICBASE:NO
rem set l_sys=/SUBSYSTEM:WINDOWS /ENTRY:WinMainCRTStartup /PROFILE /GUARD:NO
rem set l_sys=/SUBSYSTEM:WINDOWS /ENTRY:WinMainCRTStartup /PROFILE /GUARD:NO
set l_arch=/MACHINE:X64 /STACK:0x100000,0x100000
rem set l_options=/WX %l_arch% /NODEFAULTLIB %l_sys%
set l_options=/WX %l_arch% %l_sys%

set d3d12libs=d3dcompiler.lib D3D12.lib dxgi.lib dxguid.lib

set asanlibs=clang_rt.asan_static_runtime_thunk-x86_64.lib
set libs=shlwapi.lib Kernel32.lib User32.lib Gdi32.lib %asanlibs% %d3d12libs%

set l_files=/ILK:%out_path%.ilk /MAP:%out_path%.map 

rem set l_delay=/DELAYLOAD:d3d12.dll
rem set l_opti=/LTCGU
set l_all=%l_debug% %l_options% %libs% %l_delay% %l_opti%
set l_out=/OUT:%out_path%%ext%

rem compilation -----------------------------------------------------------------
%cc% %src_dir%\test.c %cc_flags% %cc_out%
if not %errorlevel% == 0 (exit /b %errorlevel%)

rem linking ---------------------------------------------------------------------
%linker% %cc_obj% %bin%\*.res %l_files% %l_out% %l_all% %asanlibs%
if not %errorlevel% == 0 (exit /b %errorlevel%)

rem launch it -------------------------------------------------------------------
rem %out_path%%ext%

rem misc ------------------------------------------------------------------------
ctags -f tags --langmap=c:.c.h --languages=c -R .
