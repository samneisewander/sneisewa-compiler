#!/bin/bash

# Reset all formatting
RESET="\e[0m"

# Foreground colors
RED="\e[31m"
GREEN="\e[32m"

# Non-linking tests
for testfile in test/codegen/non-linking/good*.bminor
do
	if bin/bminor --codegen $testfile $testfile.s &> $testfile.out
	then
		echo -e "$testfile success ${GREEN}(as expected)${RESET}"
	else
		echo -e "$testfile failure ${RED}(INCORRECT)${RESET}"
	fi
done

for testfile in test/codegen/linking/good*.bminor
do
	if bin/bminor --codegen $testfile $testfile.s &> /dev/null
	then
		if gcc -g $testfile.s src/library.c -o $testfile.exe
		then
			if chmod +x $testfile.exe && $testfile.exe &> $testfile.out
			then
				echo -e "$testfile success ${GREEN}(as expected)${RESET}"
			else
				echo -e "$testfile.exe failure ${RED}(INCORRECT)${RESET}"
			fi
		else
			echo -e "gcc failed to assemble $testfile ${RED}(INCORRECT)${RESET}"
		fi 
	else
		echo -e "$testfile couldn't complete codegen ${RED}(INCORRECT)${RESET}"
	fi
done