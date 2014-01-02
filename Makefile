
all: ./build ./hydrad

./deps/libuv:
	git clone --depth 1 git://github.com/joyent/libuv.git ./deps/libuv

./deps/gyp:
	git clone --depth 1 https://chromium.googlesource.com/external/gyp.git ./deps/gyp

./build: ./deps/gyp ./deps/libuv
	deps/gyp/gyp --depth=. -Goutput_dir=./out -Icommon.gypi --generator-output=./build -Dlibrary=static_library -f make

./hydrad: src/hydrad.c src/util.h
	make -C ./build/ hydrad
	cp ./build/out/Release/hydrad ./hydrad

distclean:
	make clean
	rm -rf ./build

clean:
	rm -rf ./build/out/Release/obj.target/hydrad/
	rm -f ./build/out/Release/hydrad
	rm -f ./hydrad

.PHONY: test
Window size: x 
Viewport size: x
