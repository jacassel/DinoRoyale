"""Keep close canopy polygons from filling the third-person camera in dense groves."""
import unreal
E=unreal.EditorAssetLibrary;M=unreal.MaterialEditingLibrary
mat=unreal.load_asset('/Game/Materials/M_NaturalFoliageSurface')
assert mat
if E.get_metadata_tag(mat,'DinoCameraClearance')!='1':
    depth=M.create_material_expression(mat,unreal.MaterialExpressionPixelDepth,250,1000)
    offset=M.create_material_expression(mat,unreal.MaterialExpressionSubtract,450,1000)
    near=M.create_material_expression(mat,unreal.MaterialExpressionConstant,250,1150);near.set_editor_property('r',80)
    scale=M.create_material_expression(mat,unreal.MaterialExpressionDivide,650,1000)
    span=M.create_material_expression(mat,unreal.MaterialExpressionConstant,450,1150);span.set_editor_property('r',180)
    assert M.connect_material_expressions(depth,'',offset,'A')
    assert M.connect_material_expressions(offset,'',scale,'A')
    assert M.connect_material_expressions(near,'',offset,'B')
    assert M.connect_material_expressions(span,'',scale,'B')
    assert M.connect_material_property(scale,'',unreal.MaterialProperty.MP_OPACITY_MASK)
    mat.set_editor_property('blend_mode',unreal.BlendMode.BLEND_MASKED)
    mat.set_editor_property('opacity_mask_clip_value',.333)
    E.set_metadata_tag(mat,'DinoCameraClearance','1')
    M.recompile_material(mat);E.save_loaded_asset(mat)
print('FOLIAGE_CAMERA_CLEARANCE_COMPLETE',flush=True)
