if(APPLE)
  cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
else()
  cmake_minimum_required(VERSION 2.8.7 FATAL_ERROR)
endif()

project(jsonrpcppTest)

#------------------------------------------------------------------------------
# Required CPM Setup - See: http://github.com/iauns/cpm
#------------------------------------------------------------------------------
set(CPM_DIR "${CMAKE_CURRENT_BINARY_DIR}/cpm-packages" CACHE TYPE STRING)
find_package(Git)
if(NOT GIT_FOUND)
  message(FATAL_ERROR "CPM requires Git.")
endif()
if ((NOT DEFINED CPM_MODULE_CACHE_DIR) AND (NOT "$ENV{CPM_CACHE_DIR}" STREQUAL ""))
  set(CPM_MODULE_CACHE_DIR "$ENV{CPM_CACHE_DIR}")
endif()
if ((NOT EXISTS ${CPM_DIR}/CPM.cmake) AND (DEFINED CPM_MODULE_CACHE_DIR))
  if (EXISTS "${CPM_MODULE_CACHE_DIR}/github_iauns_cpm")
    message(STATUS "Found cached version of CPM.")
    file(COPY "${CPM_MODULE_CACHE_DIR}/github_iauns_cpm/" DESTINATION ${CPM_DIR})
  endif()
endif()
if (NOT EXISTS ${CPM_DIR}/CPM.cmake)
  message(STATUS "Cloning repo (https://github.com/iauns/cpm)")
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" clone https://github.com/iauns/cpm ${CPM_DIR}
    RESULT_VARIABLE error_code
    OUTPUT_QUIET ERROR_QUIET)
  if(error_code)
    message(FATAL_ERROR "CPM failed to get the hash for HEAD")
  endif()
endif()
include(${CPM_DIR}/CPM.cmake)


#-----------------------------------------------------------------------
# Compilation flags
#-----------------------------------------------------------------------
if(MSVC)
    add_definitions(/Ob1) # Inline methods marked as inlined
    add_definitions(/W4)
    add_definitions(/w44640) # Warning on thread un-safe static member
else()
    #-----------------------------------------------------------------------
    # C++11 support
    #-----------------------------------------------------------------------
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    if(CMAKE_HOST_APPLE)
        include(CheckCXXCompilerFlag)
        enable_language(CXX)
        check_cxx_compiler_flag("-stdlib=libc++" COMPILER_HAS_CXX_FLAG_libcxx)
        if(COMPILER_HAS_CXX_FLAG_libcxx)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
        endif()
    endif()
endif()


#-----------------------------------------------------------------------
# Dependencies
#-----------------------------------------------------------------------
CPM_AddModule("jsonrpcpp"
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.."
  EXPORT_MODULE TRUE)

CPM_AddModule("catch"
    GIT_REPOSITORY "https://github.com/bcachet/cpm-catch"
    GIT_TAG origin/master)

CPM_Finish()


#-----------------------------------------------------------------------
# Source
#-----------------------------------------------------------------------
file(GLOB Sources
  "*.cpp"
  "*.hpp"
  )


#-----------------------------------------------------------------------
# Tests
#-----------------------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_executable(tests ${Sources})
target_link_libraries(tests
  ${CPM_LIBRARIES}
  ${PTHREADS})


#-----------------------------------------------------------------------
# VIM support
#-----------------------------------------------------------------------
SET(USE_VIM_CLANG YES CACHE BOOL "If set, will generate a .clang_complete file used by Vim vim-clang plugin")
IF(USE_VIM_CLANG)
  ################################################
  #   GENERATE VIM CLANG_COMPLETE FILE
  ################################################
  GET_PROPERTY(PROJECT_HEADERS DIRECTORY . PROPERTY INCLUDE_DIRECTORIES)
  FILE(WRITE ../.clang "")
  FOREACH(PATH ${PROJECT_HEADERS})
    FILE(APPEND ../.clang "-I${PATH}\n")
  ENDFOREACH(PATH)
ENDIF()

