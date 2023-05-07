#!/bin/bash

[ -d "${HOME}" ] || exit 1

# Arg 1
IMG_FILE=$1
if [ "${IMG_FILE}" = "" ]; then
	IMG_FILE=${HOME}/__makefs/img
	mkdir -p `dirname ${IMG_FILE}` || exit 1
fi

if [ -e ${IMG_FILE} ]; then
	rm ${IMG_FILE} || exit 1
fi

# Arg 2
SRC_DIR=$2
if [ "${SRC_DIR}" = "" ]; then
	SRC_DIR=${HOME}/__makefs/src
	mkdir -p `dirname ${SRC_DIR}` || exit 1
fi

if [ ! -d ${SRC_DIR} ]; then
	echo "No such directory ${SRC_DIR}"
	exit 1
fi

if [ "${MAKEFS}" = "" ]; then
	MAKEFS=./src/makefs
fi
if [ ! -x ${MAKEFS} ]; then
	echo "No such exeutable ${MAKEFS}"
	exit 1
fi

# helper functions
run_fsck() {
	BIN=$1
	OPT=$2
	FILE=$3
	which ${BIN} >/dev/null 2>&1
	if [ $? -eq 0 ]; then
		${BIN} ${OPT} ${FILE}
	fi
}

# 4.4BSD FFS
echo "### 4.4BSD FFS"
${MAKEFS} -Z -t ffs -o version=1 ${IMG_FILE} ${SRC_DIR} || exit 1
file ${IMG_FILE} || exit 1
rm ${IMG_FILE} || exit 1
echo

# FreeBSD UFS2
echo "### FreeBSD UFS2"
${MAKEFS} -Z -t ffs -o version=2 ${IMG_FILE} ${SRC_DIR} || exit 1
file ${IMG_FILE} || exit 1
rm ${IMG_FILE} || exit 1
echo

# ISO9660
echo "### ISO9660"
${MAKEFS} -Z -t cd9660 ${IMG_FILE} ${SRC_DIR} || exit 1
file ${IMG_FILE} || exit 1
rm ${IMG_FILE} || exit 1
echo

# FAT32
echo "### FAT32"
${MAKEFS} -Z -t msdos -s 4g ${IMG_FILE} ${SRC_DIR} || exit 1
file ${IMG_FILE} || exit 1
rm ${IMG_FILE} || exit 1
echo

# exFAT
echo "### exFAT"
${MAKEFS} -Z -t exfat -s 4g ${IMG_FILE} ${SRC_DIR} || exit 1
file ${IMG_FILE} || exit 1
rm ${IMG_FILE} || exit 1
echo

# HAMMER2
echo "### HAMMER2"
${MAKEFS} -Z -t hammer2 ${IMG_FILE} ${SRC_DIR} || exit 1
file ${IMG_FILE} || exit 1
run_fsck fsck_hammer2 "" ${IMG_FILE}
rm ${IMG_FILE} || exit 1
echo

echo "success"
