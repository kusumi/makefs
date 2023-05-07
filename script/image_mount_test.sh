#!/bin/bash

if [ `whoami` != root ]; then
	echo "XXX Not root"
	exit 1
fi

UNAME=`uname` || exit 1
if [[ ${UNAME} != Linux ]] && [[ ${UNAME} != FreeBSD ]] && [[ ${UNAME} != DragonFly ]]; then
	echo "XXX ${UNAME}" # XXX
	exit 1
fi

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

# Arg 3
MNT_DIR=$3
if [ "${MNT_DIR}" = "" ]; then
	MNT_DIR=/mnt/__makefs
	mkdir -p ${MNT_DIR} || exit 1
fi

# helper functions
run_dirhash() {
	DIR=$1
	if [ -x "${DIRHASH}" ]; then
		${DIRHASH} -squash -verbose ${DIR} || exit 1
	fi
}

unmount() {
	DIR=$1
	mount | grep ${DIR}
	if [ $? -eq 0 ]; then
		run_dirhash ${DIR}
		umount ${DIR} || exit 1
	fi
}

unlink() {
	FILE=$1
	if [ -f ${FILE} ]; then
		rm ${FILE} || exit 1
	fi
}

freebsd_init_mdconfig() {
	FILE=$1
	mdconfig ${FILE} || exit 1
}

freebsd_cleanup_mdconfig() {
	NAME=$1
	mdconfig -l | grep ${NAME} >/dev/null || exit 1
	if [ $? -eq 0 ]; then
		mdconfig -d -u ${NAME} || exit 1
	fi
}

dragonfly_init_vnconfig() {
	NAME=$1
	FILE=$2
	vnconfig ${NAME} ${FILE} || exit 1
}

dragonfly_cleanup_vnconfig() {
	NAME=$1
	vnconfig -l | grep "${NAME}: covering" >/dev/null || exit 1
	if [ $? -eq 0 ]; then
		vnconfig -u ${NAME} || exit 1
	fi
}

atexit() {
	if [ ${UNAME} = Linux ]; then
		unmount ${MNT_DIR}
	elif [ ${UNAME} = FreeBSD ]; then
		unmount ${MNT_DIR}
		freebsd_cleanup_mdconfig ${MDNAME}
	elif [ ${UNAME} = DragonFly ]; then
		unmount ${MNT_DIR}
		dragonfly_cleanup_vnconfig ${VNNAME}
	fi
	unlink ${IMG_FILE}
}
trap atexit EXIT

if [ "${MDNAME}" = "" ]; then
	MDNAME=md0
fi

if [ "${VNNAME}" = "" ]; then
	VNNAME=vn0
fi

if [ "${MAKEFS}" = "" ]; then
	MAKEFS=./src/makefs
fi
if [ ! -x ${MAKEFS} ]; then
	echo "No such exeutable ${MAKEFS}"
	exit 1
fi

# 4.4BSD FFS
echo "### 4.4BSD FFS"
${MAKEFS} -Z -t ffs -o version=1 ${IMG_FILE} ${SRC_DIR} || exit 1
file ${IMG_FILE} || exit 1
if [ ${UNAME} = Linux ]; then
	mount -o ufstype=44bsd ${IMG_FILE} ${MNT_DIR} || exit 1
	unmount ${MNT_DIR}
elif [ ${UNAME} = FreeBSD ]; then
	MDNAME=`freebsd_init_mdconfig ${IMG_FILE}`
	mount /dev/${MDNAME} ${MNT_DIR} || exit 1
	unmount ${MNT_DIR}
	freebsd_cleanup_mdconfig ${MDNAME}
elif [ ${UNAME} = DragonFly ]; then
	dragonfly_init_vnconfig ${VNNAME} ${IMG_FILE}
	mount /dev/${VNNAME} ${MNT_DIR} || exit 1
	unmount ${MNT_DIR}
	dragonfly_cleanup_vnconfig ${VNNAME}
else
	echo "### Ignore 4.4BSD FFS mount on ${UNAME}"
fi
unlink ${IMG_FILE}
echo

# FreeBSD UFS2
echo "### FreeBSD UFS2"
${MAKEFS} -Z -t ffs -o version=2 ${IMG_FILE} ${SRC_DIR} || exit 1
file ${IMG_FILE} || exit 1
if [ ${UNAME} = Linux ]; then
	mount -o ufstype=ufs2 ${IMG_FILE} ${MNT_DIR} || exit 1
	unmount ${MNT_DIR}
elif [ ${UNAME} = FreeBSD ]; then
	MDNAME=`freebsd_init_mdconfig ${IMG_FILE}`
	mount /dev/${MDNAME} ${MNT_DIR} || exit 1
	unmount ${MNT_DIR}
	freebsd_cleanup_mdconfig ${MDNAME}
else
	echo "### Ignore FreeBSD UFS2 mount on ${UNAME}"
fi
unlink ${IMG_FILE}
echo

# ISO9660
echo "### ISO9660"
${MAKEFS} -Z -t cd9660 ${IMG_FILE} ${SRC_DIR} || exit 1
file ${IMG_FILE} || exit 1
if [ ${UNAME} = Linux ]; then
	mount ${IMG_FILE} ${MNT_DIR} || exit 1
	unmount ${MNT_DIR}
elif [ ${UNAME} = FreeBSD ]; then
	MDNAME=`freebsd_init_mdconfig ${IMG_FILE}`
	mount_cd9660 /dev/${MDNAME} ${MNT_DIR} || exit 1
	unmount ${MNT_DIR}
	freebsd_cleanup_mdconfig ${MDNAME}
elif [ ${UNAME} = DragonFly ]; then
	dragonfly_init_vnconfig ${VNNAME} ${IMG_FILE}
	mount_cd9660 /dev/${VNNAME} ${MNT_DIR} || exit 1
	unmount ${MNT_DIR}
	dragonfly_cleanup_vnconfig ${VNNAME}
else
	echo "### Ignore ISO9660 mount on ${UNAME}"
fi
unlink ${IMG_FILE}
echo

# FAT32
echo "### FAT32"
${MAKEFS} -Z -t msdos -s 4g ${IMG_FILE} ${SRC_DIR} || exit 1
file ${IMG_FILE} || exit 1
if [ ${UNAME} = Linux ]; then
	mount ${IMG_FILE} ${MNT_DIR} || exit 1
	unmount ${MNT_DIR}
elif [ ${UNAME} = FreeBSD ]; then
	MDNAME=`freebsd_init_mdconfig ${IMG_FILE}`
	mount_msdosfs /dev/${MDNAME} ${MNT_DIR} || exit 1
	unmount ${MNT_DIR}
	freebsd_cleanup_mdconfig ${MDNAME}
elif [ ${UNAME} = DragonFly ]; then
	dragonfly_init_vnconfig ${VNNAME} ${IMG_FILE}
	mount_msdos /dev/${VNNAME} ${MNT_DIR} || exit 1
	unmount ${MNT_DIR}
	dragonfly_cleanup_vnconfig ${VNNAME}
else
	echo "### Ignore FAT32 mount on ${UNAME}"
fi
unlink ${IMG_FILE}
echo

# exFAT
echo "### exFAT"
${MAKEFS} -Z -t exfat -s 4g ${IMG_FILE} ${SRC_DIR} || exit 1
file ${IMG_FILE} || exit 1
if [ ${UNAME} = Linux ]; then
	mount ${IMG_FILE} ${MNT_DIR} || exit 1
	unmount ${MNT_DIR}
else
	echo "### Ignore exFAT mount on ${UNAME}"
fi
unlink ${IMG_FILE}
echo

# HAMMER2
echo "### HAMMER2"
${MAKEFS} -Z -t hammer2 ${IMG_FILE} ${SRC_DIR} || exit 1
file ${IMG_FILE} || exit 1
if [ ${UNAME} = FreeBSD ]; then
	MDNAME=`freebsd_init_mdconfig ${IMG_FILE}`
	mount_hammer2 /dev/${MDNAME} ${MNT_DIR} || exit 1
	unmount ${MNT_DIR}
	freebsd_cleanup_mdconfig ${MDNAME}
elif [ ${UNAME} = DragonFly ]; then
	dragonfly_init_vnconfig ${VNNAME} ${IMG_FILE}
	mount_hammer2 /dev/${VNNAME} ${MNT_DIR} || exit 1
	unmount ${MNT_DIR}
	dragonfly_cleanup_vnconfig ${VNNAME}
else
	echo "### Ignore HAMMER2 mount on ${UNAME}"
fi
unlink ${IMG_FILE}
echo

echo "success"
