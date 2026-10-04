"""Read-only mesh LOD audit for the measured performance pass."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());rows=[]
sub=unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem) or unreal.StaticMeshEditorSubsystem()
for name in ['SM_ConiferCanopy','SM_ConiferTrunk','SM_Grass','SM_Fern','SM_Boulder']:
    mesh=unreal.load_asset('/Game/World/'+name)
    count=sub.get_lod_count(mesh)
    rows.append(dict(mesh=name,lods=count,vertices=[sub.get_number_verts(mesh,i) for i in range(count)],materials=len(mesh.get_editor_property('static_materials'))))
(root/'Tests/Results/alpha03/foliage-before.json').write_text(json.dumps(rows,indent=2))
unreal.log(str(rows))
