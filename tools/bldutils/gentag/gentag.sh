#/bin/bash

# Set up some defaults for the tag
#
cur_branch='unknown'
cur_datestamp=`date +%y%m%d-%H%M | xargs echo -n`
cur_hash='unknown'
cur_user=`whoami | xargs echo -n`

# Check we're in a git repo, if the user has downloaded as ZIP from jithub then
# we won't have any metadata
#
git status >/dev/null 2>&1

if [[ $? -eq 0 ]]
then
    cur_branch=`git branch --show-current | xargs echo -n`
    cur_hash=`git rev-parse --short HEAD | xargs echo -n`
fi

# Construct and output the tag now
#
tag="${cur_hash}.${cur_branch}(${cur_user}).${cur_datestamp}"

echo -n "${tag}"