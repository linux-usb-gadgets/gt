#!/bin/bash


CHECK_MEM=0;
MEMCHECK_ERROR=2;
VALGRIND="valgrind -q --error-exitcode=$MEMCHECK_ERROR";

function usage {
	echo "Usage: $0 [-hv] [executable]";
	echo "Options:";
	echo "-h Print this help";
	echo "-v Look for memory leaks using valgrind memcheck";
}

while getopts vh opt; do
	case $opt in
	v)
		CHECK_MEM=1;
		;;
	h)
		usage;
		exit 0;
		;;
	?)
		usage;
		exit 1;
		;;
	esac
done

shift $(( OPTIND - 1));


if [ "$1" ]
then
	if  which "$1" &> /dev/null;
	then
		GT="$1";
	else
		echo "$1: Executable file not found. Aborted.";
		exit 1;
	fi
elif [ -f "gt-parse-test" ]
then
	GT="./gt-parse-test";
else
	echo "Executable file not specified. Aborted."
	exit 1;
fi

if [ $CHECK_MEM -ne 0 ]
then
	EXECUTABLE="$VALGRIND $GT";
else
	EXECUTABLE=$GT;
fi

ERROR_COUNT=0;
SUCCESS_COUNT=0;

function print_failure {
	echo "$(tput setaf 1)[FAILED]$(tput sgr0) $1";
}

function print_success {
	echo "$(tput setaf 2)[OK]$(tput sgr0) $1";
}

function expect_success {
	$EXECUTABLE $1 > test.out 2>/dev/null;
	ret=$?;
	if [[ $res -eq $MEMCHECK_ERROR ]]
	then
		ERROR_COUNT=$((ERROR_COUNT + 1));
		print_failure "gt $1 -- memcheck error";
	elif [[ $res -ne 0 ]] ; then
		ERROR_COUNT=$((ERROR_COUNT + 1));
		print_failure " gt $1 -- gt failed";
	elif ! diff -w <(tail -n+2 test.out) <(echo "$2") &> /dev/null; then
		ERROR_COUNT=$((ERROR_COUNT + 1));
		print_failure "gt $1 -- unexpected output";
		echo "Expected:";
		echo "$2";
		echo "Got: ";
		tail -n+2 test.out;
	else
		SUCCESS_COUNT=$((SUCCESS_COUNT + 1));
		print_success "gt $1 -- succeed";
	fi
}

function expect_failure {
	$EXECUTABLE $1 &> /dev/null;
	ret=$?;
	if [[ $ret -eq $MEMCHECK_ERROR ]]
	then
		ERROR_COUNT=$((ERROR_COUNT + 1));
		print_failure "gt $1 -- memcheck error";
	elif [[ $ret -eq 0 ]]
	then
		ERROR_COUNT=$((ERROR_COUNT + 1));
		print_failure "gt $1 -- expected failure";
	else
		SUCCESS_COUNT=$((SUCCESS_COUNT + 1));
		print_success "gt $1 -- failed as expected";
	fi
}

expect_success "settings set default-udc=udc1" "default-udc=udc1, ";
expect_success "settings set default-udc=udc1 default-template-path=path1"\
	"default-udc=udc1, default-template-path=path1, ";
expect_success "settings get configfs-path" "configfs-path, ";
expect_success "settings get configfs-path default-udc"\
	"configfs-path, default-udc,";
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
expect_success "create -f gadget2 idVendor=1" "name=gadget2, force=1, idVendor=1";
expect_success "create --force gadget3 idVendor=1 idProduct=2"\
	"name=gadget3, force=1, idVendor=1, idProduct=2";
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
expect_success "get gadget1 idVendor" "name=gadget1, attrs=idVendor,";
expect_success "get gadget2 idVendor idProduct" \
	"name=gadget2, attrs=idVendor, idProduct,";
expect_success "set gadget idVendor=1" "name=gadget, idVendor=1";
expect_success "set gadget idVendor=1 idProduct=2"\
	"name=gadget, idVendor=1, idProduct=2";

expect_failure "get";
expect_failure "set gadget";
expect_failure "set gadget attr equals val";
expect_failure "set";

expect_success "enable gadget udc" "gadget=gadget, udc=udc";
expect_success "enable gadget1" "gadget=gadget1,";
expect_success "disable" "";
expect_success "disable gadget1" "gadget=gadget1,";
expect_success "disable --udc=udc1" "udc=udc1";

expect_failure "enable";
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
	"gadget=gadget1, name=gadget1, file=file, force=0, stdout=0";
expect_success "save gadget1 --stdout attr=val"\
	"gadget=gadget1, name=gadget1, force=0, stdout=1, attr=val";
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

expect_success "config create gadget1 config 1"\
	"gadget=gadget1, cfg_label=config, cfg_id=1, force=0";
expect_success "config create gadget1 config 1 attr=val"\
	"gadget=gadget1, cfg_label=config, cfg_id=1, force=0, attr=val";
expect_success "config create -f gadget1 config 1"\
	"gadget=gadget1, cfg_label=config, cfg_id=1, force=1";
expect_success "config rm gadget config 1"\
	"gadget=gadget, config_name=config, config_id=1, force=0, recursive=0";
expect_success "config rm -rf gadget config 1"\
	"gadget=gadget, config_name=config, config_id=1, force=1, recursive=1";

expect_failure "config create -r gadget1 config1";
expect_failure "config create -f gadget1";
expect_failure "config rm gadget";
expect_failure "config rm -rfv gadget config";
expect_failure "config rm gadget config function";

expect_success "config get gadget config"\
	"gadget=gadget, config=config, attrs=";
expect_success "config get gadget config attr1 attr2"\
	"gadget=gadget, config=config, attrs=attr1,attr2,";
expect_success "config set gadget config attr=val"\
	"gadget=gadget, config=config, attr=val";
expect_success "config set gadget config attr1=val1 attr2=val2"\
	"gadget=gadget, config=config, attr1=val1, attr2=val2";

expect_failure "config get gadget";
expect_failure "config set gadget1";
expect_failure "config set gadget1 config1 func1";

expect_success "config gadget1 config1"\
	"gadget=gadget1, config_label=config1, verbose=0, recursive=0";
expect_success "config gadget1" \
	"gadget=gadget1, verbose=0, recursive=0";
expect_success "config gadget1 -v"\
	"gadget=gadget1, verbose=1, recursive=0";
expect_success "config -r gadget1 config 1"\
	"gadget=gadget1, config_label=config, config_id=1, verbose=0, recursive=1";

expect_failure "config gadget1 config1 func1";
expect_failure "config gadget1 -f";

expect_success "config add gadget1 conf 1 acm name"\
	"gadget=gadget1, cfg_label=conf, cfg_id=1, type=acm, instance=name";
expect_success "config del gadget1 conf 1 acm name"\
	"gadget=gadget1, cfg_label=conf, cfg_id=1, type=acm, instance=name";

expect_failure "config add gadget1 conf1";
expect_failure "config add gadget1";
expect_failure "config add gadget1 conf1 func";
expect_failure "config add";
expect_failure "config add gadget1 conf acm name more";
expect_failure "config del gadget1 conf1";
expect_failure "config del gadget1";
expect_failure "config del gadget1 conf1 func";
expect_failure "config del";
expect_failure "config del gadget1 conf acm name more";

expect_success "config template"\
	"verbose=0, recursive=0";
expect_success "config template name"\
	"name=name, verbose=0, recursive=0";
expect_success "config template get name"\
	"name=name, attr=";
expect_success "config template set name attr=val"\
	"name=name, attr=val";
expect_success "config template rm name"\
	"name=name";

expect_failure "config template name1 name2";
expect_failure "config template get";
expect_failure "config template --force";
expect_failure "config template -f name";
expect_failure "config template set name";
expect_failure "config tempalte set name attr";
expect_failure "config template rm name1 name2";
expect_failure "config template set";

expect_success "config load name1 gadget1 config1"\
	"name=name1, gadget=gadget1, config=config1, recursive=0, force=0, stdin=0";
expect_success "config load name gadget"\
	"name=name, gadget=gadget, recursive=0, force=0, stdin=0";
expect_success "config load name gadget -f"\
	"name=name, gadget=gadget, recursive=0, force=1, stdin=0";
expect_success "config load name1 gadget1 --stdin"\
	"gadget=gadget1, config=name1, recursive=0, force=0, stdin=1";
expect_success "config load name1 gadget1 --file=file1"\
	"gadget=gadget1, config=name1, file=file1, recursive=0, force=0, stdin=0";
expect_success "config load -r name1 gadget1 config1"\
	"name=name1, gadget=gadget1, config=config1, recursive=1, force=0, stdin=0";

expect_failure "config load";
expect_failure "config load name1";
expect_failure "config load name1 gadget1 config1 -v";
expect_failure "config load name1 gadget1 config1 --stdin --path=path1";
expect_failure "config load name gadget config1 --stdin";
expect_failure "config load name gadget conf --file=file1";
expect_failure "config load name gadget --path=p --file=f";
expect_failure "config load name gadget conf --stdin";

expect_success "config save gadget1 conf name"\
	"gadget=gadget1, config=conf, name=name, force=0, stdout=0";
expect_success "config save gadget1 conf name attr=val"\
	"gadget=gadget1, config=conf, name=name, force=0, stdout=0, attr=val";
expect_success "config save gadget1 conf name attr1=val1 attr2=val2"\
	"gadget=gadget1, config=conf, name=name, force=0, stdout=0, attr1=val1, attr2=val2";
expect_success "config save gadget1 conf name -f"\
	"gadget=gadget1, config=conf, name=name, force=1, stdout=0";
expect_success "config save gadget1 conf --stdout"\
	"gadget=gadget1, config=conf, force=0, stdout=1";
expect_success "config save gadget1 conf"\
	"gadget=gadget1, config=conf, force=0, stdout=0";
expect_success "config save gadget1 conf --file=file1"\
	"gadget=gadget1, config=conf, file=file1, force=0, stdout=0";
expect_success "config save gadget1 conf name --path=path1"\
	"gadget=gadget1, config=conf, name=name, path=path1, force=0, stdout=0";

expect_failure "config save gadget1 conf name --stdout";
expect_failure "config save";
expect_failure "config save gadget1 conf --stdout --file=file";
expect_failure "config save gadget1 conf --file=file --path=p";
expect_failure "config save gadget1 conf --path=p --stdout";
expect_failure "config save gadget1 conf --path";
expect_failure "config save gadget1 conf --file";

expect_success "func create gadget acm name"\
	"gadget=gadget, type=acm, name=name, force=0";
expect_success "func create gadget acm name"\
	"gadget=gadget, type=acm, name=name, force=0";
expect_success "func create -f gadget acm name"\
	"gadget=gadget, type=acm, name=name, force=1";
expect_success "func create -f gadget acm name"\
	"gadget=gadget, type=acm, name=name, force=1";
expect_success "func create gadget acm name1 attr=val"\
	"gadget=gadget, type=acm, name=name1, force=0, attr=val";
expect_success "func create gadget acm name1 attr1=val1 attr2=val2"\
	"gadget=gadget, type=acm, name=name1, force=0, attr1=val1, attr2=val2";

expect_failure "func create gadget acm name -v";
expect_failure "func create gadget acm name -r";
expect_failure "func create gadget";
expect_failure "func create gadget acm name attrval";
expect_failure "func create gadget acmname";

expect_success "func rm gadget acm name"\
	"gadget=gadget, type=acm, instance=name, recursive=0, force=0";
expect_success "func rm gadget1 acm name"\
	"gadget=gadget1, type=acm, instance=name, recursive=0, force=0";
expect_success "func rm -r gadget1 acm name1"\
	"gadget=gadget1, type=acm, instance=name1, recursive=1, force=0";
expect_success "func rm -f gadget2 acm name2"\
	"gadget=gadget2, type=acm, instance=name2, recursive=0, force=1";
expect_success "func rm -rf gadget3 acm name3"\
	"gadget=gadget3, type=acm, instance=name3, recursive=1, force=1";

expect_failure "func rm";
expect_failure "func rm gadget1";
expect_failure "func rm gadget1 function";
expect_failure "func rm gadget1 acm name attr=val";
expect_failure "func rm gadget1 -v acm name";
expect_failure "func rm gadget1 -o acm name";

expect_success "func get gadget1 acm name1"\
	"gadget=gadget1, type=acm, name=name1, attrs=";
expect_success "func get gadget1 acm name1 attr1 attr2"\
	"gadget=gadget1, type=acm, name=name1, attrs=attr1,attr2,";
expect_success "func get gadget1 acm name1 attr"\
	"gadget=gadget1, type=acm, name=name1, attrs=attr,";

expect_failure "func get gadget1";
expect_failure "func get";
expect_failure "func get gadget function";

expect_success "func set gadget1 acm name1"\
	"gadget=gadget1, type=acm, name=name1";
expect_success "func set gadget1 acm name1"\
	"gadget=gadget1, type=acm, name=name1";
expect_success "func set gadget1 acm name1 attr1=val1 attr2=val2"\
	"gadget=gadget1, type=acm, name=name1, attr1=val1, attr2=val2";
expect_success "func set gadget1 acm name1 attr=val"\
	"gadget=gadget1, type=acm, name=name1, attr=val";

expect_failure "func set";
expect_failure "func set gadget1";
expect_failure "func set gadget1 function":
expect_failure "func set gadget1 acm name attr";

expect_success "func show gadget1" "gadget=gadget1, verbose=0";
expect_success "func show gadget1 acm name"\
	"gadget=gadget1, type=acm, instance=name, verbose=0";
expect_success "func show -v gadget1" "gadget=gadget1, verbose=1";
expect_success "func --verbose gadget1 acm name"\
	"gadget=gadget1, type=acm, instance=name, verbose=1";

expect_failure "func gadget2 -f";
expect_failure "func gadget3 -r";
expect_failure "func gadget4 -o";
expect_failure "func gadget5 func func func";

expect_success "func load name1 gadget1 func1"\
	"name=name1, gadget=gadget1, func=func1, force=0, stdin=0";
expect_success "func load name1 gadget1 func1 -f"\
	"name=name1, gadget=gadget1, func=func1, force=1, stdin=0";
expect_success "func load name1 gadget1 --stdin"\
	"gadget=gadget1, func=name1, force=0, stdin=1";
expect_success "func load name1 gadget1 --file=file1"\
	"gadget=gadget1, func=name1, file=file1, force=0, stdin=0";

expect_failure "func load name gadget --file";
expect_failure "func load name gadget --path";
expect_failure "func load name1 gadget1 --stdin --file=f";
expect_failure "func load name1 gadget1 --file=f --path=p";
expect_failure "func load name2 gadget2 --path=p --stdin";
expect_failure "func load name1 gadget1 --stdin --file=f --path=p";
expect_failure "func load name1 gadget1 func1 --stdin";

expect_success "func save gadget1 function1 name1"\
	"gadget=gadget1, func=function1, name=name1, force=0, stdout=0";
expect_success "func save gadget1 func1 name1 attr1=val1 attr2=val2"\
	"gadget=gadget1, func=func1, name=name1, force=0, stdout=0, attr1=val1, attr2=val2";
expect_success "func save gadget1 func1 attr1=val1"\
	"gadget=gadget1, func=func1, force=0, stdout=0, attr1=val1";
expect_success "func save gadget1 func1 --stdout"\
	"gadget=gadget1, func=func1, force=0, stdout=1";
expect_success "func save gadget1 func1 -f"\
	"gadget=gadget1, func=func1, force=1, stdout=0";
expect_success "func save gadget1 func1 name1 -f attr1=val1 attr2=val2"\
	"gadget=gadget1, func=func1, name=name1, force=1, stdout=0, attr1=val1, attr2=val2";
expect_success "func save gadget1 func1 --file=f attr1=val1 attr2=val2"\
	"gadget=gadget1, func=func1, file=f, force=0, stdout=0, attr1=val1, attr2=val2";
expect_success "func save gad1 fun1 nam1 --path=p --force"\
	"gadget=gad1, func=fun1, name=nam1, path=p, force=1, stdout=0";

expect_failure "func save";
expect_failure "func save gadget1";
expect_failure "func save gadget1 func1 name1 --file=f";
expect_failure "func save gadget1 func1 name1 --stdout";
expect_failure "func save gadget1 fucn1 --path=p --file=f";

expect_success "func template" "verbose=0";
expect_success "func template name1" "name=name1, verbose=0";
expect_success "func template -v" "verbose=1";
expect_success "func template -v name1" "name=name1, verbose=1";

expect_failure "func template name1 name2";
expect_failure "func template -r";
expect_failure "func template -f";

expect_success "func template get name1" "name=name1, attrs=";
expect_success "func template get name2 attr" "name=name2, attrs=attr,";
expect_success "func template get name3 attr1 attr2 attr3"\
	"name=name3, attrs=attr1, attr2, attr3,";

expect_failure "func template get";

expect_success "func template set name1 attr1=val1"\
	"name=name1, attr1=val1";
expect_success "func template set name2 attr1=val1 attr2=val2 attr3=val3"\
	"name=name2, attr1=val1, attr2=val2, attr3=val3";

expect_failure "func template set";
expect_failure "func template set name1";
expect_failure "func template set name1 attr";
expect_failure "func template set name1 attr1=val1 attr2val2";

expect_success "func template rm name1" "name=name1";

expect_failure "func template rm";
expect_failure "func template rm name1 name2";

echo "Testing finished, $SUCCESS_COUNT tests passed, $ERROR_COUNT failed.";
