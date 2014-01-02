
# Specify BUILDTYPE=Release on the command line for a release build.
BUILDTYPE ?= Release

all: ./build ./hydrad

./deps/gyp:
	git clone --depth 1 https://chromium.googlesource.com/external/gyp.git ./deps/gyp

./deps/libuv:
	git clone --depth 1 git://github.com/joyent/libuv.git ./deps/libuv

./deps/buffer:
	git clone --depth 1 git@github.com:clibs/buffer.git ./deps/buffer

./deps/yajl:
	git clone --depth 1 git@github.com:lloyd/yajl.git ./deps/yajl

./build: ./deps/gyp ./deps/libuv ./deps/buffer ./deps/yajl
	BUILDTYPE=${BUILDTYPE} deps/gyp/gyp --depth=. -Goutput_dir=./out -Icommon.gypi --generator-output=./build -Dlibrary=static_library -f make

./hydrad: src/hydrad.c src/util.h
	BUILDTYPE=${BUILDTYPE} make -C ./build/ hydrad
	cp ./build/out/${BUILDTYPE}/hydrad ./hydrad

distclean:
	make clean
	rm -rf ./build

clean:
	rm -rf ./build/out/${BUILDTYPE}/obj.target/hydrad/
	rm -f ./build/out/${BUILDTYPE}/hydrad
	rm -f ./hydrad

.PHONY: test
Window size: x 
Viewport size: x
