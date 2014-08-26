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
		echo "$(tput setaf 1)[FAILED]$(tput sgr0) gt $1 -- gt failed";
	elif ! diff -w <(tail -n+2 test.out) <(echo "$2") &> /dev/null; then
		ERROR_COUNT=$((ERROR_COUNT + 1));
		echo "$(tput setaf 1)[FAILED]$(tput sgr0) gt $1 -- unexpected output";
		echo "Expected:";
		echo "$2";
		echo "Got: ";
		tail -n+2 test.out;
	else
		SUCCESS_COUNT=$((SUCCESS_COUNT + 1));
		echo "$(tput setaf 2)[OK]$(tput sgr0) gt $1 -- succeed";
	fi
}

function expect_failure {
	if $GT $1 &> /dev/null; then
		ERROR_COUNT=$((ERROR_COUNT + 1));
		echo "$(tput setaf 1)[FAILED]$(tput sgr0) gt $1 -- expected failure";
	else
		SUCCESS_COUNT=$((SUCCESS_COUNT + 1));
		echo "$(tput setaf 2)[OK]$(tput sgr0) gt $1 -- failed as expected";
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

expect_success "create gadget1" "name=gadget1, force=0";
expect_success "create -f gadget2" "name=gadget2, force=1";
expect_success "create -f gadget2 attr=val" "name=gadget2, force=1, attr=val";
expect_success "create --force gadget3 attr1=val1 attr2=val2"\
	"name=gadget3, force=1, attr1=val1, attr2=val2";
expect_success "rm gadget1" "name=gadget1, force=0, recursive=0";
expect_success "rm -r gadget2" "name=gadget2, force=0, recursive=1";
expect_success "rm -f gadget3" "name=gadget3, force=1, recursive=0";
expect_success "rm -rf gadget4" "name=gadget4, force=1, recursive=1";

expect_failure "create gadget1 --verbose";
expect_failure "create gadget1 and more";
expect_failure "create";
expect_failure "rm -rf gadget gadget";
expect_failure "rm";

expect_success "get gadget1" "name=gadget1, attrs=";
expect_success "get gadget1 attr" "name=gadget1, attrs=attr,";
expect_success "get gadget2 attr1 attr2 attr3 attr4" \
	"name=gadget2, attrs=attr1, attr2, attr3, attr4,";
expect_success "set gadget attr=val" "name=gadget, attr=val";
expect_success "set gadget attr1=val1 attr2=val2"\
	"name=gadget, attr1=val1, attr2=val2";

expect_failure "get";
expect_failure "set gadget";
expect_failure "set gadget attr equals val";
expect_failure "set";

expect_success "enable" "";
expect_success "enable --gadget=gadget --udc=udc" "gadget=gadget, udc=udc";
expect_success "enable --gadget=gadget1" "gadget=gadget1,";
expect_success "enable --udc=udc1" "udc=udc1";
expect_success "disable" "";
expect_success "disable gadget1" "gadget=gadget1,";
expect_success "disable --udc=udc1" "udc=udc1";

expect_failure "disable gadget1 --udc=udc";
expect_failure "enable -f";
expect_failure "enable -v";
expect_failure "enable -r";
expect_failure "enable -o";
expect_failure "disable gadget1 sth";
expect_failure "disable gadget1 --gadget=gadget2";
expect_failure "disable gadget1 -f";
expect_failure "disable gadget1 -v";
expect_failure "disable gadget1 -r";
expect_failure "disable gadget1 -o";

expect_success "gadget gadget2" "name=gadget2, recursive=0, verbose=0";
expect_success "gadget gadget1 -v" "name=gadget1, recursive=0, verbose=1";
expect_success "gadget gadget3 -r" "name=gadget3, recursive=1, verbose=0";
expect_success "gadget gadget4 -vr" "name=gadget4, recursive=1, verbose=1";

expect_failure "gadget gadget -f";
expect_failure "gadget gadget gadget";

expect_success "load name gadget1" "name=name, gadget=gadget1, off=0, stdin=0";
expect_success "load name gadget1 --off" "name=name, gadget=gadget1, off=1, stdin=0";
expect_success "load name --stdin" "gadget=name, off=0, stdin=1";
expect_success "load name gadget1 --path=path"\
	"name=name, gadget=gadget1, path=path, off=0, stdin=0";
expect_success "save gadget1 name" "gadget=gadget1, name=name, force=0, stdout=0";
expect_success "save gadget1 --file=file"\
	"gadget=gadget1, file=file, force=0, stdout=0";
expect_success "save gadget1 --stdout attr=val"\
	"gadget=gadget1, force=0, stdout=1, attr=val";
expect_success "save gadget1 name --path=path"\
	"gadget=gadget1, name=name, path=path, force=0, stdout=0";
expect_success "save gadget1 name -f"\
	"gadget=gadget1, name=name, force=1, stdout=0";

expect_failure "load name --file=file1 --stdin";
expect_failure "load name --stdin --path=path1";
expect_failure "load name --path=path1 --file=file1";
expect_failure "load name --path";
expect_failure "load name --file";
expect_failure "load";
expect_failure "save gadget --file=file1 --stdin";
expect_failure "save gadget --stdin --path=path1";
expect_failure "save gadget --path=path1 --file=file1";
expect_failure "save gadget --path";
expect_failure "save gadget --file";
expect_failure "save";

expect_success "template name" "name=name, verbose=0, recursive=0";
expect_success "template" "verbose=0, recursive=0";
expect_success "template --verbose --recursive" "verbose=1, recursive=1";
expect_success "template --verbose name" "name=name, verbose=1, recursive=0";
expect_success "template --recursive name" "name=name, verbose=0, recursive=1";

expect_failure "template -f";
expect_failure "template -o";
expect_failure "template name1 name2";

expect_success "template get name" "name=name, attr=";
expect_success "template get name attr" "name=name, attr=attr,";
expect_success "template set name attr=val" "name=name, attr=val";
expect_success "template rm name" "name=name";

expect_failure "template get"
expect_failure "template set name"
expect_failure "template set name attr"
expect_failure "template set name attr1=val1 attr2"
expect_failure "template rm"
expect_failure "template rm name1 name2"

expect_success "config create gadget1 config1"\
	"gadget=gadget1, config=config1, force=0";
expect_success "config create gadget1 config1 attr=val"\
	"gadget=gadget1, config=config1, force=0, attr=val";
expect_success "config create -f gadget1 config1"\
	"gadget=gadget1, config=config1, force=1";
expect_success "config rm gadget config"\
	"gadget=gadget, config=config, force=0, recursive=0";
expect_success "config rm -rf gadget config"\
	"gadget=gadget, config=config, force=1, recursive=1";

expect_failure "config create -r gadget1 config1";
expect_failure "config create -f gadget1";
expect_failure "config rm gadget";
expect_failure "config rm -rfv gadget config";
expect_failure "config rm gadget config function";

echo "Testing finished, $SUCCESS_COUNT tests passed, $ERROR_COUNT failed.";
