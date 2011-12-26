@echo off
for /R %%i IN (*.S) DO "C:\Program Files\devkitPro\devkitPPC\bin\powerpc-gekko-as.exe" %%i -o %%~ni.elf
for /R %%i IN (*.S) DO "C:\Program Files\devkitPro\devkitPPC\bin\powerpc-gekko-strip.exe" -s %%~ni.elf -O binary -o %%~ni.bin
for /R %%i in (*.bin) DO "..\..\bin2h\bin2h.exe" %%~ni.bin
REM for /R %%i IN (*.S) DO "C:\Programme\devkitPro\devkitPPC\bin\powerpc-eabi-as.exe" %%i -o %%~ni.elf
REM for /R %%i IN (*.S) DO "C:\Programme\devkitPro\devkitPPC\bin\powerpc-eabi-strip.exe" -s %%~ni.elf -O binary -o %%~ni.bin
REM for /R %%i in (*.bin) DO "..\..\bin2h\bin2h.exe" %%~ni.bin
