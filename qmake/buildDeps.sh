#!/bin/bash
################################################################################
#   QBuildSystem
#
#   Copyright(c) 2021 by Targoman Intelligent Processing <http://tip.co.ir>
#
#   Redistribution and use in source and binary forms are allowed under the
#   terms of BSD License 2.0.
################################################################################
source `dirname ${BASH_SOURCE[0]}`/helper.shi

PROJECT_BASE_DIR=$1
DEPS_BUILT_FILE=$2
DONT_BUILD_DEPS=$3

if [ "$DONT_BUILD_DEPS" -eq 1 ]; then
    warn "Dependency build has been disabled"
    exit 0;
fi

DEPS_BUILT=$(realpath $(dirname $DEPS_BUILT_FILE))/$(basename $DEPS_BUILT_FILE)
cd $(realpath $PROJECT_BASE_DIR)
BASE_PATH=$(pwd)

CPU_COUNT=$(cat /proc/cpuinfo | grep processor | wc -l)
CPU_COUNT=$((CPU_COUNT-1))

QMAKE_CLI=qmake-qt5
if ! which $QMAKE_CLI >/dev/null 2>&1; then
    QMAKE_CLI=qmake
    if ! which $QMAKE_CLI >/dev/null 2>&1; then
        error "'qmake' command not found"
        exit 1
    fi
fi

echo "Using $QMAKE_CLI ..."

if ! grep "Using Qt version 5." <<< $($QMAKE_CLI -v) >/dev/null 2>&1; then
    error "Qt version 5.x is needed for compiling this library."
    exit 1
fi

mkdir -p $BASE_PATH/out

DisabledDeps=''
for Disabled in ${@:4}; do
    DisabledDeps="$DisabledDeps $Disabled=0"
done

if [ -f .gitmodules ]; then
  Deps=$(grep "\[submodule " .gitmodules | cut -d ' ' -f 2 | tr -d '\"\]')
  for Dep in $Deps; do
    info "\n=====================> Building $Dep <========================"
    if [[ " ${@:4} " =~ " $(basename $Dep) " ]]; then
      ignore "Dependency $Dep building ignored as specified"
      continue
    fi
    if  fgrep "$Dep" "$DEPS_BUILT" >/dev/null 2>&1; then
        ignore "Dependency $Dep has already built."
        continue
    fi
    pushd $Dep
      if [ -r *".pro" ]; then
          make distclean
          $QMAKE_CLI PREFIX=$BASE_PATH/out DONT_BUILD_DEPS=1 $DisabledDeps
          make install -j $CPU_COUNT
          if [ $? -ne 0 ]; then
          error "Error building $Dep as Qt project"
            exit 1
          fi
      elif [ -r "CMakeLists.txt" ];then
          mkdir -p buildForProject
          pushd buildForProject
            cmake -DCMAKE_INSTALL_PREFIX:PATH=$BASE_PATH/out ..
            make install -j $CPU_COUNT
            if [ $? -ne 0 ]; then
                error "Error building $Dep as CMake project"
                exit 1
            fi
          popd
      else
        warn "Dependency $Dep type could not be determined so will not be compiled"
      fi
      echo $Dep >> $DEPS_BUILT
    popd
  done
fi 

happy "Dependency build finished\n"
