include(tlocUserVarsPre.cmake)

# -----------------------------------------------------------------------------
# functions
# -----------------------------------------------------------------------------

function(add_apple_plist EXEC_NAME ASSETS_PATH)
  if(TLOC_PLATFORM_IOS)
    # Check if a PLIST file was specified. If not, then go with default.
    if(SOLUTION_IOS_PLIST_PATH)
      set(PLIST_PATH ${SOLUTION_IOS_PLIST_PATH})
    else()
      set(PLIST_PATH "${SOLUTION_INSTALL_PATH}/platform/iOS/tlocEngine.plist")
    endif()

    set(EXECUTABLE_NAME "\${EXECUTABLE_NAME}" PARENT_SCOPE)
    set(PRODUCT_NAME    "\${PRODUCT_NAME}" PARENT_SCOPE)
    set(BUNDLE_NAME     ${EXEC_NAME} PARENT_SCOPE)
    set_target_properties(${EXEC_NAME} PROPERTIES 
		MACOSX_BUNDLE_INFO_PLIST ${PLIST_PATH})
    set_target_properties(${EXEC_NAME} PROPERTIES
    		RESOURCE "${ASSETS_PATH}")
  endif()
endfunction()

#------------------------------------------------------------------------------
# Install function for libraries

function(tloc_install_library target_name include_folder)
  if(DISTRIBUTION_FULL_SOURCE)
    file(WRITE ${CMAKE_BINARY_DIR}/${target_name}Copy.cmake
      "file(COPY \"${SOLUTION_PATH}/src/${target_name}/${include_folder}\" 
      DESTINATION \"${SOLUTION_INSTALL_PATH}/include/${target_name}\" 
      FILE_PERMISSIONS OWNER_READ
      FILES_MATCHING PATTERN \"*.h\" PATTERN \"*.cpp\"
      )")
  else()
    file(WRITE ${CMAKE_BINARY_DIR}/${target_name}Copy.cmake
      "file(COPY \"${SOLUTION_PATH}/src/${target_name}/${include_folder}\" 
      DESTINATION \"${SOLUTION_INSTALL_PATH}/include/${target_name}\" 
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

# -----------------------------------------------------------------------------
# Start generating libraries, projects and tests
# -----------------------------------------------------------------------------

tloc_add_definitions_strict()

message("")
message(STATUS "-- Libraries --")
foreach(lib ${SOLUTION_LIBRARY_PROJECTS})
  message(STATUS "Adding ${lib} library project")
  set(SOLUTION_CURRENT_PROJECT_TYPE ${SOLUTION_PROJECT_TYPE_LIB})
  set(SOLUTION_CURRENT_PROJECT_NAME ${lib})

  add_subdirectory(${lib})

  # put the libraries in the "Libraries" folder
  set_target_properties(${lib} 
    PROPERTIES
    FOLDER Libraries
    )
endforeach()
message(STATUS "-- Libraries -- DONE") 

message("")
message(STATUS "-- Executables --")
foreach(exec ${SOLUTION_EXECUTABLE_PROJECTS})
  message(STATUS "Adding ${exec} executable project")
  set(SOLUTION_CURRENT_PROJECT_TYPE ${SOLUTION_PROJECT_TYPE_EXE})
  set(SOLUTION_CURRENT_PROJECT_NAME ${exec})
  
  add_subdirectory(${exec})

  # put the executable in the "Projects" folder
  set_target_properties(${exec} 
    PROPERTIES
    FOLDER Projects
    )
endforeach()
message(STATUS "-- Executables -- DONE")

# -----------------------------------------------------------------------------
# Add all test executables
# -----------------------------------------------------------------------------

if(TLOC_INCLUDE_TESTS)
  tloc_add_definitions()

  message("")
  message(STATUS "-- Tests --")
  foreach(test ${SOLUTION_TEST_PROJECTS})
    message(STATUS "Adding ${test} executable project")
    set(SOLUTION_CURRENT_PROJECT_TYPE ${SOLUTION_PROJECT_TYPE_EXE})
    set(SOLUTION_CURRENT_PROJECT_NAME ${test})

  # test projects require exceptions and rtti to be enabled - build fails in xcode only
  if(TLOC_COMPILER_XCODE)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG_DEFAULT} -DTLOC_DEBUG -std=c++11 -stdlib=libc++ -Wno-unused-function -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE_DEFAULT} -DTLOC_RELEASE -std=c++11 -stdlib=libc++ -Wno-unused-function")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO_DEFAULT} -DTLOC_RELEASE_DEBUGINFO -std=c++11 -stdlib=libc++ -Wno-unused-function -g")
  endif()
    
    add_subdirectory(${test})

    # put the executable in the "Tests" folder
    set_target_properties(${test} 
      PROPERTIES
      FOLDER Tests 
      )
  endforeach()
  message(STATUS "-- Tests -- DONE")
endif()

# -----------------------------------------------------------------------------
# Generate tlocAssetPath.h file
# -----------------------------------------------------------------------------

if (NOT ASSETS_PATH_FILE_NAME)
  set(ASSETS_PATH_FILE_NAME "gameAssetsPath")
endif()

if(NOT DISTRIBUTION_BUILD)
  SET(ASSETS_PATH ${SOLUTION_ASSETS_PATH})
else()
  message(STATUS ${SOLUTION_ASSETS_PATH})
  SET(ASSETS_PATH "../../../assets/")

  add_custom_target(tlocAssetsCopier ALL)
  add_custom_command(TARGET tlocAssetsCopier POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${CMAKE_SOURCE_DIR}/assets"
      "${SOLUTION_INSTALL_PATH}/assets")

endif()

FILE(WRITE "${SOLUTION_PATH}/INSTALL/${SOLUTION_BUILD_FOLDER_NAME}/${ASSETS_PATH_FILE_NAME}.h" 
"
#pragma once

#ifndef _ASSETS_PATH_H_
#define _ASSETS_PATH_H_

#include <cstring>

#if defined (__APPLE__) && defined (__OBJC__)
# import <Foundation/Foundation.h>
#endif

#if defined(TLOC_OS_WIN)

inline const char* GetAssetsPath()
{
static const char* assetPath = \"${ASSETS_PATH}/\";
  return assetPath;
}
#elif defined(TLOC_OS_IPHONE)
inline const char* GetAssetsPath()
{
  static char assetPath[1024];
  strcpy(assetPath, [[[NSBundle mainBundle] resourcePath]
                     cStringUsingEncoding:[NSString defaultCStringEncoding]]);
  strcat(assetPath, \"/assets/\");

  return assetPath;
}
#endif

#endif // _ASSETS_PATH_H_

")

include(tlocUserVarsPost.cmake)
