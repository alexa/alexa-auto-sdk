# - Find SQLite3 include directory and library
#
#  SQLITE3_INCLUDE_DIRS - Where to find SQLite3 includes, etc.
#  SQLITE3_LIBRARIES    - List of libraries when using SQLite3.
#  SQLITE3_FOUND        - Set to TRUE if SQLite3 is found.

set(GLIB_INCLUDE_DIRS "/usr/include/glib-2.0/")
set(GLIB_LIBRARIES "/usr/lib/x86_64-linux-gnu")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLIB
    DEFAULT_MSG GLIB_LIBRARIES GLIB_INCLUDE_DIRS
)
