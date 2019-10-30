#!/usr/bin/env python

import commands

MAJOR_VERSION = "0"
MINOR_VERSION = "1"
LAST_COMMIT = commands.getoutput('git log -n 1 --pretty=format:"%H"')

print MAJOR_VERSION + "." + MINOR_VERSION + "." + LAST_COMMIT[0:7]

