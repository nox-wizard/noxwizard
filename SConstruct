mode_requested = ARGUMENTS.get('mode', 'debug')

if (mode_requested in ['debug', 'release']):

  print '**** Compiling in ' + mode_requested + 'mode...'

  debugflags     = '-g' # ['-g', '-D_DEBUG'] # ['-W1', '-GX', '-EHsc', '-D_DEBUG']
  debugdefines   = 'DEBUG'
  releaseflags   = '-O2' #['-O2', '-DNDEBUG']
  releasedefines = 'NDEBUG'

  env = Environment(CCFLAGS=['-w',
                             '-I./src',
                             '-fpermissive'],
                    LIBS='pthread')

  if mode_requested == 'debug':
    env.Append(CCFLAGS=debugflags)
    env.Append(CPPDEFINES=debugdefines)
  else:
    env.Append(CCFLAGS=releaseflags)
    env.Append(CPPDEFINES=releasedefines)

    SConscript('src/SConstruct',          # Load SConstruct from src-directory
               variant_dir='bin/' + mode_requested, # compile to bin/release
               duplicate=0,               # don't copy sources
               exports='env')             # export env-var
else:
  print "Error: expected 'debug' or 'release', found: " + mode_requested
  Exit(1)

