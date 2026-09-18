import unreal
for kind in ['Trex','Raptor','Trike','Prey']:
    mesh=unreal.load_asset('/Game/Dinosaurs/'+kind+'/'+kind)
    unreal.log('INSPECT '+kind+' '+str(mesh.get_bounds()))
    for slot in mesh.get_editor_property('materials'):
        unreal.log('INSPECT_SLOT '+str(slot.get_editor_property('imported_material_slot_name'))+' '+str(slot.get_editor_property('material_interface')))
    data=mesh.get_editor_property('asset_import_data')
    unreal.log('INSPECT_IMPORT '+str(data))
for name in ['M_DinoSkin','M_Terrain','M_WorldVertex']:
    mat=unreal.load_asset('/Game/Materials/'+name)
    unreal.log('INSPECT_MATERIAL '+str(mat))
