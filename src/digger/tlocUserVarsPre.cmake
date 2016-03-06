#------------------------------------------------------------------------------
# This file is included AFTER CMake adds the executable/library
# Do NOT remove the following variables. Modify the variables to suit your 
# project.

# Do NOT remove the following variables. Modify the variables to suit your project
set(SOLUTION_SOURCE_FILES
  src/main.cpp
  )

# Do not include individual assets here. Only add paths
set(SOLUTION_ASSETS_PATH
  ../../assets
  )

# Dependent project is compiled after dependency
set(SOLUTION_PROJECT_DEPENDENCIES
  "skopworksLib"
  )

# Libraries that the executable needs to link against
set(SOLUTION_EXECUTABLE_LINK_LIBRARIES
  "skopworksLib"
  )
