"""Import only new roster assets, leaving all prior dinosaurs intact."""
import unreal,pathlib,importlib.util
root=pathlib.Path(unreal.Paths.project_dir()).resolve()
spec=importlib.util.spec_from_file_location('dino_import',root/'Tools/Unreal/import_dinosaurs.py')
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
for kind in ['Anky','Pachy','Brachi']:
    folder=root/'Assets/Export/Roster05'
    if not (folder/(kind+'.fbx')).exists():continue
    mesh=m.import_species(kind,folder)
    for clip in ['Quick2','Quick3','Hit','PivotLeft','PivotRight']:
        m.imp(folder/(kind+'_'+clip+'.fbx'),kind+'_'+clip,mesh.get_editor_property('skeleton'),True)
    unreal.log('ROSTER05_IMPORTED '+kind)
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True,True)
