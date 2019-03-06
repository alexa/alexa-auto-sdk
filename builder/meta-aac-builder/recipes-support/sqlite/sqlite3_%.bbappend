INHIBIT_DEFAULT_DEPS = "1"

# Android doesn't support versioning in SONAME
# Note: Since sqlite3.inc override FILES_* variable
#       we also need to override in here...
FILES_lib${BPN}_android = "${libdir}/*.so"
FILES_lib${BPN}-dev_remove_android = "${libdir}/*.so"