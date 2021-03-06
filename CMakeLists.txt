cmake_minimum_required(VERSION 2.8.0 FATAL_ERROR)

#-----------------------------------------------------------------------
# CPM configuration
#-----------------------------------------------------------------------
set(CPM_MODULE_NAME "jsonrpcpp")
set(CPM_LIB_TARGET_NAME ${CPM_MODULE_NAME})

if ((DEFINED CPM_DIR) AND (DEFINED CPM_UNIQUE_ID) AND (DEFINED CPM_TARGET_NAME))
  set(CPM_LIB_TARGET_NAME ${CPM_TARGET_NAME})
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CPM_DIR})
  include(CPM)
else()
  set (CPM_DIR "${CMAKE_CURRENT_BINARY_DIR}/cpm-packages" CACHE TYPE STRING)
  find_package(Git)
  if(NOT GIT_FOUND)
    message(FATAL_ERROR "CPM requires Git.")
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
endif()


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
CPM_AddModule("socketpp"
    GIT_REPOSITORY "https://github.com/bcachet/socketpp"
    GIT_TAG origin/develop
    EXPORT_MODULE TRUE)

CPM_AddModule("json11"
    GIT_REPOSITORY "https://github.com/bcachet/cpm-json11"
    GIT_TAG origin/master
    EXPORT_MODULE TRUE)

CPM_AddModule("Fit"
    GIT_REPOSITORY "https://github.com/bcachet/cpm-Fit"
    GIT_TAG origin/master
    EXPORT_MODULE TRUE)

CPM_InitModule(${CPM_MODULE_NAME})


#-----------------------------------------------------------------------
# Source
#-----------------------------------------------------------------------
file(GLOB Sources
  "${CPM_MODULE_NAME}/src/*.cpp"
  "${CPM_MODULE_NAME}/*.hpp")


#-----------------------------------------------------------------------
# Build the library.
#-----------------------------------------------------------------------
add_library(${CPM_LIB_TARGET_NAME} ${Sources})
target_link_libraries(${CPM_LIB_TARGET_NAME} ${CPM_LIBRARIES})
