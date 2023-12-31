find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBBSD REQUIRED libbsd)

mark_as_advanced(LIBBSD_LIBRARIES)