#!/bin/bash
set -e

$(git diff --quiet && git diff --cached --quiet) || (echo "There are repo changes, giving up."; exit 1)

ver=`curl -s http://sqlite.org/news.html | grep '^<a name.*</a><h3>.* - Release ' | \
    sed -e 's/.* - Release \(.*\)<\/.*/\1/'`
file=src/drivers/CMakeLists.txt
perl -pi -e "s/^set\(SQLITE_RECOMMENDED_VERSION.*/set\(SQLITE_RECOMMENDED_VERSION $ver\)/" $file
git diff --quiet && (echo "Already updated to $ver."; exit 1)
git add $file
git commit -m "GIT_SILENT bump recommended version of SQLite to $ver}"
git log -1
