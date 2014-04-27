#-*- Mode: Python -*-#

Env = Environment(
    CXX='clang++',
    CC='clang')
Env.ParseConfig('pkg-config --libs --cflags sdl2')
Env.Append(CPPFLAGS=['-std=gnu++11', '-g'])
Env.Append(LIBS=['GL', 'Box2D', 'SDL2_image'])

Env.SConscript([
    'SConscript',
    ], ['Env'])
