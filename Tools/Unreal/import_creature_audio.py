"""Import the finite PCM source clips as native, cooked Unreal sound assets."""
import unreal, pathlib, json
root=pathlib.Path(unreal.Paths.project_dir()).resolve()
tasks=[]
for p in sorted((root/'Assets/Audio/Designed').glob('*.wav')):
    t=unreal.AssetImportTask();t.filename=str(p);t.destination_path='/Game/Audio/Creatures'
    t.automated=True;t.replace_existing=True;t.save=True
    tasks.append(t)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
rows=[]
for t in tasks:
    p=pathlib.Path(t.filename)
    a=unreal.load_asset('/Game/Audio/Creatures/'+p.stem)
    assert a, p.name
    a.set_editor_property('looping',False)
    a.set_editor_property('compression_quality',85)
    a.set_editor_property('loading_behavior',unreal.SoundWaveLoadingBehavior.FORCE_INLINE)
    unreal.EditorAssetLibrary.save_loaded_asset(a)
    rows.append(dict(name=p.stem,duration=a.get_editor_property('duration')))
assert len(rows)==81
(root/'Tests/Results/sound-terrain/audio-import.json').write_text(json.dumps(rows,indent=2))
print('CREATURE_AUDIO_IMPORT_COMPLETE',len(rows),flush=True)
