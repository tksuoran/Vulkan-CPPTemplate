#!/usr/bin/env bash

set -e

printf "Locating tools\n"
compiler=$(readlink -e "build/subprojects/glslang/StandAlone/glslangValidator")
optimizer=$(readlink -e "build/subprojects/SPIRV-Tools/tools/spirv-opt")
dis=$(readlink -e "build/subprojects/SPIRV-Tools/tools/spirv-dis")
cross=$(readlink -e "build/subprojects/SPIRV-Cross/spirv-cross")
printf "\tcompiler  = ${compiler}\n"
printf "\toptimizer = ${optimizer}\n"
printf "\tdis       = ${dis}\n"
printf "\tcross     = ${cross}\n"
#${compiler} --help
#${optimizer} --help
(
    cd "res/shaders" || exit
    printf "Compiling\n"
    ${compiler} -V -g simple.frag -o simple.frag.spv --target-env vulkan1.1
    ${compiler} -V -g simple.vert -o simple.vert.spv --target-env vulkan1.1
    printf "Optimizing\n"
    ${optimizer} -O simple.frag.spv -o simple.frag.opt.spv --target-env=vulkan1.1 --validate-after-all
    ${optimizer} -O simple.vert.spv -o simple.vert.opt.spv --target-env=vulkan1.1 --validate-after-all
    printf "Disassembly\n"
    ${dis} simple.frag.opt.spv
    ${dis} simple.vert.opt.spv
    printf "Reflection\n"
    ${cross} simple.frag.opt.spv -V --reflect --remove-unused-variables --output simple.frag.opt.json
    ${cross} simple.vert.opt.spv -V --reflect --remove-unused-variables --output simple.vert.opt.json
    printf "Done\n"
)
