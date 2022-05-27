# Copyright 2016 The Fuchsia Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
if [[ -z "${TOOL_NAME}" ]]; then
  echo "This script is not intended to be run directly.";
  exit 1
fi
case "$(uname -s)" in
  Darwin)
    readonly HOST_PLATFORM="mac-x64"
    ;;
  Linux)
    readonly HOST_PLATFORM="linux-x64"
    ;;
  *)
    echo "Unknown operating system. Cannot run ${TOOL_NAME}."
    exit 1
    ;;
esac
readonly TOOL_PATH="${SCRIPT_ROOT}/${HOST_PLATFORM}/${TOOL_NAME}"
if [[ ! -x "${TOOL_PATH}" ]]; then
  echo "Cannot find ${TOOL_PATH}"
  exit 1
fi
exec "${TOOL_PATH}" "$@"
