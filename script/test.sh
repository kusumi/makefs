#!/bin/bash

if [ `whoami` != root ]; then
	echo "XXX Not root"
	exit 1
fi

if [[ `uname` = *BSD ]] || [ `uname` = "DragonFly" ]; then
	MAKE=gmake
else
	MAKE=make
fi

ARGS=$1
if [ "${ARGS}" = "" ]; then
	ARGS="/"
fi

f=./script/build_test.sh
echo "##### ${f}"
bash ${f} || exit 1

${MAKE} clean || exit 1
${MAKE} || exit 1

f=./script/image_creat_test.sh
echo "##### ${f}"
bash ${f} || exit 1

f=./script/image_creat_test.sh
for x in ${ARGS}; do
	echo "##### IMG_FILE_PATH=${x} ${f}"
	IMG_FILE_PATH=${x} bash ${f} || exit 1
done

f=./script/image_mount_test.sh
echo "##### ${f}"
bash ${f} || exit 1

echo "all success"
