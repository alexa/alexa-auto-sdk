# - Find SQLite3 include directory and library
#
#  SQLITE3_INCLUDE_DIRS - Where to find SQLite3 includes, etc.
#  SQLITE3_LIBRARIES    - List of libraries when using SQLite3.
#  SQLITE3_FOUND        - Set to TRUE if SQLite3 is found.

find_path(SQLITE3_INCLUDE_DIR sqlite3.h)

set(SQLITE3_INCLUDE_DIRS ${SQLITE3_INCLUDE_DIR})

find_library(SQLITE3_LIBRARY sqlite3)

set(SQLITE3_LIBRARIES ${SQLITE3_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SQLite3
    DEFAULT_MSG SQLITE3_LIBRARIES SQLITE3_INCLUDE_DIRS
)
