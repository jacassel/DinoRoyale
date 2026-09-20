import pathlib,unreal
folder=pathlib.Path(unreal.Paths.project_dir()).resolve()/'Tools/Unreal'
for name in ['modernize_environment.py','import_modern_vegetation.py']:
    script=folder/name
    exec(compile(script.read_text(),str(script),'exec'),{'__name__':'__main__'})
    print('ENVIRONMENT_STAGE_OK',name,flush=True)
