#!/bin/bash

for testfile in test/encode/good*.bminor
do
	if ./bin/bminor --encode $testfile &> $testfile.out
	then
		echo "$testfile success (as expected)"
	else
		echo "$testfile failure (INCORRECT)"
	fi
done

for testfile in test/encode/bad*.bminor
do
	if ./bin/bminor --encode $testfile &> $testfile.out
	then
		echo "$testfile success (INCORRECT)"
	else
		echo "$testfile failure (as expected)"
	fi
done

for testfile in test/scanner/good*.bminor
do
	if ./bin/bminor --scan $testfile &> $testfile.out
	then
		echo "$testfile success (as expected)"
	else
		echo "$testfile failure (INCORRECT)"
	fi
done

for testfile in test/scanner/bad*.bminor
do
	if ./bin/bminor --scan $testfile &> $testfile.out
	then
		echo "$testfile success (INCORRECT)"
	else
		echo "$testfile failure (as expected)"
	fi
done

for testfile in test/parser/good*.bminor
do
	if ./bin/bminor --parse $testfile &> $testfile.out
	then
		echo "$testfile success (as expected)"
	else
		echo "$testfile failure (INCORRECT)"
	fi
done

for testfile in test/parser/bad*.bminor
do
	if ./bin/bminor --parse $testfile &> $testfile.out
	then
		echo "$testfile success (INCORRECT)"
	else
		echo "$testfile failure (as expected)"
	fi
done

for testfile in test/printer/*.bminor
do
	if ./bin/bminor --print $testfile &> $testfile.out
	then
		echo "$testfile success (as expected)"
	else
		echo "$testfile failure (INCORRECT)"
	fi
done