#!/bin/bash
set +x

pushd .. > /dev/null
vendor/premake/bin/premake5.exe vs2022
popd > /dev/null

read -p "Press [Enter] key to continue..."
