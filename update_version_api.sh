#!/bin/bash

CHECK=0
PRINT=0
SET=0
PRINT=
VERSION=

function usage
{
    echo -e "usage: ./update_version_api.sh [OPTION...]"
    echo -e ""
    echo -e "update the api version"
    echo -e ""
    echo -e " -s, --set    vX.Y   set api version (vX.Y) in all files"
    echo -e " -c, --check         check api version in all files"
    echo -e " -p, --print         print api version in all files"
    echo -e " -h, --help          print this help"
}

while [ "$1" != "" ]; do
    case $1 in
        -s | --set )
            shift
            VERSION=$1
            SET=1
            ;;
        -c | --check )
            shift
            VERSION=$1
            CHECK=1
            ;;
        -p | --print )
            PRINT=1
            ;;
        -h | --help )
            usage
            exit
            ;;
        * )
            usage
            exit 1
    esac
    shift
done

INCLUDE_RE=".*\* Version:.*"
INCLUDE_SUBS=" \* Version:     ${VERSION}"
INCLUDE_PATH="include/*.h"

PSEUDOCODE_RE=$INCLUDE_RE
PSEUDOCODE_SUBS=$INCLUDE_SUBS
PSEUDOCODE_PATH="axiom_docs/pseudo_code/*.c"

APPS_RE=$INCLUDE_RE
APPS_SUBS=$INCLUDE_SUBS
APPS_PATH="../axiom-evi-apps/*"

DRIVER_RE="MODULE_VERSION(.*"
DRIVER_SUBS="MODULE_VERSION(\"${VERSION}\");"
DRIVER_PATH="axiom_netdev_driver/*"

DATASHEET_RE=".*\\\newcommand{\\\versionapi}.*"
DATASHEET_SUBS="\\\newcommand{\\\versionapi}{${VERSION}}"
DATASHEET_PATH="axiom_docs/datasheet/*.tex"

if [ "$SET" = "1" ]; then
    echo "setting version: $VERSION"
    set -x
    sed -i -e "s/${INCLUDE_RE}/${INCLUDE_SUBS}/" ${INCLUDE_PATH}
    sed -i -e "s/${PSEUDOCODE_RE}/${PSEUDOCODE_SUBS}/" ${PSEUDOCODE_PATH}
    grep -rl "$APPS_RE" $APPS_PATH | xargs sed -i -e "s/${APPS_RE}/${APPS_SUBS}/"
    grep -rIl "$DRIVER_RE" $DRIVER_PATH | xargs sed -i -e "s/${DRIVER_RE}/${DRIVER_SUBS}/"
    sed -i -e "s/${DATASHEET_RE}/${DATASHEET_SUBS}/" ${DATASHEET_PATH}
elif [ "$PRINT" = "1" ]; then
    echo "printing version"
    grep -rnIe "$INCLUDE_RE" $INCLUDE_PATH
    grep -rnIe "$PSEUDOCODE_RE" $PSEUDOCODE_PATH
    grep -rnIe "$APPS_RE" $APPS_PATH
    grep -rnIe "$DRIVER_RE" $DRIVER_PATH
    grep -rnIe "$DATASHEET_RE" $DATASHEET_PATH
elif [ "$CHECK" = "1" ]; then
    echo "checking version: $VERSION"
    echo "TODO"
else
    usage
    exit 1
fi
