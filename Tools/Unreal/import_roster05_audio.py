import unreal,pathlib,json
root=pathlib.Path(unreal.Paths.project_dir()).resolve();tasks=[]
for p in sorted((root/'Assets/Audio/Designed/Roster05').glob('*.wav')):
 t=unreal.AssetImportTask();t.filename=str(p);t.destination_path='/Game/Audio/Creatures';t.automated=True;t.replace_existing=True;t.save=True;tasks.append(t)
assert len(tasks)==81
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
rows=[]
for t in tasks:
 name=pathlib.Path(t.filename).stem;a=unreal.load_asset('/Game/Audio/Creatures/'+name);assert a
 a.set_editor_property('looping',False);a.set_editor_property('compression_quality',85);a.set_editor_property('loading_behavior',unreal.SoundWaveLoadingBehavior.FORCE_INLINE);unreal.EditorAssetLibrary.save_loaded_asset(a)
 rows.append(dict(name=name,duration=a.get_editor_property('duration')))
out=root/'Tests/Results/roster05';out.mkdir(parents=True,exist_ok=True);(out/'audio-import.json').write_text(json.dumps(rows,indent=2));print('ROSTER05_AUDIO_IMPORTED',len(rows))
