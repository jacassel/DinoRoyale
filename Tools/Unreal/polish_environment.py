"""Original terrain/water materials and updated pond map; no external art."""
import unreal,pathlib
A=unreal.AssetToolsHelpers.get_asset_tools();E=unreal.EditorAssetLibrary;M=unreal.MaterialEditingLibrary
root=pathlib.Path(unreal.Paths.project_dir())
for file,path in [(root/'Assets/Export/Textures/Water_Normal.png','/Game/Materials/Textures')]+[(p,'/Game/UI') for p in (root/'Assets/Export/UI').glob('*.png')]:
    t=unreal.AssetImportTask();t.filename=str(file);t.destination_path=path;t.automated=True;t.replace_existing=True;t.save=True;A.import_asset_tasks([t])
    tex=unreal.load_asset(path+'/'+file.stem)
    if file.stem.endswith('Normal'):tex.compression_settings=unreal.TextureCompressionSettings.TC_NORMALMAP;tex.srgb=False
    else:tex.lod_group=unreal.TextureGroup.TEXTUREGROUP_UI
    E.save_loaded_asset(tex)
def create(name):
    existing=unreal.load_asset('/Game/Materials/'+name)
    return existing or A.create_asset(name,'/Game/Materials',unreal.Material,unreal.MaterialFactoryNew())
def n(mat,typ,**props):
    x=M.create_material_expression(mat,getattr(unreal,'MaterialExpression'+typ))
    for k,v in props.items():x.set_editor_property(k,v)
    return x
def link(a,b,p,out=''):M.connect_material_expressions(a,out,b,p)
terrain=create('M_ValleyTerrain')
col=n(terrain,'VertexColor');boost=n(terrain,'Multiply',const_b=2.1);link(col,boost,'A','RGB')
pos=n(terrain,'WorldPosition');mask=n(terrain,'ComponentMask',r=True,g=True);link(pos,mask,'Input');uv=n(terrain,'Multiply',const_b=.002);link(mask,uv,'A')
tex=n(terrain,'TextureSample',texture=unreal.load_asset('/Game/Materials/Textures/Ground_Height'),sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR);link(uv,tex,'Coordinates')
shade=n(terrain,'Multiply',const_b=.40);link(tex,shade,'A','R');offset=n(terrain,'Add',const_b=.72);link(shade,offset,'A')
grass=n(terrain,'Constant3Vector',constant=unreal.LinearColor(.15,.235,.065,1));soil=n(terrain,'Constant3Vector',constant=unreal.LinearColor(.30,.20,.105,1))
height=n(terrain,'ComponentMask',b=True);link(pos,height,'Input');minus=n(terrain,'Subtract',const_b=450);link(height,minus,'A');divide=n(terrain,'Divide',const_b=1500);link(minus,divide,'A');clamp=n(terrain,'Clamp');link(divide,clamp,'Input')
biome=n(terrain,'LinearInterpolate');link(grass,biome,'A');link(soil,biome,'B');link(clamp,biome,'Alpha')
base=n(terrain,'Multiply');link(biome,base,'A');link(offset,base,'B');M.connect_material_property(base,'',unreal.MaterialProperty.MP_BASE_COLOR)
rough=n(terrain,'Constant',r=.90);M.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
M.recompile_material(terrain);E.save_loaded_asset(terrain)
water=create('M_ValleyWater');water.set_editor_property('blend_mode',unreal.BlendMode.BLEND_TRANSLUCENT);water.set_editor_property('two_sided',True)
water.set_editor_property('translucency_lighting_mode',unreal.TranslucencyLightingMode.TLM_SURFACE_PER_PIXEL_LIGHTING)
color=n(water,'Constant3Vector',constant=unreal.LinearColor(.028,.10,.075,1));M.connect_material_property(color,'',unreal.MaterialProperty.MP_BASE_COLOR)
rough=n(water,'Constant',r=.18);M.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
opacity=n(water,'DepthFade',fade_distance_default=110,opacity_default=.75);M.connect_material_property(opacity,'',unreal.MaterialProperty.MP_OPACITY)
pos=n(water,'WorldPosition');mask=n(water,'ComponentMask',r=True,g=True);link(pos,mask,'Input');uv=n(water,'Multiply',const_b=.00035);link(mask,uv,'A');pan=n(water,'Panner',speed_x=.012,speed_y=.008);link(uv,pan,'Coordinate')
normal=n(water,'TextureSample',texture=unreal.load_asset('/Game/Materials/Textures/Water_Normal'),sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL);link(pan,normal,'Coordinates');M.connect_material_property(normal,'RGB',unreal.MaterialProperty.MP_NORMAL)
M.recompile_material(water);E.save_loaded_asset(water)
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True,True);unreal.log('ENVIRONMENT_POLISH_SUCCESS')
