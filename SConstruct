env = Environment(CCFLAGS=['-w',
                           '-I./src',
                           '-fpermissive'],
                  LIBS='pthread')

SConscript('src/SConstruct', exports='env')
