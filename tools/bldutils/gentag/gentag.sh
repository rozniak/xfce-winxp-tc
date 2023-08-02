#/bin/bash
cur_branch=`git branch --show-current | xargs echo -n`
cur_datestamp=`date +%y%m%d-%H%M | xargs echo -n`
cur_hash=`git rev-parse --short HEAD | xargs echo -n`
cur_user=`whoami | xargs echo -n`

tag="${cur_hash}.${cur_branch}(${cur_user}).${cur_datestamp}"

echo -n "${tag}"
