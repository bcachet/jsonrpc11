CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE(ExternalProject)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

FILE(MAKE_DIRECTORY
    ${THIRDPARTY_DIR}/lib
    ${THIRDPARTY_DIR}/include
    ${THIRDPARTY_DIR}/bin
)

ExternalProject_Add(
    HippomocksTool
    URL https://github.com/dascandy/hippomocks/archive/042b7fc1e7ac6af2475f32b7f5dc33d8688879b7.zip
    URL_MD5 f48124e1fd6ba0d2975fab71ffc621fc
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND cp -rf <SOURCE_DIR>/HippoMocks/hippomocks.h ${THIRDPARTY_DIR}/include/
    )

ExternalProject_Add(
    CatchTool
    URL https://github.com/philsquared/Catch/archive/d4e5f184369ce34592bb6f89e793bdb22d1d011a.zip
    URL_MD5  6ace7b9180da6f04b26c844fb6c774c9
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND cp -rf <SOURCE_DIR>/single_include/catch.hpp ${THIRDPARTY_DIR}/include/
    )

ExternalProject_Add(
    Json11Lib
    URL https://github.com/bcachet/json11/archive/08e11b332c327a0e97f4302f43509aeca303d4a8.zip
    URL_MD5 451de54e159e32799f818c12c5dbb5fe
    CMAKE_ARGS
      -DCMAKE_INSTALL_PREFIX=${PROJECT_SOURCE_DIR}/thirdparty
      -DPROJECT_BUILD_TESTS=NO
      -DPROJECT_BUILD_SHARED=NO
      -Wno-dev)

