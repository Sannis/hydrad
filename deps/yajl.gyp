{
  'variables': {
    'YAJL_MAJOR': '2',
    'YAJL_MINOR': '0',
    'YAJL_MICRO': '5',
  },
  'targets': [
    {
      'target_name': 'yajl',
      'type': 'static_library',
      'dependencies': ['copy_headers'],
      'sources': [
        'yajl/src/yajl.c',
        'yajl/src/yajl_alloc.c',
        'yajl/src/yajl_buf.c',
        'yajl/src/yajl_encode.c',
        'yajl/src/yajl_gen.c',
        'yajl/src/yajl_lex.c',
        'yajl/src/yajl_parser.c',
        'yajl/src/yajl_tree.c',
        'yajl/src/yajl_version.c',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../build/yajl_include/yajl/include/',
        ]
      },
      'include_dirs': [
        'yajl/src',
        '../build/yajl_include/yajl/include',
      ],
    },
    {
      'target_name': 'copy_headers',
      'type': 'none',
      'copies': [
        {
          'destination': '../build/yajl_include/yajl/include/yajl',
          'files': [
            'yajl/src/api/yajl_common.h',
            'yajl/src/api/yajl_gen.h',
            'yajl/src/api/yajl_parse.h',
            'yajl/src/api/yajl_tree.h',
          ]
        }
      ],
      'actions': [
        {
          'variables': {
            'replacements': [
              '{YAJL_MAJOR}:<(YAJL_MAJOR)',
              '{YAJL_MINOR}:<(YAJL_MINOR)',
              '{YAJL_MICRO}:<(YAJL_MICRO)',
            ]
          },
          'action_name': 'version_header',
          'inputs': [
            'yajl/src/api/yajl_version.h.cmake'
          ],
          'outputs': [
            '../build/yajl_include/yajl/include/yajl/yajl_version.h',
          ],
          'action': [
            'python',
            '../tools/lame_sed.py',
            '<@(_inputs)',
            '<@(_outputs)',
            '<@(replacements)',
          ],
        }
      ]
    }
  ]
}
