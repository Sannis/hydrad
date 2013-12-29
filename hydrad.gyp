{
  'includes': [ 'common.gypi' ],
  'targets': [
    {
      'target_name': 'hydrad',
      'type': 'executable',
      'sources': [
        'src/hydrad.c'
      ],
      'dependencies': [
        './deps/libuv/uv.gyp:libuv'
      ],
    }
  ],
}
