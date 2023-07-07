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
	else
		echo "${BIN} not found"
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
echo
${MAKEFS} -t hammer2 -o B ${IMG_FILE} __ || exit 1
echo
${MAKEFS} -t hammer2 -o B ${IMG_FILE} __ || exit 1
echo
${MAKEFS} -t hammer2 -o B ${IMG_FILE} __ || exit 1
echo
${MAKEFS} -t hammer2 -o G ${IMG_FILE} __ || exit 1
echo
${MAKEFS} -t hammer2 -o P=lookup:DATA ${IMG_FILE} __ || exit 1
echo
${MAKEFS} -t hammer2 -o P=create:pfs ${IMG_FILE} __ || exit 1
echo
${MAKEFS} -t hammer2 -o P=snapshot ${IMG_FILE} __ || exit 1
echo
${MAKEFS} -t hammer2 -o P=snapshot:LOCAL_snapshot -o m=LOCAL ${IMG_FILE} __ || exit 1
echo
${MAKEFS} -t hammer2 -o P=get ${IMG_FILE} __ || exit 1
echo
${MAKEFS} -t hammer2 -o P=delete:pfs ${IMG_FILE} __ || exit 1
echo
${MAKEFS} -t hammer2 -o P=delete:LOCAL_snapshot ${IMG_FILE} __ || exit 1
echo
${MAKEFS} -t hammer2 -o P=get ${IMG_FILE} __ || exit 1
echo
if [ "${IMG_FILE_PATH}" != "" ]; then
	${MAKEFS} -t hammer2 -o I=get:${IMG_FILE_PATH} ${IMG_FILE} __ || exit 1
	echo
	${MAKEFS} -t hammer2 -o I=get://${IMG_FILE_PATH}// ${IMG_FILE} __ || exit 1
	echo
	${MAKEFS} -t hammer2 -o I=setcheck:${IMG_FILE_PATH}:sha192 ${IMG_FILE} __ || exit 1
	echo
	${MAKEFS} -t hammer2 -o I=setcheck://${IMG_FILE_PATH}//:sha192 ${IMG_FILE} __ || exit 1
	echo
	${MAKEFS} -t hammer2 -o I=setcomp:${IMG_FILE_PATH}:zlib:9 ${IMG_FILE} __ || exit 1
	echo
	${MAKEFS} -t hammer2 -o I=setcomp://${IMG_FILE_PATH}//:zlib:9 ${IMG_FILE} __ || exit 1
	echo

	OUT_DIR=${HOME}/__makefs/out
	if [ -d ${OUT_DIR} ]; then
		TMPSTR=`mktemp`
		mv ${OUT_DIR} ${OUT_DIR}.`basename ${TMPSTR}`
	fi
	mkdir -p ${OUT_DIR} || exit 1
	${MAKEFS} -t hammer2 -o R=${IMG_FILE_PATH} ${IMG_FILE} ${OUT_DIR} || exit 1
	F1=${SRC_DIR}/${IMG_FILE_PATH}
	F2=${OUT_DIR}/`basename ${IMG_FILE_PATH}`
	stat ${F1} || exit 1
	stat ${F2} || exit 1
	if [ -f ${F2} ]; then
		echo "compare files"
		diff ${F1} ${F2} || exit 1
		cmp ${F1} ${F2} || exit 1
	elif [ -d ${F2} ]; then
		echo "compare directories"
		diff -aNur ${F1} ${F2} >/dev/null || exit 1
	else
		echo "${F2} is neither file nor directory"
		exit 1
	fi
	echo

	if [ ${IMG_FILE_PATH} != "/" ]; then # can't destroy /
		${MAKEFS} -t hammer2 -o D=/${IMG_FILE_PATH} ${IMG_FILE} __ || exit 1 # need leading /
		echo
		${MAKEFS} -t hammer2 -o D=//${IMG_FILE_PATH}// ${IMG_FILE} __ && exit 1 # need leading /
		echo
		${MAKEFS} -t hammer2 -o I=get:${IMG_FILE_PATH} ${IMG_FILE} __ && exit 1
		echo
		${MAKEFS} -t hammer2 -o I=get://${IMG_FILE_PATH}// ${IMG_FILE} __ && exit 1
		echo
	fi
fi
run_fsck fsck_hammer2 "" ${IMG_FILE}
rm ${IMG_FILE} || exit 1
echo

echo "success"
