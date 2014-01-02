
# Specify BUILDTYPE=Release on the command line for a release build.
BUILDTYPE ?= Release

all: ./deps ./build ./hydrad

./deps:
	git submodule init

./build: ./deps
	BUILDTYPE=${BUILDTYPE} deps/gyp/gyp --depth=. -Goutput_dir=./out -Icommon.gypi --generator-output=./build -Dlibrary=static_library -f make

./hydrad: src/hydrad.c src/util.h
	BUILDTYPE=${BUILDTYPE} make -C ./build/ hydrad
	cp ./build/out/${BUILDTYPE}/hydrad ./hydrad

distclean:
	rm -rf ./build
	rm -f ./hydrad

clean:
	rm -rf ./build/out/${BUILDTYPE}/obj.target/hydrad/
	rm -f ./build/out/${BUILDTYPE}/hydrad
	rm -f ./hydrad

.PHONY: test
Window size: x 
Viewport size: x
