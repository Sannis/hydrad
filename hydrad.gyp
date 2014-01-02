{
  'includes': [ 'common.gypi' ],
  'targets': [
    {
      'target_name': 'hydrad',
      'type': 'executable',
      'sources': [
        'src/hydrad.c',
        'deps/buffer/buffer.c',
      ],
      'include_dirs': [
        'deps/buffer',
      ],
      'dependencies': [
        'deps/libuv/uv.gyp:libuv',
        'deps/yajl.gyp:yajl',
      ],
    }
  ],
}
