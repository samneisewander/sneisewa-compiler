#!/bin/bash

# Reset all formatting
RESET="\e[0m"

# Foreground colors
RED="\e[31m"
GREEN="\e[32m"

# Encoder
for testfile in test/encode/good*.bminor
do
	if ./bin/bminor --encode $testfile &> $testfile.out
	then
		echo -e "$testfile success ${GREEN}(as expected)${RESET}"
	else
		echo -e "$testfile failure ${RED}(INCORRECT)${RESET}"
	fi
done

for testfile in test/encode/bad*.bminor
do
	if ./bin/bminor --encode $testfile &> $testfile.out
	then
		echo -e "$testfile success ${RED}(INCORRECT)${RESET}"
	else
		echo -e "$testfile failure ${GREEN}(as expected)${RESET}"
	fi
done

# Scanner
for testfile in test/scanner/good*.bminor
do
	if ./bin/bminor --scan $testfile &> $testfile.out
	then
		echo -e "$testfile success ${GREEN}(as expected)${RESET}"
	else
		echo -e "$testfile failure ${RED}(INCORRECT)${RESET}"
	fi
done

for testfile in test/scanner/bad*.bminor
do
	if ./bin/bminor --scan $testfile &> $testfile.out
	then
		echo -e "$testfile success ${RED}(INCORRECT)${RESET}"
	else
		echo -e "$testfile failure ${GREEN}(as expected)${RESET}"
	fi
done

# Parser
for testfile in test/parser/good*.bminor
do
	if ./bin/bminor --parse $testfile &> $testfile.out
	then
		echo -e "$testfile success ${GREEN}(as expected)${RESET}"
	else
		echo -e "$testfile failure ${RED}(INCORRECT)${RESET}"
	fi
done

for testfile in test/parser/bad*.bminor
do
	if ./bin/bminor --parse $testfile &> $testfile.out
	then
		echo -e "$testfile success ${RED}(INCORRECT)${RESET}"
	else
		echo -e "$testfile failure ${GREEN}(as expected)${RESET}"
	fi
done

# Printer
for testfile in test/printer/*.bminor
do
	./bin/bminor --print $testfile &> $testfile.pretty
done

for testfile in test/printer/*.pretty
do
	if ./bin/bminor --print $testfile &> $testfile.out
	then
		if diff $testfile $testfile.out &> /dev/null
		then
			echo -e "$testfile success ${GREEN}(as expected)${RESET}"
		else
			echo -e "$testfile bad diff ${RED}(INCORRECT)${RESET}"
		fi
	else
		echo -e "$testfile parse failure ${RED}(INCORRECT)${RESET}"
	fi
done

# Resolver
for testfile in test/resolve/good*.bminor
do
	if ./bin/bminor --resolve $testfile &> $testfile.out
	then
		echo -e "$testfile success ${GREEN}(as expected)${RESET}"
	else
		echo -e "$testfile failure ${RED}(INCORRECT)${RESET}"
	fi
done

for testfile in test/resolve/bad*.bminor
do
	if ./bin/bminor --resolve $testfile &> $testfile.out
	then
		echo -e "$testfile success ${RED}(INCORRECT)${RESET}"
	else
		echo -e "$testfile failure ${GREEN}(as expected)${RESET}"
	fi
done

# Typechecker
for testfile in test/typechecker/good*.bminor
do
	if ./bin/bminor --typecheck $testfile &> $testfile.out
	then
		echo -e "$testfile success ${GREEN}(as expected)${RESET}"
	else
		echo -e "$testfile failure ${RED}(INCORRECT)${RESET}"
	fi
done

for testfile in test/typechecker/bad*.bminor
do
	if ./bin/bminor --typecheck $testfile &> $testfile.out
	then
		echo -e "$testfile success ${RED}(INCORRECT)${RESET}"
	else
		echo -e "$testfile failure ${GREEN}(as expected)${RESET}"
	fi
done