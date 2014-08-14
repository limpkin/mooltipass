#!/bin/sh

if [ $# -lt 1 ]; then
    echo "Extracts the version string from a given hex file"
    echo "Usage: ${0} <hex file>"
    exit 1
fi

if [ ! -f "${1}" ]; then
    echo "${0}: '${1}': No such file"
    exit 1
fi

version=$(strings "${1}" | grep -E '(v[0-9]+(\.[0-9]+){1,2}(-[0-9]+-g[0-9a-f]{5}+)?(-dirty)?)|(g[0-9a-f]{5}(-dirty)?)')
if [ -z "${version}" ]; then
    echo "Version string not found" >&2
else
    echo "${version}"
fi
