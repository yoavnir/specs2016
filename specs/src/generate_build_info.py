#!/usr/bin/env python3
"""Generate build_info.h with current build information."""

import datetime
import os
import subprocess
import sys


def report_success(name, value):
    print('Setting {} to "{}"'.format(name, value))


def report_failure(name, exc):
    sys.stderr.write("Failed to determine {}: {}\n".format(name, repr(exc)))
    # If the failure came from a subprocess, surface the command's stderr too,
    # since that usually explains *why* (e.g. git's "dubious ownership" error).
    output = getattr(exc, "output", None)
    if output:
        if isinstance(output, bytes):
            output = output.decode(errors="replace")
        sys.stderr.write("  stdout: {}\n".format(output.strip()))
    stderr = getattr(exc, "stderr", None)
    if stderr:
        if isinstance(stderr, bytes):
            stderr = stderr.decode(errors="replace")
        sys.stderr.write("  stderr: {}\n".format(stderr.strip()))


def run_git(name, args):
    """Run a git command, capturing stderr so failures can be reported."""
    try:
        value = subprocess.check_output(
            ['git'] + args,
            stderr=subprocess.PIPE
        ).decode().strip()
        if value:
            report_success(name, value)
        return value
    except Exception as exc:
        report_failure(name, exc)
        return ""


# Get commit hash
build_commit = run_git("SPECS_BUILD_COMMIT", ['rev-parse', '--short', 'HEAD'])

# Get branch name
build_branch = run_git("SPECS_BUILD_BRANCH", ['branch', '--show-current'])

# Fall back to the SPECS_BRANCH environment variable (set by release.yml)
# since `git branch --show-current` is empty on a detached HEAD / tag checkout.
if build_branch == "":
    build_branch = os.environ.get("SPECS_BRANCH", "")
    if build_branch:
        report_success("SPECS_BUILD_BRANCH (from SPECS_BRANCH env)", build_branch)

# Get UTC build time
build_time = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%S")
report_success("SPECS_BUILD_TIME", build_time)

# Get build source and number from environment
build_source = os.environ.get("SPECS_BUILD_SOURCE", "local")
report_success("SPECS_BUILD_SOURCE", build_source)
build_number = os.environ.get("SPECS_BUILD_NUMBER", "")
report_success("SPECS_BUILD_NUMBER", build_number)

# Write the header file
with open("utils/build_info.h", "w") as f:
    f.write('#define SPECS_BUILD_COMMIT "{}"\n'.format(build_commit))
    f.write('#define SPECS_BUILD_BRANCH "{}"\n'.format(build_branch))
    f.write('#define SPECS_BUILD_TIME   "{}"\n'.format(build_time))
    f.write('#define SPECS_BUILD_SOURCE "{}"\n'.format(build_source))
    f.write('#define SPECS_BUILD_NUMBER "{}"\n'.format(build_number))

print("Generated utils/build_info.h")
