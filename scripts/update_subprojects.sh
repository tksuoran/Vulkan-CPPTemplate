#!/usr/bin/env python3

#!/usr/bin/env bash

# subprojects=(fmt GSL tinyxml2)
# for subproject in "${subprojects[@]}"
# do
#     pushd "subprojects/${subproject}"
#     git remote update
#     git checkout origin/master
#     git status
#     popd
# done

import subprocess

subprojects = ['fmt', 'GSL', 'tinyxml2']

for subproject in subprojects:
