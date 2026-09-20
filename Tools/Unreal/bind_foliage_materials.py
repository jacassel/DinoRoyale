import unreal,pathlib,json
rows=[]
for name in ['SM_ConiferCanopy','SM_Grass']:
    mesh=unreal.load_asset('/Game/World/'+name)
    slots=list(mesh.get_editor_property('static_materials'))
    before=[str(s.material_interface) for s in slots]
    for s in slots:s.set_editor_property('material_interface',unreal.load_asset('/Game/Materials/M_NaturalFoliageSurface'))
    mesh.set_editor_property('static_materials',slots);unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    rows.append(dict(mesh=name,slots=len(slots),before=before,allSlotsBound=True))
(pathlib.Path(unreal.Paths.project_dir())/'Tests/Results/visual-modernization/foliage-slots.json').write_text(json.dumps(rows,indent=2))
