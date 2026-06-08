#!/usr/bin/env python3
"""Generate build_info.h with current build information."""

import datetime
import os
import subprocess

# Get commit hash
build_commit = ""
try:
    build_commit = subprocess.check_output(
        ['git', 'rev-parse', '--short', 'HEAD'],
        stderr=subprocess.DEVNULL
    ).decode().strip()
except Exception:
    pass

# Get branch name
build_branch = ""
try:
    build_branch = subprocess.check_output(
        ['git', 'branch', '--show-current'],
        stderr=subprocess.DEVNULL
    ).decode().strip()
except Exception:
    pass

# Fall back to the SPECS_BRANCH environment variable (set by release.yml)
# since `git branch --show-current` is empty on a detached HEAD / tag checkout.
if build_branch == "":
    build_branch = os.environ.get("SPECS_BRANCH", "")

# Get UTC build time
build_time = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%S")

# Get build source and number from environment
build_source = os.environ.get("SPECS_BUILD_SOURCE", "local")
build_number = os.environ.get("SPECS_BUILD_NUMBER", "")

# Write the header file
with open("utils/build_info.h", "w") as f:
    f.write('#define SPECS_BUILD_COMMIT "{}"\n'.format(build_commit))
    f.write('#define SPECS_BUILD_BRANCH "{}"\n'.format(build_branch))
    f.write('#define SPECS_BUILD_TIME   "{}"\n'.format(build_time))
    f.write('#define SPECS_BUILD_SOURCE "{}"\n'.format(build_source))
    f.write('#define SPECS_BUILD_NUMBER "{}"\n'.format(build_number))

print("Generated utils/build_info.h")
