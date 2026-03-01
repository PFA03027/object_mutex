

# Debug or Release or ...
# BUILDTYPE=Debug
BUILD_TYPE?=Release
BUILD_CONFIG?=common
BUILD_DIR?=build

JOBS=$(shell grep cpu.cores /proc/cpuinfo | sort -u | sed 's/[^0-9]//g')

#############################################################################################
all: configure-cmake
	cmake --build ${BUILD_DIR} -j ${JOBS} -v --target all

test: build-test
	setarch $(uname -m) -R ctest --test-dir ${BUILD_DIR} -j ${JOBS} -v

build-test: configure-cmake
	cmake --build ${BUILD_DIR} -j ${JOBS} -v --target build-test

configure-cmake:
	cmake -S . -B ${BUILD_DIR} -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DBUILD_CONFIG=${BUILD_CONFIG} 

clean:
	-cmake --build ${BUILD_DIR} -j ${JOBS} -v --target clean

clean-all:
	-rm -fr ${BUILD_DIR}

# This is inatall command example
install: all
	DESTDIR=/tmp/install-test cmake --install ${BUILD_DIR} --prefix /opt/xxx


coverage: clean
	set -e; \
	make BUILD_CONFIG=codecoverage BUILD_TYPE=Debug test;  \
	cd ${BUILD_DIR}; \
	lcov --rc branch_coverage=1 --rc geninfo_unexecuted_blocks=1 --ignore-errors negative --ignore-errors mismatch -c -d . --include '*/inc/object_mutex.hpp' -o output.info; \
	genhtml --branch-coverage -o OUTPUT -p . -f output.info

sanitizer:
	set -e; \
	for i in `seq 1 5`; do \
		make sanitizer.$$i.sanitizer; \
		echo $$i / 5 done; \
	done

sanitizer.%.sanitizer: clean
	make BUILD_CONFIG=common BUILD_TYPE=Debug SANITIZER_TYPE=$* test
