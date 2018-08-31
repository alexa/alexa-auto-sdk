agreement_check() {
	if [ -f /.dockerenv ] || [ ! -z ${CODEBUILD_SRC_DIR} ]; then
		# Skip agreement check on container environment
		return
	fi

	cat << EOF

*******************************************************************************
The scripts provided herein will retrieve several third-party libraries,
environments, and/or other software packages at build-time
("External Dependencies") from third-party sources. These are terms and
conditions that you need to agree to abide by if you choose to build the
External Dependencies. Licenses for the External Dependencies may be found at
builder/README.md. If you do not agree with every term and condition
associated with the External Dependencies, enter “QUIT” in the command line
when prompted by the script.
*******************************************************************************

EOF

	answer="dummy"
	while [ ! -z $answer ]; do
		read -p "Type \"QUIT\" to exit the script now, press ENTER to continue: " -r -n 4 answer
		if [ "$answer" = "QUIT" ]; then
			exit
		fi
		echo ""
	done
}