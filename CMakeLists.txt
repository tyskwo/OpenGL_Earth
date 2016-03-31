cmake_minimum_required(VERSION 2.6)

set(SOLUTION_NAME "solution" CACHE STRING "You may change the name based on build configuration")

#--------------------------------------------------------------------------
# Project name
project(${SOLUTION_NAME})

set(internal_FIXME "FIXME!")

#------------------------------------------------------------------------------
# Install function for libraries

function(tloc_install_library target_name include_folder)
  if(DISTRIBUTION_FULL_SOURCE)
    file(WRITE ${CMAKE_BINARY_DIR}/${target_name}Copy.cmake
      "file(COPY \"${TLOC_ENGINE_PATH}/src/${include_folder}\" 
      DESTINATION \"${TLOC_ENGINE_INSTALL_PATH}/include/\" 
      FILE_PERMISSIONS OWNER_READ
      FILES_MATCHING PATTERN \"*.h\" PATTERN \"*.cpp\"
      )")
  else()
    file(WRITE ${CMAKE_BINARY_DIR}/${target_name}Copy.cmake
      "file(COPY \"${TLOC_ENGINE_PATH}/src/${include_folder}\" 
      DESTINATION \"${TLOC_ENGINE_INSTALL_PATH}/include/\" 
      FILE_PERMISSIONS OWNER_READ
      FILES_MATCHING PATTERN \"*.h\" PATTERN \"*.opt.inl.h\" EXCLUDE
      )")
  endif()

  add_custom_command(TARGET ${target_name} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/${target_name}Copy.cmake")
endfunction()

function(tloc_check_paths path_list)
  foreach(PATHDIR ${path_list})
    if (${${PATHDIR}} MATCHES ${internal_FIXME})
      message(FATAL_ERROR "Please enter a path for ${PATHDIR}")
    endif()
  endforeach()
endfunction()

#------------------------------------------------------------------------------
# CACHE Variables

set(TLOC_DEP_INSTALL_PATH ${internal_FIXME} CACHE PATH "The tloc dependency INSTALL path goes here.")
set(TLOC_ENGINE_INSTALL_PATH ${internal_FIXME} CACHE PATH "Install directory for the engine.")
set(TLOC_INCLUDE_TESTS TRUE CACHE BOOL "Include tests in the project.")

set(SOLUTION_PATH ${CMAKE_SOURCE_DIR} CACHE PATH "Root folder for this project. You do not need to change this path" FORCE)
set(SOLUTION_INSTALL_PATH ${CMAKE_SOURCE_DIR}/INSTALL CACHE PATH "Install directory for the binaries")
set(SOLUTION_ASSETS_PATH ${CMAKE_SOURCE_DIR}/assets CACHE PATH "You can specify your custom assets path here. NOTE: In Distribution build, the assets from this folder are copied to be relative to the binary files.")

if(TLOC_PLATFORM_IOS)
  set(SOLUTION_IOS_PLIST_PATH "" CACHE PATH "If you have your own custom plist, specify here. If blank, 2LoC will generate one for you.")
endif()

#------------------------------------------------------------------------------
# Common configuration file that sets up platform and compiler specific details
tloc_check_paths(${TLOC_DEP_INSTALL_PATH})
set(TLOC_COMMON_CMAKE_PATH "${TLOC_DEP_INSTALL_PATH}/tlocCommon.cmake")
include(${TLOC_COMMON_CMAKE_PATH})

tloc_get_leaf_folder_name(${CMAKE_BINARY_DIR} SOLUTION_BUILD_FOLDER_NAME)

#--------------------------------------------------------------------------
# User adjustable variables
include(tlocUserVars.cmake)

#--------------------------------------------------------------------------
# Check paths are valid

set(TLOC_CHECK_PATHS
    ${TLOC_CHECK_PATHS}
    TLOC_DEP_INSTALL_PATH
    TLOC_ENGINE_INSTALL_PATH
    )
tloc_check_paths(${TLOC_CHECK_PATHS})

#--------------------------------------------------------------------------
# Sanity checks
list(LENGTH SOLUTION_EXECUTABLE_PROJECTS tlocExecProjLength)
list(LENGTH SOLUTION_LIBRARY_PROJECTS tlocLibProjLength)
if(${tlocExecProjLength} EQUAL 0 AND 
   ${tlocLibProjLength} EQUAL 0)
   message(SEND_ERROR "You must have at least one executable or library")
endif()

#------------------------------------------------------------------------------
# Set binary build directories

foreach (OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
  string (TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${SOLUTION_INSTALL_PATH}/${SOLUTION_BUILD_FOLDER_NAME}/lib/${CMAKE_CFG_INTDIR})
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${SOLUTION_INSTALL_PATH}/${SOLUTION_BUILD_FOLDER_NAME}/lib/${CMAKE_CFG_INTDIR})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${SOLUTION_INSTALL_PATH}/${SOLUTION_BUILD_FOLDER_NAME}/bin/${CMAKE_CFG_INTDIR})
endforeach (OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})

#------------------------------------------------------------------------------
# Subdirectories

message("")
add_subdirectory(src)
