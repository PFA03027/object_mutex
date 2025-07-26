

# Debug or Release or ...
# BUILDTYPE=Debug
BUILDTYPE?=Release
BUILDTARGET?=common
BUILDDIR=build

JOBS=$(shell grep cpu.cores /proc/cpuinfo | sort -u | sed 's/[^0-9]//g')

cmake_configure: ${BUILDDIR}
	set -e; cd ${BUILDDIR}; cmake -DCMAKE_BUILD_TYPE=${BUILDTYPE} -D BUILD_TARGET=${BUILDTARGET} -G "Unix Makefiles" ../

cmake_debug_configure: ${BUILDDIR}
	set -e; cd ${BUILDDIR}; cmake -DCMAKE_BUILD_TYPE=Debug -D BUILD_TARGET=${BUILDTARGET} -G "Unix Makefiles" ../

cmake_codecoverage_configure: ${BUILDDIR}
	set -e; cd ${BUILDDIR}; cmake -DCMAKE_BUILD_TYPE=Debug -D BUILD_TARGET=codecoverage -G "Unix Makefiles" ../

all: cmake_configure
	set -e; cd ${BUILDDIR}; cmake --build . -j ${JOBS} -v

debug-all: cmake_debug_configure
	set -e; cd ${BUILDDIR}; cmake --build . -j ${JOBS} -v

test: debug-all
	set -e; cd ${BUILDDIR}; ctest -j ${JOBS} -v

coverage: clean
	set -e; \
	make BUILDTARGET=codecoverage BUILDTYPE=Debug test;  \
	cd ${BUILDDIR}; \
	lcov -c -d . --include 'inc/*' --branch-coverage -o tmp.info; \
	genhtml --branch-coverage -o OUTPUT -p . -f tmp.info

clean:
	-rm -fr ${BUILDDIR}

echo:
	echo JOBS = ${JOBS}

${BUILDDIR}:
	mkdir -p ${BUILDDIR}
