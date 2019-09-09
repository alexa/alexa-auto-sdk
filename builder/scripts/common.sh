if [ -z ${BUILDER_HOME} ]; then
	echo "Must run from AAC builder" && exit 1
fi

error_and_exit() {
	echo -e "\033[1;31mERROR\033[0;31m: $1\033[0m" && exit 1
}

error() {
	echo -e "\033[1;31mERROR\033[0;31m: $1\033[0m"
}

note() {
	echo -e "\033[1mNOTE\033[0m: $1"
}

warn() {
	echo -e "\033[1;33mWARNING\033[0;31m: $1\033[0m"
}

exit_if_failure() {
	[ $? -eq 0 ] || exit 1
}