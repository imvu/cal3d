Import('*')

# Build Cal3D tests.

skipdebug = ARGUMENTS.get('skipdebug', '')
skiprelease = ARGUMENTS.get('skiprelease', '')

if not skipdebug:
    env = baseEnv.Copy(tools=[Debug])
    Export('env')
    SConscript('SConscript.library', build_dir='Debug', duplicate=0)

if not skiprelease:
    env = baseEnv.Copy(tools=[Release])
    Export('env')
    SConscript('SConscript.library', build_dir='Release', duplicate=0)
