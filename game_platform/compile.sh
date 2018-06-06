BUILD_TYPE=debug

mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE\
    -DCMAKE_INSTALL_PREFIX=..\
	../src/
make install
cd ..
