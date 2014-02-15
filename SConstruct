env = Environment(CCFLAGS=['-w',
                           '-I./src',
                           '-fpermissive'],
                  LIBS='pthread')

SConscript('src/SConstruct', variant_dir='bin/release', exports='env')
