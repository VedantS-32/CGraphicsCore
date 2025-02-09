#!/bin/bash
set +x

pushd .. > /dev/null
Vendor/premake/bin/premake5 gmake2
popd > /dev/null

read -p "Press [Enter] key to continue..."
