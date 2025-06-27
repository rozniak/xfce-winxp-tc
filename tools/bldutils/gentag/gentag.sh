#/usr/bin/env bash

# Set up some defaults for the tag
#
WINTC_VER_MAJOR=1
WINTC_VER_MINOR=0
WINTC_VER_BUILD=0
WINTC_VER_DATETIME=`date +%y%m%d%H%M | xargs echo -n`


WINTC_VER_BRANCH='unknown'
WINTC_VER_DATESTAMP=`date +%y%m%d-%H%M | xargs echo -n`
WINTC_VER_HASH='unknown'
WINTC_VER_USER=`whoami | xargs echo -n`

# Check we're in a git repo, if the user has downloaded as ZIP from jithub then
# we won't have any metadata
#
git status >/dev/null 2>&1

if [[ $? -eq 0 ]]
then
    WINTC_VER_BUILD=`git log master --oneline | wc -l | xargs echo -n`
    WINTC_VER_BRANCH=`git branch --show-current | xargs echo -n`
    WINTC_VER_HASH=`git rev-parse --short HEAD | xargs echo -n`
fi

# Construct the tag
#
WINTC_VER_TAG="${WINTC_VER_HASH}.${WINTC_VER_BRANCH}(${WINTC_VER_USER}).${WINTC_VER_DATESTAMP}"

# Output (for CMake, shell scripts can source vars)
echo "WINTC_VER_MAJOR=${WINTC_VER_MAJOR}"
echo "WINTC_VER_MINOR=${WINTC_VER_MINOR}"
echo "WINTC_VER_BUILD=${WINTC_VER_BUILD}"
echo "WINTC_VER_DATETIME=${WINTC_VER_DATETIME}"

echo "WINTC_VER_BRANCH=${WINTC_VER_BRANCH}"
echo "WINTC_VER_DATESTAMP=${WINTC_VER_DATESTAMP}"
echo "WINTC_VER_HASH=${WINTC_VER_HASH}"
echo "WINTC_VER_USER=${WINTC_VER_USER}"

echo "WINTC_VER_TAG=${WINTC_VER_TAG}"
