#!/bin/bash

assert_libuuid() {
	EXPECT=$1
	if [[ `uname` = *BSD ]] || [ `uname` = "DragonFly" ]; then
		return # part of libc
	fi
	if [ "${EXPECT}" = "y" ]; then
		if [[ `uname` = CYGWIN* ]]; then
			ldd ./src/makefs | grep cyguuid
		else
			ldd ./src/makefs | grep libuuid
		fi
		if [ $? -ne 0 ]; then
			echo "XXX libuuid should exist"
			exit 1
		fi
	else
		if [[ `uname` = CYGWIN* ]]; then
			ldd ./src/makefs | grep cyguuid
		else
			ldd ./src/makefs | grep libuuid
		fi
		if [ $? -eq 0 ]; then
			echo "XXX libuuid shouldn't exist"
			exit 1
		fi
	fi
}

assert_hammer2() {
	EXPECT=$1
	if [ "${EXPECT}" = "y" ]; then
		nm ./src/makefs | grep hammer2 >/dev/null
		if [ $? -ne 0 ]; then
			echo "XXX hammer2 should exist"
			exit 1
		fi
	else
		nm ./src/makefs | grep hammer2 >/dev/null
		if [ $? -eq 0 ]; then
			echo "XXX hammer2 shouldn't exist"
			exit 1
		fi
	fi
}

assert_exfat() {
	EXPECT=$1
	if [ "${EXPECT}" = "y" ]; then
		nm ./src/makefs | grep exfat >/dev/null
		if [ $? -ne 0 ]; then
			echo "XXX exfat should exist"
			exit 1
		fi
	else
		nm ./src/makefs | grep exfat >/dev/null
		if [ $? -eq 0 ]; then
			echo "XXX exfat shouldn't exist"
			exit 1
		fi
	fi
}

NO_HAMMER2="USE_HAMMER2=0"
NO_EXFAT="USE_EXFAT=0"

if [[ `uname` = *BSD ]] || [ `uname` = "DragonFly" ]; then
	MAKE=gmake
else
	MAKE=make
fi

for x in "" ${NO_HAMMER2} ${NO_EXFAT}; do
	for y in "" ${NO_HAMMER2} ${NO_EXFAT}; do
		if [ "${x}" = "${y}" ]; then
			continue
		fi

		echo "========================================"
		${MAKE} clean >/dev/null || exit 1

		CMD="${MAKE} ${x} ${y}"
		echo ${CMD}
		${CMD}
		if [ $? -ne 0 ]; then
			echo "XXX \"${CMD}\" failed"
			exit 1
		fi

		if [ "${x}" = ${NO_HAMMER2} ] || [ "${y}" = ${NO_HAMMER2} ]; then
			assert_libuuid "n"
			assert_hammer2 "n"
		else
			assert_libuuid "y"
			assert_hammer2 "y"
		fi

		if [ "${x}" = ${NO_EXFAT} ] || [ "${y}" = ${NO_EXFAT} ]; then
			assert_exfat "n"
		else
			assert_exfat "y"
		fi
	done
done

echo "success"
