if [ -z ${BUILDER_HOME} ]; then
	echo "Must run from AAC builder" && exit 1
fi

error() {
	echo -e "\e[1;31mERROR\e[0;31m: $1\e[0m" && exit 1
}

note() {
	echo -e "\e[1mNOTE\e[0m: $1"
}

warn() {
	echo -e "\e[1;33mWARNING\e[0;31m: $1\e[0m"
}