#!/usr/bin/env bash
# vim: set sw=4 sts=4 ts=4 et tw=80 :

# latest = current stable versions
# next = all testing versions unmasked
IMAGE_VERSION=${1:-latest}

if [[ ${IMAGE_VERSION} == "latest" ]]; then
    DEPENDENCY_VERSIONS="stable"
elif [[ ${IMAGE_VERSION} == "next" ]]; then
    DEPENDENCY_VERSIONS="testing broken"
else
    echo "Unknown image version, use 'latest' or 'next'"
    exit 1
fi

docker build \
    --no-cache \
    --rm \
    --pull \
    --build-arg DEPENDENCY_VERSIONS="${DEPENDENCY_VERSIONS}" \
    --tag paludis/exherbo-gcc:${IMAGE_VERSION} \
    .
