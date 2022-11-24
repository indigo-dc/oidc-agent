#!/bin/bash

DEVSTRING="pr"
VERSION_FILE=VERSION

while [[ $# -gt 0 ]]; do
  case $1 in
    --devstring)
      DEVSTRING="$2"
      shift # past argument
      shift # past value
      ;;
    --version_file)
      VERSION_FILE="$2"
      shift # past argument
      shift # past value
      ;;
    -*|--*)
      echo "Unknown option $1"
      exit 1
      ;;
  esac
done

[ "x${CI}" == "xtrue" ] && {
    git config --global --add safe.directory "$PWD"
}

# Get master branch name:
#   use origin if exists
#   else use last found remote
REMOTES=$(git remote show)
for R in $REMOTES; do
    MASTER=$(git remote show "$R"  2>/dev/null \
        | sed -n '/HEAD branch/s/.*: //p')
    MASTER_BRANCH="refs/remotes/${R}/${MASTER}"
    #echo "Master-branch: ${MASTER_BRANCH}"
    [ "x${R}" == "xorigin" ] && break
done

PREREL=$(git rev-list --count HEAD ^"$MASTER_BRANCH")

# use version file:
VERSION=$(cat $VERSION_FILE)
PR_VERSION="${VERSION}-${DEVSTRING}${PREREL}"
echo "$PR_VERSION" > $VERSION_FILE
echo "$PR_VERSION"

TILDE_VERSION="$(echo $PR_VERSION | sed 's/-/~/g')"

# if we store the version in debian changelog:
[ -e debian/changelog ] && {
    # get the latest version
    DEBIAN_VERSION=$(cat debian/changelog \
        | grep "(.*) " \
        | head -n 1 \
        | cut -d\( -f 2 \
        | cut -d\) -f 1)
    NEW_DEB_VERSION="${TILDE_VERSION}-1"
    sed s%${DEBIAN_VERSION}%${NEW_DEB_VERSION}% -i debian/changelog
}

# lets see if RPM also needs a version to be set
SPEC_FILES=$(ls rpm/*spec)
[ -z "${SPEC_FILES}" ] || {
    [ -z "${VERSION}" ] || {
        for SPEC_FILE in $SPEC_FILES; do
            grep -q "$VERSION" "$SPEC_FILE" && { # version found, needs update
                sed "s/${VERSION}/${TILDE_VERSION}/" -i "$SPEC_FILE"
            }
        done
    }
}
