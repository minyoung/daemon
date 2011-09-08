#!/usr/bin/env python

from fabricate import *
import glob
import os


def find_files(base, globbing):
    files = glob.glob("%s/%s" % (base, globbing))
    base_len = len(base)+1
    return [path[base_len:] for path in files]

def object_name(src, prepend=''):
    return os.path.join(PATHS['obj'], prepend + src.replace('.c', '.o'))


PATHS = {
    'bin': 'bin',
    'inc': 'inc',
    'src': 'src',
    'obj': 'obj',
    'test': 'tests',
    'lib': '.libs',
}

SOURCES = find_files(PATHS['src'], '*.c')
TEST_FILES = find_files(PATHS['test'], 'check_*.c')

BINARY = 'daemon'
CC = 'clang'
CFLAGS = ['-g']
CFLAGS.append('-Wno-implicit-function-declaration')
INCLUDES = [PATHS['inc'], os.path.join(PATHS['lib'], 'inc')]
LIBRARIES = ['-L%s' % PATHS['lib'], '-lini', '-lpthread', '-lssl']
DEFINES = []

INCLUDES = ['-I%s' % inc for inc in INCLUDES]


def all():
    build()
    tests()

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
    LIBRARIES.append('-lcheck')
    objects = {}
    for src in SOURCES:
        if src == 'main.c':
            continue
        objects[os.path.join(PATHS['src'], src)] = object_name(src, '_')
    for test in TEST_FILES:
        objects[os.path.join(PATHS['test'], test)] = object_name(test, '_')
        build_binary(os.path.join(PATHS['bin'], test[:-2]), objects)
        del objects[os.path.join(PATHS['test'], test)]

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
