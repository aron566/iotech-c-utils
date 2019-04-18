#!/bin/sh
set -e -x

ARCH=`uname -m`
ROOT=$(dirname $(dirname $(readlink -f $0)))

UTEST=false
VALG=false
CPPCHK=false
LCOV=false

case ${ARCH} in
  armv6l)
    BROOT=${ROOT}/arm32
    break
  ;;
  armv7l)
    BROOT=${ROOT}/arm32
    break
  ;;
  aarch64)
    BROOT=${ROOT}/arm64
    break
  ;;
  x86_64)
    BROOT=${ROOT}/x86_64
    break
  ;;
  *)
    echo "Unsupported: ${ARCH}"
    exit 2
  ;;
esac

# Process arguments

while [ $# -gt 0 ]
do
  case $1 in
     -utest)
      UTEST=true
      shift 1
    ;;
    -valgrind)
      VALG=true
      shift 1
    ;;
    -cppcheck)
      CPPCHK=true
      shift 1
    ;;
    -lcov)
      LCOV=true
      shift 1
    ;;
    *)
      shift 1
    ;;
  esac
done

# Release build

mkdir -p ${BROOT}/release
cd ${BROOT}/release
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release $ROOT/src
make 2>&1 | tee release.log
make package

# Static release build

mkdir -p ${BROOT}/static
cd ${BROOT}/static
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release -DCUTILS_BUILD_STATIC=ON $ROOT/src
make 2>&1 | tee static.log
make package

# Debug build

mkdir -p ${BROOT}/debug
cd ${BROOT}/debug
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug $ROOT/src
make 2>&1 | tee debug.log

# Unit tests

if [ "$UTEST" = "true" ]
then
  cd ${BROOT}/release
  c/utests/runner/runner -a -j
fi

# Coverage

if [ "$LCOV" = "true" ]
then

  # Build with profiling enabled

  mkdir -p ${BROOT}/lcov
  cd ${BROOT}/lcov
  cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug -DCUTILS_BUILD_LCOV=ON $ROOT/src
  make

  # Run executables

  c/examples/scheduler
  c/examples/data
  c/examples/pubsub
  c/examples/container
  c/utests/runner/runner -a -j

  # Generate coverage html report

  lcov --capture --no-external -d . -b $ROOT/src -o lcov.tmp1
  lcov --remove lcov.tmp1 "*/c/cunit/*" -o lcov.tmp2
  genhtml -o html lcov.tmp2
  gcovr --xml > cobertura.xml

fi

# Run cppcheck if configured

if [ "$CPPCHK" = "true" ]
then
  cd ${ROOT}
  cppcheck -DNDEBUG -D_GNU_SOURCE --std=c11 --xml --xml-version=2 --enable=performance --enable=portability --enable=warning --relative-paths --output-file=${BROOT}/release/cppcheck.xml -I ./include ./src/c
fi

# Valgrind

if [ "$VALG" = "true" ]
then
  cd ${BROOT}/debug
  VG_FLAGS="--xml=yes --leak-resolution=high --num-callers=16 --track-origins=yes --tool=memcheck --leak-check=full --show-reachable=yes"
  valgrind $VG_FLAGS --xml-file=scheduler_vg.xml c/examples/scheduler
  valgrind $VG_FLAGS --xml-file=data_vg.xml c/examples/data
  valgrind $VG_FLAGS --xml-file=pubsub_vg.xml c/examples/pubsub
  valgrind $VG_FLAGS --xml-file=container_vg.xml c/examples/container
  valgrind $VG_FLAGS --xml-file=utests_vg.xml c/utests/runner/runner -a -j
fi

# Allow deletion of generated files in mounted volume

chmod -R a+rw ${BROOT}
