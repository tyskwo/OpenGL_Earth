# -----------------------------------------------------------------------------
# Add include and link directories
# -----------------------------------------------------------------------------

include(${TLOC_ENGINE_INSTALL_PATH}/../tlocLibraries.cmake)

# common includes
include_directories(
  ${TLOC_INCLUDE_DIRECTORIES}
  ${SOLUTION_INSTALL_PATH}/include/
  ${SOLUTION_INSTALL_PATH}/${SOLUTION_BUILD_FOLDER_NAME}/
  )

# for tests
link_directories(
  ${TLOC_LINK_DIRECTORIES}
  ${SOLUTION_INSTALL_PATH}/${SOLUTION_BUILD_FOLDER_NAME}/
  )
