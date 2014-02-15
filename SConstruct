mode_requested = ARGUMENTS.get('mode', 'debug')

if not (mode_requested in ['debug', 'release']):
  print "Error: expected 'debug' or 'release', found: " + mode_requested
  Exit(1)

print '**** Compiling in ' + mode_requested + 'mode...'

debugflags   = ['-g', '-D_DEBUG'] # ['-W1', '-GX', '-EHsc', '-D_DEBUG']
releaseflags = ['-O2', '-DNDEBUG']

env = Environment(CCFLAGS=['-w',
                           '-I./src',
                           '-fpermissive'],
                  LIBS='pthread')

if mode_requested == 'debug':
  env.Append(CCFLAGS=debugflags)
else:
  env.Append(CCFLAGS=releaseflags)

SConscript('src/SConstruct',          # Load SConstruct from src-directory
           variant_dir='bin/' + mode_requested, # compile to bin/release
           duplicate=0,               # don't copy sources
           exports='env')             # export env-var
