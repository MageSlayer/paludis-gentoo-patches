CMAKE_SOURCE=${1}
DISTRIBUTION=${2}
COMPILER=${3}
BUILD_TYPE=${4:-debug}

myconf=()
if [[ ${DISTRIBUTION} == "exherbo" ]]; then
    myconf+=(
        -DPALUDIS_ENVIRONMENTS="default;test"
        -DPALUDIS_REPOSITORIES="default;accounts;gemcutter;repository"
        -DPALUDIS_DEFAULT_DISTRIBUTION=exherbo
        -DCONFIG_FRAMEWORK=eclectic

        -DRUBY_VERSION:STRING="2.4"
        -DPYTHON_VERSION:STRING="${PYTHON_VERSION}"
    )
elif [[ ${DISTRIBUTION} == "gentoo" ]]; then
    myconf+=(
        -DPALUDIS_ENVIRONMENTS="default;test"
        -DPALUDIS_REPOSITORIES="default"
        -DPALUDIS_DEFAULT_DISTRIBUTION=gentoo
        -DCONFIG_FRAMEWORK=eselect

        -DRUBY_VERSION:STRING="2.4"
        -DPYTHON_VERSION:STRING="${PYTHON_VERSION}"
    )
fi

if [[ ${COMPILER} == gcc ]]; then
    CC=gcc
    CXX=g++
elif [[ ${COMPILER} == clang ]]; then
    CC=clang
    CXX="clang++"
else
    echo "Unknown compiler '${COMPILER}', exiting"
    exit 1
fi

if [[ ${BUILD_TYPE} == "release" ]]; then
    CMAKE_BUILD_TYPE="Release"
    CXXFLAGS="-pipe -O2"
    LDFLAGS=""
elif [[ ${BUILD_TYPE} == "debug" ]]; then
    CMAKE_BUILD_TYPE="Debug"
    CXXFLAGS="-pipe -O0 -pedantic -g3"
    LDFLAGS=""
elif [[ ${3} == "coverage" ]]; then
    CMAKE_BUILD_TYPE="Debug"
    CXXFLAGS="${CXXFLAGS} -g -O0 --coverage -fprofile-arcs -ftest-coverage"
    LDFLAGS="${LDFLAGS} --coverage"
else
    echo "Error: Unknown build type '${BUILD_TYPE}' specified. Exiting."
    exit 1
fi

cmake \
    -G Ninja \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE} \
    -DCMAKE_C_FLAGS:STRING="${CFLAGS}" \
    -DCMAKE_CXX_FLAGS:STRING="${CXXFLAGS}" \
    -DCMAKE_EXE_LINKER_FLAGS:STRING="${LDFLAGS}" \
    -DCMAKE_AR:PATH=x86_64-pc-linux-gnu-ar \
    -DCMAKE_RANLIB:PATH=x86_64-pc-linux-gnu-ranlib \
    -DCMAKE_NM:PATH=x86_64-pc-linux-gnu-nm \
    -DCMAKE_C_COMPILER:PATH=${CC} \
    -DCMAKE_CXX_COMPILER:PATH=${CXX} \
    -DCMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES:PATH='/usr/x86_64-pc-linux-gnu/include;/usr/host/include' \
    -DCMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES:PATH='/usr/x86_64-pc-linux-gnu/include;/usr/host/include' \
    -DCMAKE_INSTALL_PREFIX:PATH=/usr/x86_64-pc-linux-gnu \
    -DCMAKE_FIND_ROOT_PATH:PATH=/usr/x86_64-pc-linux-gnu \
    -DCMAKE_SYSTEM_PREFIX_PATH:PATH=/usr/x86_64-pc-linux-gnu \
    -DCMAKE_INSTALL_LIBDIR:STRING=lib \
    -DCMAKE_INSTALL_DATAROOTDIR:PATH=/usr/share/ \
    -DCMAKE_INSTALL_SYSCONFDIR:PATH=/etc \
    -DCMAKE_INSTALL_DOCDIR:PATH=/usr/share/doc/paludis-scm \
    -DPALUDIS_VIM_INSTALL_DIR=/usr/share/vim/vimfiles \
    -DPALUDIS_CLIENTS=cave \
    -DENABLE_GTEST:BOOL=TRUE \
    -DENABLE_DOXYGEN:BOOL=TRUE \
    -DENABLE_PBINS:BOOL=TRUE \
    -DENABLE_PYTHON:BOOL=TRUE \
    -DENABLE_PYTHON_DOCS:BOOL=TRUE \
    -DENABLE_RUBY:BOOL=TRUE \
    -DENABLE_RUBY_DOCS:BOOL=TRUE \
    -DENABLE_SEARCH_INDEX:BOOL=TRUE \
    -DENABLE_VIM:BOOL=TRUE \
    -DENABLE_XML:BOOL=TRUE \
    "${myconf[@]}" \
    ${CMAKE_SOURCE}
