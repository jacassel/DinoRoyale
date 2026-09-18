import unreal
lookup={'Dino_Skin':'M_DinoSkin','Dino_Horn':'M_Horn','Dino_Claw':'M_Claw','Dino_Eye':'M_Eye','Dino_Pupil':'M_Pupil','Dino_Tooth':'M_Tooth','Dino_Mouth':'M_Mouth'}
for name in ['Trex','Raptor','Trike','Prey']:
    mesh=unreal.load_asset('/Game/Dinosaurs/'+name+'/'+name)
    mats=list(mesh.get_editor_property('materials'))
    for slot in mats:
        source=str(slot.get_editor_property('imported_material_slot_name'))
        slot.set_editor_property('material_interface',unreal.load_asset('/Game/Materials/'+lookup[source]))
    mesh.set_editor_property('materials',mats)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    for slot in mesh.get_editor_property('materials'):
        assert slot.get_editor_property('material_interface') is not None
unreal.log('MATERIAL_ASSIGNMENTS_VERIFIED')
