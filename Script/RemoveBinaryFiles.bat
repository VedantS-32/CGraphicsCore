@echo off
pushd ..\

:: Delete all `bin` folders except `Vendor\premake\bin`
for /d /r . %%d in (bin) do (
    if not "%%d"=="%CD%\Vendor\premake\bin" (
        if exist "%%d" rd /s /q "%%d"
    )
)

:: Delete all `bin-int` folders
for /d /r . %%d in (bin-int) do (
    if exist "%%d" rd /s /q "%%d"
)

:: Remove all .dll, .exe, .pdb files from RootDir/CGraphicsSandbox
set "targetDir=%CD%\CGraphicsSandbox"
if exist "%targetDir%" (
    echo Deleting .dll, .exe, and .pdb files from %targetDir%
    del /s /q "%targetDir%\*.dll"
    del /s /q "%targetDir%\*.exe"
    del /s /q "%targetDir%\*.pdb"
)

popd
PAUSE
