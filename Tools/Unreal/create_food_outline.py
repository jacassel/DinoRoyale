"""A restrained visible-silhouette cue for the Triceratops's edible plants."""
import unreal
M = unreal.MaterialEditingLibrary
path = '/Game/Materials/M_EdibleOutline'
mat = unreal.load_asset(path) or unreal.AssetToolsHelpers.get_asset_tools().create_asset(
    'M_EdibleOutline', '/Game/Materials', unreal.Material, unreal.MaterialFactoryNew())
mat.set_editor_property('material_domain', unreal.MaterialDomain.MD_POST_PROCESS)
mat.set_editor_property('blendable_location', unreal.BlendableLocation.BL_SCENE_COLOR_AFTER_TONEMAPPING)
M.delete_all_material_expressions(mat)
scene = M.create_material_expression(mat, unreal.MaterialExpressionSceneTexture)
scene.set_editor_property('scene_texture_id', unreal.SceneTextureId.PPI_POST_PROCESS_INPUT0)
stencil = M.create_material_expression(mat, unreal.MaterialExpressionSceneTexture)
stencil.set_editor_property('scene_texture_id', unreal.SceneTextureId.PPI_CUSTOM_STENCIL)
depth = M.create_material_expression(mat, unreal.MaterialExpressionSceneTexture)
depth.set_editor_property('scene_texture_id', unreal.SceneTextureId.PPI_CUSTOM_DEPTH)
custom = M.create_material_expression(mat, unreal.MaterialExpressionCustom)
custom.set_editor_property('output_type', unreal.CustomMaterialOutputType.CMOT_FLOAT3)
inputs=[]
for name in ['Scene', 'Stencil', 'Depth']:
    i=unreal.CustomInput();i.set_editor_property('input_name', name);inputs.append(i)
custom.set_editor_property('inputs', inputs)
custom.set_editor_property('code', r'''
float2 uv = GetDefaultSceneTextureUV(Parameters, 25);
float2 px = View.BufferSizeAndInvSize.zw;
float center = abs(Stencil.r - 2.0) < 0.1;
float edge = 0;
float halo = 0;
[unroll] for (int i=0; i<8; ++i) {
    float2 offset = float2((i%3)-1, (i/3)-1);
    if (i==4) offset=float2(1,1);
    float2 q=uv+offset*px*1.25;
    float mask=abs(SceneTextureLookup(q,25,false).r-2.0)<0.1;
    float visible=SceneTextureLookup(q,13,false).r <= SceneTextureLookup(q,1,false).r+2.0;
    edge=max(edge,mask*visible);
    q=uv+offset*px*2.5;
    mask=abs(SceneTextureLookup(q,25,false).r-2.0)<0.1;
    visible=SceneTextureLookup(q,13,false).r <= SceneTextureLookup(q,1,false).r+2.0;
    halo=max(halo,mask*visible);
}
float glow=(1-center)*(edge*0.40+halo*0.10);
return Scene.rgb + float3(0.55,0.85,0.16)*glow;
''')
for n,name in [(scene,'Scene'),(stencil,'Stencil'),(depth,'Depth')]:
    assert M.connect_material_expressions(n,'Color',custom,name)
assert M.connect_material_property(custom,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
M.recompile_material(mat)
assert unreal.EditorAssetLibrary.save_loaded_asset(mat)
unreal.log('EDIBLE_OUTLINE_CREATED')

