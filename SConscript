Import('*')

Env.VariantDir('build', 'src', duplicate=False)

src = [
    'build/main.cpp',
    'build/window.cpp',
    'build/event_dispatch.cpp',
    
    'build/physics.cpp',

    'build/game.cpp',
    'build/renderable.cpp',
    'build/game_entity.cpp',
]

Env.Program('out/bin/ld29', 
            src)
