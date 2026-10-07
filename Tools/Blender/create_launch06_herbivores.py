"""Generate revised herbivores into a separate source/export checkpoint."""
import pathlib,importlib.util,sys
root=pathlib.Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('roster',root/'Tools/Blender/create_roster05.py')
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
m.OUT=root/'Assets/Source/Launch06';m.EXP=root/'Assets/Export/Launch06'
m.OUT.mkdir(parents=True,exist_ok=True);m.EXP.mkdir(parents=True,exist_ok=True)
for kind in (sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else ['Anky','Pachy','Brachi']):m.build(kind)
