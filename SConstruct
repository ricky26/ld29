#-*- Mode: Python -*-#

Env = Environment(
    CXX='clang++',
    CC='clang')
Env.ParseConfig('pkg-config --libs --cflags sdl2')
Env.Append(CPPFLAGS=['-std=gnu++11'])
Env.Append(LIBS=['GL', 'Box2D'])

Env.SConscript([
    'SConscript',
    ], ['Env'])
