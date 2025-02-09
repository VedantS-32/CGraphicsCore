@echo off
pushd ..\
for /d /r . %%d in (bin) do (
    set "folderPath=%%d"
    if not "%%d"=="%CD%\Vendor\premake\bin" (
        if exist "%%d" rd /s /q "%%d"
    )
)
for /d /r . %%d in (bin-int) do (
    if exist "%%d" rd /s /q "%%d"
)
popd
PAUSE