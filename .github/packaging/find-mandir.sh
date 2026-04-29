#!/bin/bash
# Find the best man directory from manpath output
# Priority: first directory starting with /usr/share, then first starting with /usr, else fallback

MANPATH_OUTPUT=$(manpath 2>/dev/null || echo "/usr/local/share/man:/usr/share/man")

IFS=':' read -ra PATHS <<< "$MANPATH_OUTPUT"

# First priority: directories starting with /usr/share
for path in "${PATHS[@]}"; do
    if [[ "$path" == /usr/share* ]]; then
        echo "$path"
        exit 0
    fi
done

# Second priority: directories starting with /usr (but not /usr/share)
for path in "${PATHS[@]}"; do
    if [[ "$path" == /usr* && "$path" != /usr/share* ]]; then
        echo "$path"
        exit 0
    fi
done

# Fallback
echo "/usr/local/share/man"
