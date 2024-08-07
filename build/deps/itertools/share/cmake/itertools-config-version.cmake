set(PACKAGE_VERSION 1.3.0)

if (PACKAGE_FIND_VERSION VERSION_EQUAL PACKAGE_VERSION)
 set(PACKAGE_VERSION_EXACT TRUE)
endif()

if (PACKAGE_FIND_VERSION VERSION_LESS PACKAGE_VERSION)
  set(PACKAGE_VERSION_COMPATIBLE TRUE)
endif()

if (PACKAGE_FIND_VERSION VERSION_GREATER PACKAGE_VERSION)
 set(PACKAGE_VERSION_UNSUITABLE TRUE)
endif()
