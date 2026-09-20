"""Original terrain, wet banks, layered water and differentiated vegetation surfaces."""
import unreal,pathlib,json
root=pathlib.Path(unreal.Paths.project_dir()).resolve()
A=unreal.AssetToolsHelpers.get_asset_tools();E=unreal.EditorAssetLibrary;M=unreal.MaterialEditingLibrary
textures={}
for file in (root/'Assets/Export/NaturalSurfaces').glob('*.png'):
    t=unreal.AssetImportTask();t.filename=str(file);t.destination_path='/Game/Materials/Natural';t.automated=True;t.replace_existing=True;t.save=True;A.import_asset_tasks([t])
    tex=unreal.load_asset('/Game/Materials/Natural/'+file.stem)
    tex.set_editor_property('srgb',file.stem.endswith('BaseColor'))
    if file.stem.endswith('Normal'):tex.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_NORMALMAP)
    elif file.stem.endswith('Masks'):tex.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_MASKS)
    E.save_loaded_asset(tex);textures[file.stem]=tex

def mat(name):
    unreal.log_warning('BUILD_MATERIAL '+name)
    m=unreal.load_asset('/Game/Materials/'+name) or A.create_asset(name,'/Game/Materials',unreal.Material,unreal.MaterialFactoryNew())
    # Loaded instanced foliage can root expression objects in this UE version.
    # Rewire that graph without deleting those objects; unused branches do not compile.
    if name!='M_NaturalFoliageSurface':
        for old in list(M.get_material_expressions(m)):M.delete_material_expression(m,old)
    return m
def n(m,typ,**props):
    a=M.create_material_expression(m,getattr(unreal,'MaterialExpression'+typ))
    for k,v in props.items():a.set_editor_property(k,v)
    return a
def link(a,b,p,out=''):
    if not out and isinstance(a,unreal.MaterialExpressionWorldPosition):out='XYZ'
    if p in ('Input','VectorInput','Coordinates','Coordinate'):p=''
    assert M.connect_material_expressions(a,out,b,p), (a.get_class().get_name(),out,b.get_class().get_name(),p)
def prop(a,key,out=''):M.connect_material_property(a,out,getattr(unreal.MaterialProperty,'MP_'+key))
def const(m,x):return n(m,'Constant',r=x)
def color(m,c):return n(m,'Constant3Vector',constant=unreal.LinearColor(*c,1))
def op(m,typ,a,b=None,**p):
    o=n(m,typ,**p);link(a,o,'A')
    if b is not None:link(b,o,'B')
    return o
def lerp(m,a,b,f):
    o=n(m,'LinearInterpolate');link(a,o,'A');link(b,o,'B');link(f,o,'Alpha');return o
def mask(m,a,channels='r'):
    o=n(m,'ComponentMask',r='r' in channels,g='g' in channels,b='b' in channels);link(a,o,'Input');return o
def tex(m,name,uv):
    kind='NORMAL' if name.endswith('Normal') else 'MASKS' if name.endswith('Masks') else 'COLOR'
    o=n(m,'TextureSample',texture=textures[name],sampler_type=getattr(unreal.MaterialSamplerType,'SAMPLERTYPE_'+kind));link(uv,o,'Coordinates');return o
def uv(m,scale,channels='rg'):
    pos=n(m,'WorldPosition');return op(m,'Multiply',mask(m,pos,channels),const_b=scale)
def done(m):M.recompile_material(m);E.save_loaded_asset(m)

ground=mat('M_ValleyTerrain')
fine=tex(ground,'Soil_Masks',uv(ground,.0032))
macro=tex(ground,'Soil_Masks',uv(ground,.00017))
biome=n(ground,'VertexColor')
base=op(ground,'Multiply',biome,const_b=.49)
patch=op(ground,'Multiply',mask(ground,macro),const_b=.60)
base=lerp(ground,base,color(ground,(.092,.064,.037)),patch)
detail=op(ground,'Add',op(ground,'Multiply',mask(ground,fine),const_b=.50),const_b=.70)
base=op(ground,'Multiply',base,detail)
# Match the existing analytic creek/pond only for material masks: no physics changes.
pos=n(ground,'WorldPosition')
bank=n(ground,'Custom',code='float creek=-5750+1300*sin(P.x/6000); float c=1-smoothstep(350,1050,abs(P.y-creek)); float pr=length(float2((P.x-3000)/2750,(P.y-4500)/1900)); float pond=1-smoothstep(1.00,1.18,pr); return saturate(max(c,pond));',output_type=unreal.CustomMaterialOutputType.CMOT_FLOAT1)
inp=unreal.CustomInput();inp.set_editor_property('input_name','P');bank.set_editor_property('inputs',[inp]);link(pos,bank,'P')
base=lerp(ground,base,color(ground,(.058,.043,.028)),bank);prop(base,'BASE_COLOR')
normal=tex(ground,'Soil_Normal',uv(ground,.0032));prop(normal,'NORMAL','RGB')
prop(lerp(ground,const(ground,.91),const(ground,.40),bank),'ROUGHNESS');prop(const(ground,.22),'SPECULAR');done(ground)

for asset,name,key,scale in [('SM_Boulder','M_NaturalRock','Rock',.0022),('SM_ConiferTrunk','M_NaturalBark','Bark',.0045)]:
    m=mat(name);m.set_editor_property('used_with_instanced_static_meshes',True)
    coords=uv(m,scale,'rb')
    c=tex(m,key+'_BaseColor',coords);prop(c,'BASE_COLOR','RGB');prop(tex(m,key+'_Normal',coords),'NORMAL','RGB');prop(tex(m,key+'_Masks',coords),'ROUGHNESS','G')
    prop(const(m,.24),'SPECULAR');done(m)
    mesh=unreal.load_asset('/Game/World/'+asset);mesh.set_material(0,m);E.save_loaded_asset(mesh)

foliage=mat('M_NaturalFoliageSurface');foliage.set_editor_property('two_sided',True);foliage.set_editor_property('used_with_instanced_static_meshes',True)
foliage.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_TWO_SIDED_FOLIAGE)
variation=tex(foliage,'Soil_Masks',uv(foliage,.002))
leafcolor=lerp(foliage,color(foliage,(.038,.070,.016)),color(foliage,(.12,.17,.045)),mask(foliage,variation))
prop(leafcolor,'BASE_COLOR');prop(color(foliage,(.09,.19,.035)),'SUBSURFACE_COLOR')
prop(const(foliage,.76),'ROUGHNESS');prop(const(foliage,.18),'SPECULAR');done(foliage)
for asset in ['SM_ConiferCanopy','SM_Fern','SM_Grass']:
    mesh=unreal.load_asset('/Game/World/'+asset);mesh.set_material(0,foliage);E.save_loaded_asset(mesh)

water=mat('M_ValleyWater');water.set_editor_property('blend_mode',unreal.BlendMode.BLEND_TRANSLUCENT);water.set_editor_property('two_sided',True)
water.set_editor_property('translucency_lighting_mode',unreal.TranslucencyLightingMode.TLM_SURFACE_PER_PIXEL_LIGHTING)
water.set_editor_property('screen_space_reflections',True)
depth=n(water,'DepthFade',fade_distance_default=420,opacity_default=1)
prop(lerp(water,color(water,(.065,.13,.105)),color(water,(.009,.044,.040)),depth),'BASE_COLOR')
fade=n(water,'DepthFade',fade_distance_default=95,opacity_default=1)
opacity=op(water,'Add',op(water,'Multiply',depth,const_b=.34),const_b=.56)
prop(op(water,'Multiply',opacity,fade),'OPACITY')
normals=[]
for scale,sx,sy in [(.00046,.016,.007),(.00105,-.009,.012)]:
    pan=n(water,'Panner',speed_x=sx,speed_y=sy);link(uv(water,scale),pan,'Coordinate');normals.append(tex(water,'Ripples_Normal',pan))
blend=n(water,'Add');link(normals[0],blend,'A','RGB');link(normals[1],blend,'B','RGB')
normalize=n(water,'Normalize');link(blend,normalize,'VectorInput');prop(normalize,'NORMAL')
prop(const(water,.13),'ROUGHNESS');prop(const(water,.62),'SPECULAR')
prop(const(water,1.018),'REFRACTION')
done(water)
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True,True)
(root/'Tests/Results/visual-modernization/environment-import.json').write_text(json.dumps(dict(materials=['M_ValleyTerrain','M_ValleyWater','M_NaturalRock','M_NaturalBark','M_NaturalFoliageSurface'],textures=list(textures)),indent=2))
