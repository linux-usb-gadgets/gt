#!/bin/bash

if [ "$1" ]
then
	if [ -f "$1" ]
	then
		GT="$1";
	elif [ which $1 &> /dev/null ]
	then
		GT="$1";
	else
		echo "Executable file not found. Aborted.";
		exit 1;
	fi
elif [ -f "gt" ]
then
	GT="./gt";
else
	echo "Executable file not specified. Aborted."
	exit 1;
fi

ERROR_COUNT=0;
SUCCESS_COUNT=0;

function expect_success {
	if ! $GT $1 > test.out; then
		ERROR_COUNT=$((ERROR_COUNT + 1));
		echo "[FAILED] gt $1 -- gt failed";
	elif ! diff -w <(tail -n+2 test.out) <(echo "$2") &> /dev/null; then
		ERROR_COUNT=$((ERROR_COUNT + 1));
		echo "[FAILED] gt $1 -- unexpected output";
		echo "Expected:";
		echo "$2";
		echo "Got: ";
		tail -n+2 test.out;
	else
		SUCCESS_COUNT=$((SUCCESS_COUNT + 1));
		echo "[OK] gt $1 -- succeed";
	fi
}

function expect_failure {
	if $GT $1 &> /dev/null; then
		ERROR_COUNT=$((ERROR_COUNT + 1));
		echo "[FAILED] gt $1 -- expected failure";
	else
		SUCCESS_COUNT=$((SUCCESS_COUNT + 1));
		echo "[OK] gt $1 -- failed as expected";
	fi

}

expect_success "settings set default-udc=udc1" "default-udc=udc1, ";
expect_success "settings set default-udc=udc1 default-template-path=path1"\
	"default-udc=udc1, default-template-path=path1, ";
expect_success "settings get config-fs-path" "config-fs-path, ";
expect_success "settings get config-fs-path default-udc"\
	"config-fs-path, default-udc,";
expect_success "settings get" "";
expect_success "settings append lookup-path value" "var=lookup-path, val=value";
expect_success "settings detach lookup-path value" "var=lookup-path, val=value";

expect_failure "settings set badopt=val";
expect_failure "settings get badopt";
expect_failure "settings append badopt badval";
expect_failure "settings append lookup-path too many";
expect_failure "settings append lookup-path";
expect_failure "settings detach badopt badval";
expect_failure "settings detach lookup-path and more";
expect_failure "settings detach lookup-path";

echo "Testing finished, $SUCCESS_COUNT tests passed, $ERROR_COUNT failed.";
