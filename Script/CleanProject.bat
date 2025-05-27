@echo off
pushd ..\
echo Cleaning Visual Studio files...

del /s /q *.sln
del /s /q *.vcxproj
del /s /q *.vcxproj.filters
del /s /q *.vcxproj.user
del /s /q *.suo
del /s /q *.sdf

echo Done!

popd
pause