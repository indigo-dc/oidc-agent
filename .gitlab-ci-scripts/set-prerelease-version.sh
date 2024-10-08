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
MASTER_BRANCH=""
get_master_branch_of_mteam() {
    git remote -vv | awk -F[\\t@:] '{ print $1 " " $3 }' | while read REMOTE HOST; do 
        # echo " $HOST -- $REMOTE"
        MASTER=$(git remote show "$REMOTE"  2>/dev/null \
            | sed -n '/HEAD branch/s/.*: //p')
        MASTER_BRANCH="refs/remotes/${REMOTE}/${MASTER}"
        [ "x${HOST}" == "xcodebase.helmholtz.cloud" ] && {
            echo "${MASTER_BRANCH}"
            break
        }
        [ "x${HOST}" == "xgit.scc.kit.edu" ] && {
            echo "${MASTER_BRANCH}"
            break
        }
        [ "x${REMOTE}" == "xorigin" ] && {
            echo "${MASTER_BRANCH}"
            break
        }
    done
}

MASTER_BRANCH=$(get_master_branch_of_mteam)
PREREL=$(git rev-list --count HEAD ^"$MASTER_BRANCH")



# use version file:
VERSION=$(cat $VERSION_FILE)
VERSION_ESCAPED=$(echo ${VERSION} | sed s/\\\./\\\\./g); echo $VER
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
    DEBIAN_VERSION_ESCAPED=$(echo ${DEBIAN_VERSION} | sed s/\\\./\\\\./g); echo $VER
    NEW_DEB_VERSION="${TILDE_VERSION}-1"
    sed s%${DEBIAN_VERSION_ESCAPED}%${NEW_DEB_VERSION}% -i debian/changelog
}


# lets see if RPM also needs a version to be set
SPEC_FILES=$(ls rpm/*spec)
[ -z "${SPEC_FILES}" ] || {
    [ -z "${VERSION_ESCAPED}" ] || {
        for SPEC_FILE in $SPEC_FILES; do
            grep -q "$VERSION_ESCAPED" "$SPEC_FILE" && { # version found, needs update
                sed "s/${VERSION_ESCAPED}/${TILDE_VERSION}/" -i "$SPEC_FILE"
            }
        done
    }
}
