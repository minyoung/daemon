#!/usr/bin/env python

from fabricate import *
import os

SOURCES = [
    'common.c',
    'config.c',
    'logging.c',
    'daemon.c',
    'main.c',
]

TESTS = {
    'test_common': {
        'test': ['test_common.c'],
        'src': ['common.c']
    },
    'test_config': {
        'test': ['test_config.c'],
        'src': ['config.c', 'common.c', 'logging.c']
    },
    'test_logging': {
        'test': ['test_logging.c'],
        'src': ['logging.c']
    },
    'test_daemon': {
        'test': ['test_daemon.c'],
        'src': ['daemon.c', 'config.c', 'common.c', 'logging.c']
    },
}

PATHS = {
    'bin': 'bin',
    'inc': 'inc',
    'src': 'src',
    'obj': 'obj',
    'test': 'tests',
    'lib': '.libs',
}

BINARY = 'daemon'
CC = 'gcc'
CFLAGS = ['-g']
INCLUDES = [PATHS['inc'], os.path.join(PATHS['lib'], 'inc')]
LIBRARIES = ['-L%s' % PATHS['lib'], '-lini']
DEFINES = []

INCLUDES = ['-I%s' % inc for inc in INCLUDES]

def object_name(src, prepend=''):
    return os.path.join(PATHS['obj'], prepend + src.replace('.c', '.o'))

def build():
    '''build main program binary'''
    objects = {}
    for src in SOURCES:
        objects[os.path.join(PATHS['src'], src)] = object_name(src)
    binary = os.path.join(PATHS['bin'], BINARY)
    build_binary(binary, objects)

def tests():
    '''build tests'''
    DEFINES.append('-DUNIT_TESTING')
    LIBRARIES.append('-lcmockery')
    for binary, files in TESTS.items():
        objects = {}
        for path, sources in files.items():
            for src in sources:
                objects[os.path.join(PATHS[path], src)] = object_name(src, '_')
        build_binary(os.path.join(PATHS['bin'], binary), objects)

def build_binary(binary, objects):
    compile(objects)
    link(binary, objects.values())

def compile(objects):
    for src, obj in objects.items():
        cmd = []
        cmd.append(CC)
        cmd.extend(CFLAGS)
        cmd.extend(INCLUDES)
        cmd.extend(DEFINES)
        cmd.append('-c')
        cmd.extend(['-o', obj])
        cmd.append(src)
        run(*cmd)

def link(binary, objects):
    cmd = []
    cmd.append(CC)
    cmd.extend(CFLAGS)
    cmd.extend(INCLUDES)
    cmd.extend(DEFINES)
    cmd.extend(['-o', binary])
    cmd.extend(objects)
    cmd.extend(LIBRARIES)
    run(*cmd)

def clean():
    autoclean()

main()
