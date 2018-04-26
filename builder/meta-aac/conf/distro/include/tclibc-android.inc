LIBCEXTENSION = "-android"

# Universal override for Android
MACHINEOVERRIDES = "android"

# Use dedicated tmpdir per API level
TCLIBCAPPEND = "-${ANDROID_PLATFORM}"

LIBC_DEPENDENCIES = ""

# Use OpenSSL 1.1+
# Note: This file will be loaded after default-versions.inc
#       so PREFERRED_VERSION for OpenSSL will be overridden.
PREFERRED_VERSION_openssl = "1.1%"