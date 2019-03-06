#********************************************************************************
# Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License").
# You may not use this file except in compliance with the License.
# A copy of the License is located at
#
#     http://aws.amazon.com/apache2.0/
#
# or in the "license" file accompanying this file. This file is distributed
# on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
# express or implied. See the License for the specific language governing
# permissions and limitations under the License.
#*********************************************************************************

agreement_check() {
	cat << EOF

*******************************************************************************
The scripts provided herein will retrieve several third-party libraries,
environments, and/or other software packages at build-time
("External Dependencies") from third-party sources. These are terms and
conditions that you need to agree to abide by if you choose to build the
External Dependencies. Licenses for the External Dependencies may be found at
README.md. If you do not agree with every term and condition
associated with the External Dependencies, enter “QUIT” in the command line
when prompted by the script.
*******************************************************************************

EOF

	answer="dummy"
	while [ ! -z $answer ]; do
		read -p "Type \"QUIT\" to exit the script now, press ENTER to continue: " -r  answer
		if [ "$answer" = "QUIT" ]; then
			exit 1
		fi
		echo ""
	done
}

agreement_check
exit 0