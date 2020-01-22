#def FlagsForFile(filename, **kwargs ):
#  return {
#    'flags': [ '-x','c++' ,'-Wall', '-Wextra', '-Werror' ,'-I./include/','-I','../PakkiUtils/', 
#    '-I','C:/VulkanSDK/1.1.85.0/Include']
#  }

# ,'-IC:/Users/Pate/Desktop/PakkiUtils']


import os
import ycm_core

flags = [
        '-Wall',
        '-Wextra',
        '-Werror',
        '-Wno-long-long',
        '-Wno-variadic-macros',
        '-fexceptions',
        '-ferror-limit=10000',
        '-DNDEBUG',
        '-DYCMIGNORE',
        '-std=c99',
        '-xc',
        '-isystem/usr/include/',
        '-I./external/include',
        '-I','/home/pate/Downloads/vulkan/1.1.126.0/x86_64/include/',
        '-D PAKKI_DEBUG'
        '-D_CRT_SECURE_NO_WARNINGS'
        ]

SOURCE_EXTENSIONS = [ '.cpp', '.cxx', '.cc', '.c', ]

def FlagsForFile( filename, **kwargs ):
    return {
            'flags': flags,
            'do_cache': True
            }


