#!/bin/bash

FILES=""

echo "======== oidc-agent-local-before-script starting======="
case ${DISTRO} in
    debian|ubuntu)
        ls -la
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

            git rev-parse --quiet --verify scc/v${VERSION} > /dev/null  && {
                echo "using branch scc/v${VERSION}"
                git clone -b scc/v${VERSION} http://git.scc.kit.edu/m-team/oidc-agent.git delme
            }
            git rev-parse --quiet --verify scc/v${VERSION} > /dev/null  || {
                echo "using branch scc/latest"
                git clone -b scc/latest http://git.scc.kit.edu/m-team/oidc-agent.git delme
            }

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
            export RELEASE=1
            export DATE=`date -R`
            # envsubst
            FILES="${FILES} debian/changelog"
            for FILE in ${FILES}; do
                cat ${FILE}.template | envsubst > ${FILE}
                rm ${FILE}.template
                cat ${FILE}
            done
        }
    ;;
    *) # We expect only RPM by default
        # define variables
        export VERSION=`cat VERSION`
        export RELEASE=1
        export DATE=`date +"%a %B %d %Y"`
        # envsubst
        FILES="${FILES} rpm/oidc-agent.spec"
        for FILE in ${FILES}; do
            cat ${FILE}.template | envsubst > ${FILE}
            rm ${FILE}.template
            cat ${FILE}
        done
    ;;
esac
         
echo "======== oidc-agent-local-before-script done   ========"

