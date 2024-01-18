#!/bin/bash

FILES=""
PACKAGING_BRANCH="packaging"

echo "======== oidc-agent-local-before-script starting======="
export VERSION=`cat VERSION`
# clone the packages file of this repo:
# Try with VERSION
echo "Trying to use branch for packaging: ${PACKAGING_BRANCH}/v${VERSION}"
git clone -b ${PACKAGING_BRANCH}/v${VERSION} http://codebase.helmholtz.cloud/m-team/oidc/oidc-agent.git delme || {
    echo "Falling back to ${PACKAGING_BRANCH}/latest"
    git clone -b ${PACKAGING_BRANCH}/latest http://codebase.helmholtz.cloud/m-team/oidc/oidc-agent.git delme
}


case ${DISTRO} in
    debian|ubuntu)
        # ls -la
        [ -d debian ] && {
            echo "There IS an existing debian folder"
            echo "This is the content:"
            ls -la debian
        }
        [ -d docker ] && {
            echo "There IS an existing docker folder."
        }
        [ -d debian ] || {
            echo "using freshly cloned and adapted debian folder"

            mv delme/debian .
        }
        [ -d docker ] || {
            echo "Creating docker folder"
            mkdir docker
        }
        [ -e docker/debian.mk ] || {
            echo "copying docker/debian.mk"
            mv delme/docker/debian.mk docker/
        }
        # convert templates
        [ -e debian/changelog.template ] && {
            echo "converting changelog.template"
            # define variables
            export VERSION=`cat VERSION`
            export RELNUM=1
            export DATE=`date -R`
            # envsubst
            FILES="${FILES} debian/changelog"
            for FILE in ${FILES}; do
                cat ${FILE}.template | envsubst > ${FILE}
                rm ${FILE}.template
                echo -e "\n---------- generated: ${FILE}"
                # cat ${FILE}
                # echo -e "---------- end of generated: ${FILE} \n"
            done
        }
        case ${RELEASE} in
            buster)     make buster-debsource                           ;;
            bionic)     make bionic-debsource                           ;;
            focal)      make focal-debsource                            ;;
            buster)     make buster-debsource                           ;;
        esac
    ;;
    win) # Do nothing for windows
    ;;
    *) # We expect only RPM by default
        [ -d rpm ] || {
            echo "using freshly cloned and adapted rpm folder"

            mv delme/rpm .
            mv delme/debian .
        }
        # define variables
        export VERSION=`cat VERSION`
        export RELNUM=1
        export DATE=`date +"%a %b %d %Y"`
        # envsubst
        FILES="debian/changelog ${FILES} rpm/oidc-agent.spec"
        for FILE in ${FILES}; do
            cat ${FILE}.template | envsubst > ${FILE}
            rm ${FILE}.template
            echo -e "\n---------- generated: ${FILE}"
            cat ${FILE}
            echo -e "---------- end of generated: ${FILE} \n"
        done
    ;;
esac

# Clean up
rm -rf delme
         
echo "======== oidc-agent-local-before-script done   ========"

