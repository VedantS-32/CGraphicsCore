#!/bin/bash

# Change to the parent directory
cd ..

# Find and delete all Makefile files recursively
find . -type f -name "Makefile" -exec rm -f {} +

# Return to the original directory
cd -

# Pause the script (keeps the terminal open until the user presses a key)
read -p "Press any key to continue..."
