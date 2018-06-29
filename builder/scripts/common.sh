if [ -z ${BUILDER_HOME} ]; then
	echo "Must run from AAC builder" && exit 1
fi

error() {
	echo "ERROR: $1" && exit 1
}

note() {
	echo "NOTE: $1"
}

warn() {
	echo "WARNING: $1"
}