CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake)

SET(LIBRARY_NAME jsonrpc11)
SET(LIBRARY_VERSION_STRING 0.1.0)

PROJECT(${LIBRARY_NAME})

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)

################################################
#   COMPILATION FLAGS
################################################

if (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /MP ")
else()
  # Assume GCC-style arguments
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wfatal-errors -Wall -W -Werror -Wfloat-equal -Wundef -Wendif-labels")

  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.7")
      message(WARNING "C++0x activated. If you get any errors update to a compiler which fully supports C++11")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()
  endif()

  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      set(CMAKE_COMPILER_IS_CLANGXX 1)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  endif()
endif()

if (CMAKE_HOST_APPLE AND (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

##############################################################
# We install requirements in thirdparty directory
##############################################################
SET(THIRDPARTY_DIR ${PROJECT_SOURCE_DIR}/thirdparty)
INCLUDE_DIRECTORIES(${THIRDPARTY_DIR}/include)
LINK_DIRECTORIES(${THIRDPARTY_DIR}/lib)
SET(CMAKE_INCLUDE_PATH ${THIRDPARTY_DIR}/include ${CMAKE_INCLUDE_PATH})
SET(CMAKE_LIBRARY_PATH ${THIRDPARTY_DIR}/lib ${CMAKE_LIBRARY_PATH})
ADD_SUBDIRECTORY(${THIRDPARTY_DIR})

FILE(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

ADD_SUBDIRECTORY(src)

##############################################################
# Tests
##############################################################
ADD_SUBDIRECTORY(tests)

SET(USE_VIM_CLANG_COMPLETE YES CACHE BOOL "If set, will generate a .clang_complete file used by Vim clang_complete plugin")
IF(USE_VIM_CLANG_COMPLETE)
  ################################################
  #   GENERATE VIM CLANG_COMPLETE FILE
  ################################################
  GET_PROPERTY(PROJECT_HEADERS DIRECTORY tests PROPERTY INCLUDE_DIRECTORIES)
  FILE(WRITE .clang_complete "")
  FOREACH(PATH ${PROJECT_HEADERS})
    FILE(APPEND .clang_complete "-I${PATH}\n")
  ENDFOREACH(PATH)
ENDIF()
