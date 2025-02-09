#!/bin/bash

# Change to the parent directory
cd ..

# Delete all `bin` folders except `Vendor/premake/bin`
find . -type d -name "bin" -not -path "./Vendor/premake/bin" -exec rm -rf {} +

# Delete all `bin-int` folders
find . -type d -name "bin-int" -exec rm -rf {} +

# Remove all .dll, .exe, .pdb files from ./CGraphicsSandbox
targetDir="./CGraphicsSandbox"
if [ -d "$targetDir" ]; then
    echo "Deleting .dll, .exe, and .pdb files from $targetDir"
    find "$targetDir" -type f \( -iname "*.dll" -o -iname "*.exe" -o -iname "*.pdb" \) -exec rm -f {} +
fi

# Return to the original directory
cd -

# Pause the script (keeps the terminal open until the user presses a key)
read -p "Press any key to continue..."
