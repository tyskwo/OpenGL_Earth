#------------------------------------------------------------------------------
# In most cases, this file should NOT be modified by the user. User modifiable
# files are tlocUserVarsPre.cmake and tlocUserVarsPost.cmake

#------------------------------------------------------------------------------
include(tlocUserVarsPre.cmake)

#------------------------------------------------------------------------------
set(SOURCE_FILES
  ${SOLUTION_SOURCE_FILES}
  ../tlocVersionCheck.cpp
  )

#------------------------------------------------------------------------------
set(ASSETS_PATH
  ${SOLUTION_ASSETS_PATH}
  )

#------------------------------------------------------------------------------
# Create the project
if (${SOLUTION_CURRENT_PROJECT_TYPE} STREQUAL ${SOLUTION_PROJECT_TYPE_EXE})

  add_executable(${SOLUTION_CURRENT_PROJECT_NAME}
    ${TLOC_APP_TYPE}
    ${SOURCE_FILES}
    ${ASSETS_PATH}
    )
  target_link_libraries(${SOLUTION_CURRENT_PROJECT_NAME}
    ${TLOC_DEP_LIBRARIES}
    ${TLOC_ENGINE_LIBRARIES}
    )
  if (ASSETS_PATH)
    add_apple_plist(${SOLUTION_CURRENT_PROJECT_NAME} ${ASSETS_PATH})
  else()
    add_apple_plist(${SOLUTION_CURRENT_PROJECT_NAME} "")
  endif()

  # Link the user defined libraries with this project
  if(SOLUTION_EXECUTABLE_LINK_LIBRARIES)
    target_link_libraries(${SOLUTION_CURRENT_PROJECT_NAME}
      ${SOLUTION_EXECUTABLE_LINK_LIBRARIES}
      )
  endif()

elseif(${SOLUTION_CURRENT_PROJECT_TYPE} STREQUAL ${SOLUTION_PROJECT_TYPE_LIB})
  add_library(${SOLUTION_CURRENT_PROJECT_NAME} STATIC
    ${SOURCE_FILES}
    )

  tloc_install_library(${SOLUTION_CURRENT_PROJECT_NAME} "src")

endif()

#------------------------------------------------------------------------------
# Add platform specific properties
set_platform_specific_properties(${SOLUTION_CURRENT_PROJECT_NAME})

# Add dependencies (if any)
if(SOLUTION_PROJECT_DEPENDENCIES)
  add_dependencies(${SOLUTION_CURRENT_PROJECT_NAME}
    ${SOLUTION_PROJECT_DEPENDENCIES}
    )
endif()

#------------------------------------------------------------------------------
include(tlocUserVarsPost.cmake)
