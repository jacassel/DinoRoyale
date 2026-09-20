"""Audit shader bindings and remove only the unused failed sprint material."""
import unreal,pathlib,json
root=pathlib.Path(unreal.Paths.project_dir())
E=unreal.EditorAssetLibrary
obsolete='/Game/Materials/M_NaturalFoliage'
refs=E.find_package_referencers_for_asset(obsolete,True) if E.does_asset_exist(obsolete) else []
assert not refs, f'Obsolete material still referenced: {refs}'
if E.does_asset_exist(obsolete):assert E.delete_asset(obsolete)
rows=[]
for name in ['SM_ConiferCanopy','SM_Grass','SM_Fern']:
    mesh=unreal.load_asset('/Game/World/'+name)
    slots=[s.material_interface.get_path_name() for s in mesh.get_editor_property('static_materials')]
    assert all('M_NaturalFoliageSurface' in s for s in slots), (name,slots)
    rows.append(dict(mesh=name,materials=slots))
for species in ['Trex','Raptor','Trike']:
    mesh=unreal.load_asset('/Game/Dinosaurs/'+species+'/'+species)
    slots=[s.material_interface.get_path_name() for s in mesh.get_editor_property('materials')]
    assert all('M_'+species+'_Modern' in s for s in slots), (species,slots)
    rows.append(dict(mesh=species,materials=slots))
(root/'Tests/Results/visual-modernization/final-asset-bindings.json').write_text(json.dumps(dict(bindings=rows,unusedMaterialRemoved=True),indent=2))
p=root/'Tests/Results/visual-modernization/environment-import.json'
report=json.loads(p.read_text());report['materials']=[s.replace('M_NaturalFoliage','M_NaturalFoliageSurface') if s=='M_NaturalFoliage' else s for s in report['materials']];p.write_text(json.dumps(report,indent=2))
