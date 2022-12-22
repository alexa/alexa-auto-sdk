#!/usr/bin/python3
import logging, argparse
from A2ML.processor import A2MLProcessor

parser = argparse.ArgumentParser(description="A2ML Tool")

parser.add_argument("-p", "--parser", help="parser to use for creating the A2ML model", default="A2ML")
parser.add_argument(
    "-g", "--generator", help="generator to use for processing A2ML model and creating output files", default="AASB"
)

parser.add_argument("-i", "--input", metavar="DIR", nargs="+", help="directory containing input files")

parser.add_argument(
    "-d", "--dependencies", metavar="DIR", nargs="+", help="directory containing dependency input files"
)

parser.add_argument("--message-version", help="AASB message version")

parser.add_argument("-o", "--output", metavar="DIR", help="directory to write output files")

parser.add_argument("--no-output", action="store_true", default=False, help="don't generate output files")

parser.add_argument("-v", "--verbose", action="store_true", default=False, help="enable verbose logging")

try:
    builder = A2MLProcessor(vars(parser.parse_args()))
    builder.run()
except Exception as ex:
    logging.exception(f"\033[0;31mERROR: {ex}")

# reset the terminal font
print("\033[0m")
