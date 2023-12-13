find_package(PkgConfig REQUIRED)

pkg_check_modules(CHECK REQUIRED check)

mark_as_advanced(CHECK_INCLUDE_DIRS CHECK_LIBRARIES)