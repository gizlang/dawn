#!/usr/bin/env bash
set -exuo pipefail

echo "Note: You should run gclient sync before this script"

# Remove gitmodules, some third_party/ repositories contain these and leaving them around would
# cause any recursive submodule clones to fail because e.g. some reference internal Google
# repositories. We don't want them anyway.
sh -c 'find . | grep .gitmodules | xargs rm'

# Turn subrepositories into regular folders.
rm -rf third_party/abseil-cpp/.git
rm -rf third_party/angle/.git
rm -rf third_party/swiftshader/.git
rm -rf third_party/tint/.git
rm -rf third_party/vulkan_memory_allocator/.git
rm -rf third_party/zlib/.git
rm -rf third_party/vulkan-deps/.git
rm -rf third_party/vulkan-deps/.gitignore
rm -rf third_party/vulkan-deps/glslang/src/.git
rm -rf third_party/vulkan-deps/spirv-cross/src/.git
rm -rf third_party/vulkan-deps/spirv-headers/src/.git
rm -rf third_party/vulkan-deps/spirv-tools/src/.git
rm -rf third_party/vulkan-deps/vulkan-headers/src/.git
rm -rf third_party/vulkan-deps/vulkan-loader/src/.git
rm -rf third_party/vulkan-deps/vulkan-tools/src/.git
rm -rf third_party/vulkan-deps/vulkan-validation-layers/src/.git

# Remove files that are not needed.
find third_party | grep /tests/ | xargs -n1 rm -rf
find third_party | grep /docs/ | xargs -n1 rm -rf
find third_party | grep /samples/ | xargs -n1 rm -rf
find third_party | grep CMake | xargs -n1 rm -rf
rm -rf third_party/tint/test/
rm -rf third_party/angle/doc/
rm -rf third_party/angle/extensions/
rm -rf third_party/angle/third_party/logdog/
rm -rf third_party/vulkan-deps/glslang/src/Test
rm -rf third_party/vulkan-deps/spirv-cross/src/reference
rm -rf third_party/vulkan-deps/spirv-cross/src/shaders-hlsl-no-opt
rm -rf third_party/vulkan-deps/spirv-cross/src/shaders-msl-no-opt
rm -rf third_party/vulkan-deps/spirv-cross/src/shaders-msl
rm -rf third_party/vulkan-deps/spirv-cross/src/shaders-no-opt
rm -rf third_party/vulkan-deps/spirv-cross/src/shaders
rm -rf third_party/vulkan-deps/spirv-tools/src/test

rm -rf third_party/swiftshader/third_party/SPIRV-Tools # already in third_party/vulkan-deps/spirv-tools
rm -rf third_party/swiftshader/third_party/SPIRV-Headers # already in third_party/vulkan-deps/spirv-headers

git add third_party/
echo "you may now 'git commit -s -m 'update dependencies' if you are happy with the staged changes"
