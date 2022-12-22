MACRO( LIBPLIST_PACKAGE _major _minor)

  SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Library to parse and generate Apple's binary and XML PList format")
  SET(CPACK_PACKAGE_VERSION_MAJOR ${_major})
  SET(CPACK_PACKAGE_VERSION_MINOR ${_minor})
  SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
  SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING.LESSER")
  SET(CPACK_COMPONENT_LIB_DISPLAY_NAME "PList library")
  SET(CPACK_COMPONENT_DEV_DISPLAY_NAME "PList development files")
  SET(CPACK_COMPONENT_PLUTIL_DISPLAY_NAME "PList conversion tool")
  set(CPACK_COMPONENT_DEV_DEPENDS lib)
  set(CPACK_COMPONENT_PLUTIL_DEPENDS lib)
  INCLUDE( CPack )

ENDMACRO( LIBPLIST_PACKAGE )
