@echo off
REM ZPdevtools Build Script for MS-DOS 4.01+
REM Compatible with Turbo C 2.0 and Microsoft C 6.0
REM
REM Requirements:
REM   - MS-DOS 4.01 or higher
REM   - 80286 CPU (or better)
REM   - 2 MB RAM minimum
REM   - 100 MB hard drive space
REM   - Turbo C 2.0 or Microsoft C 6.0

echo ========================================
echo   ZPdevtools MS-DOS Build Script
echo ========================================
echo.

REM Detect which compiler is available
set COMPILER=NONE

REM Check for Turbo C
if exist C:\TC\TCC.EXE (
    set COMPILER=TCC
    set PATH=C:\TC;%PATH%
    echo Found: Turbo C 2.0
    echo Compiler: TCC
    goto :compiler_found
)

REM Check for Microsoft C
if exist C:\MSC\BIN\CL.EXE (
    set COMPILER=MSC
    set PATH=C:\MSC\BIN;%PATH%
    set INCLUDE=C:\MSC\INCLUDE;%INCLUDE%
    set LIB=C:\MSC\LIB;%LIB%
    echo Found: Microsoft C 6.0
    echo Compiler: CL
    goto :compiler_found
)

REM No compiler found
echo.
echo ERROR: No DOS C compiler found!
echo.
echo Please install one of:
echo   - Turbo C 2.0 in C:\TC\
echo   - Microsoft C 6.0 in C:\MSC\
echo.
goto :end

:compiler_found
echo.

REM Create output directory
if not exist BIN mkdir BIN
echo Output directory: BIN\
echo.

REM Check if Makefile.dos exists
if not exist Makefile.dos (
    echo ERROR: Makefile.dos not found!
    echo Please ensure you are in the ZPdevtools directory.
    goto :end
)

echo Building all development tools...
echo Please wait, this may take several minutes on a 286...
echo.

REM Build with appropriate make tool
if "%COMPILER%"=="TCC" (
    REM Turbo C uses Borland Make
    if exist C:\TC\MAKE.EXE (
        C:\TC\MAKE.EXE -f Makefile.dos
    ) else (
        echo ERROR: Turbo Make not found at C:\TC\MAKE.EXE
        goto :end
    )
) else if "%COMPILER%"=="MSC" (
    REM Microsoft C uses NMAKE
    if exist C:\MSC\BIN\NMAKE.EXE (
        C:\MSC\BIN\NMAKE.EXE /f Makefile.dos COMPILER=MSC
    ) else (
        echo ERROR: NMAKE not found at C:\MSC\BIN\NMAKE.EXE
        goto :end
    )
)

if errorlevel 1 goto :build_error

echo.
echo ========================================
echo   Build Complete!
echo ========================================
echo.
echo Built tools in BIN\ directory:
echo.
echo Assemblers:
if exist BIN\CPUASM.EXE echo   [OK] CPUASM.EXE
if exist BIN\APUASM.EXE echo   [OK] APUASM.EXE
if exist BIN\PPUASM.EXE echo   [OK] PPUASM.EXE
echo.
echo Disassemblers:
if exist BIN\CPUDISASM.EXE echo   [OK] CPUDISASM.EXE
if exist BIN\APUDISASM.EXE echo   [OK] APUDISASM.EXE
if exist BIN\PPUDISASM.EXE echo   [OK] PPUDISASM.EXE
echo.
echo ROM Tools:
if exist BIN\ROMBUILDER.EXE echo   [OK] ROMBUILDER.EXE
if exist BIN\ROMINSPECT.EXE echo   [OK] ROMINSPECT.EXE
echo.
echo Utilities:
if exist BIN\HEXVIEW.EXE echo   [OK] HEXVIEW.EXE
if exist BIN\WAV2MMP.EXE echo   [OK] WAV2MMP.EXE
echo.
echo C Compiler:
if exist BIN\DEF88186CC.EXE echo   [OK] DEF88186CC.EXE
echo.
echo Usage:
echo   BIN\PPUASM examples\ppu\test.asm output.bin
echo   BIN\CPUASM examples\cpu\test.asm output.bin
echo   BIN\ROMBUILDER -cpu cpu.bin -ppu ppu.bin -o game.rom
echo.
echo NOTE: All tools use C89-compliant code for DOS compatibility.
echo       See README_DOS.TXT for detailed information.
echo.
goto :end

:build_error
echo.
echo ========================================
echo   Build Failed!
echo ========================================
echo.
echo Please check:
echo   1. Compiler is properly installed
echo   2. All source files are present
echo   3. Sufficient disk space (100 MB minimum)
echo   4. Sufficient memory (2 MB minimum)
echo.
echo For help, see:
echo   - README_DOS.TXT
echo   - DOS_BUILD.MD
echo   - C89_PORTING_GUIDE.TXT
echo.

:end
