"""Retain original LOD0 and collision; add progressively cheaper distant mesh LODs."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());rows=[]
sub=unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem) or unreal.StaticMeshEditorSubsystem()
for name,levels in [
    ('SM_ConiferCanopy',[(1,1),(.35,.7),(.12,.35),(.03,.15),(.008,.06)]),
    ('SM_ConiferTrunk',[(1,1),(.5,.35),(.20,.12),(.08,.04)]),
    ('SM_Grass',[(1,1),(.40,.25),(.15,.09)]),
    ('SM_Fern',[(1,1),(.40,.4),(.12,.15),(.05,.05)])]:
    mesh=unreal.load_asset('/Game/World/'+name);before=[sub.get_number_verts(mesh,i) for i in range(sub.get_lod_count(mesh))]
    options=unreal.StaticMeshReductionOptions();options.set_editor_property('auto_compute_lod_screen_size',False)
    settings=[]
    for triangles,screen in levels:
        setting=unreal.StaticMeshReductionSettings();setting.set_editor_property('percent_triangles',triangles);setting.set_editor_property('screen_size',screen);settings.append(setting)
    options.set_editor_property('reduction_settings',settings)
    result=sub.set_lods(mesh,options);assert result>0,(name,result)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    after=[sub.get_number_verts(mesh,i) for i in range(sub.get_lod_count(mesh))]
    assert before[0]==after[0],(name,'LOD0 changed')
    rows.append(dict(mesh=name,before=before,after=after,screenSizes=list(sub.get_lod_screen_sizes(mesh))))
(root/'Tests/Results/alpha03/foliage-lods.json').write_text(json.dumps(rows,indent=2))
unreal.log('FOLIAGE_LODS_COMPLETE '+str(rows))
