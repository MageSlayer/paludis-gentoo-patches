#!/usr/bin/env bash
# vim: set sw=4 sts=4 ts=4 et tw=80 :

# latest = current stable versions
# next = all testing versions unmasked
IMAGE_VERSION=${1:-latest}

PYTHON_ABI=3.10

if [[ ${IMAGE_VERSION} == "latest" ]]; then
    DEPENDENCY_VERSIONS="stable"
elif [[ ${IMAGE_VERSION} == "next" ]]; then
    DEPENDENCY_VERSIONS="testing broken"
    PYTHON_ABI=3.10
else
    echo "Unknown image version, use 'latest' or 'next'"
    exit 1
fi

docker build \
    --no-cache \
    --rm \
    --pull \
    --build-arg DEPENDENCY_VERSIONS="${DEPENDENCY_VERSIONS}" \
    --build-arg PYTHON_ABI="${PYTHON_ABI}" \
    --tag paludis/exherbo-gcc:${IMAGE_VERSION} \
    .
