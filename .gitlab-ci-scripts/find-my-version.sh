#!/bin/bash

find_my_version() {
    AVAILABLE_VERSIONS=$(git branch -vva \
        | grep packaging/v[0-9] \
        | awk '{ print $1 }' \
        | awk -F/ '{ print $4 }' \
        | sort -V \
        | grep -v ^$ \
    )
    LATEST_VERSION=$(git branch -vva \
        | grep packaging/v[0-9] \
        | awk '{ print $1 }' \
        | awk -F/ '{ print $4 }' \
        | sort -V \
        | tail -n 1 \
    )
    MY_VERSION="v$(cat VERSION)"
    # MY_VERSION="v4.9.0"
    # MY_VERSION="v5.0.0"
    # MY_VERSION="v5.0.1"

    # echo "My version: ${MY_VERSION}"
    # echo "Available:  ${AVAILABLE_VERSIONS}"
    # echo "Latest:     ${LATEST_VERSION}"

    for VERSION in ${AVAILABLE_VERSIONS}; do
        # echo ""
        TESTVERSIONS="${VERSION} ${MY_VERSION}"
        RESULTING_VERSION=$(for i in ${TESTVERSIONS}; do echo $i; done \
            | sort -V \
            | tail -n 1 \
        )
        # echo "Testing: ${VERSION} Result: ${RESULTING_VERSION}"
        [[ "${RESULTING_VERSION}" == "${MY_VERSION}" ]] && {
            VERSION_TO_USE=${VERSION}
            # echo "Apparently:  ${RESULTING_VERSION} == ${MY_VERSION}"
        }
    done

    # echo -e "\n"
    # echo "VERSION_TO_USE: ${VERSION_TO_USE}"
    echo "${VERSION_TO_USE}"
}
