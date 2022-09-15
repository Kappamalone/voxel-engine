mkdir -p build && cd build
cmake -DTEMPLATE_TESTS=1 -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE="$1"  ..
ln -rsf compile_commands.json ..
ninja
./TEMPLATE
cd ..
