"""Import original environment assets and author the runtime materials."""
import unreal,pathlib
ROOT=pathlib.Path(unreal.Paths.project_dir()).resolve();A=unreal.AssetToolsHelpers.get_asset_tools();E=unreal.EditorAssetLibrary;M=unreal.MaterialEditingLibrary
unreal.SystemLibrary.execute_console_command(None,'Interchange.FeatureFlags.Import.FBX 0')
def node(mat,typ,x=0,y=0,**props):
    n=M.create_material_expression(mat,getattr(unreal,'MaterialExpression'+typ),x,y)
    for k,v in props.items():n.set_editor_property(k,v)
    return n
def link(a,b,p,out=''):M.connect_material_expressions(a,out,b,p)
def material(name):
    mat=unreal.load_asset('/Game/Materials/'+name)
    if not mat:mat=A.create_asset(name,'/Game/Materials',unreal.Material,unreal.MaterialFactoryNew())
    M.delete_all_material_expressions(mat);return mat
def done(mat):M.recompile_material(mat);E.save_loaded_asset(mat)
textures={}
for file in (ROOT/'Assets/Export/Textures').glob('*.png'):
    t=unreal.AssetImportTask();t.filename=str(file);t.destination_path='/Game/Materials/Textures';t.automated=True;t.replace_existing=True;t.save=True;A.import_asset_tasks([t])
    tex=unreal.load_asset('/Game/Materials/Textures/'+file.stem)
    if file.stem.endswith('Normal'):tex.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_NORMALMAP);tex.set_editor_property('srgb',False)
    else:tex.set_editor_property('srgb',False)
    E.save_loaded_asset(tex);textures[file.stem]=tex
for name in ['M_WorldVertex','M_Terrain','M_DinoSkin']:
    mat=material(name);mat.set_editor_property('two_sided',name=='M_WorldVertex');mat.set_editor_property('used_with_skeletal_mesh',name=='M_DinoSkin')
    if name=='M_WorldVertex':mat.set_editor_property('used_with_instanced_static_meshes',True)
    color=node(mat,'VertexColor',-800,0)
    M.connect_material_property(color,'RGB',unreal.MaterialProperty.MP_BASE_COLOR)
    uv=node(mat,'TextureCoordinate',-850,300,u_tiling=5 if name=='M_DinoSkin' else 1,v_tiling=5 if name=='M_DinoSkin' else 1)
    key='Scales' if name=='M_DinoSkin' else 'Ground'
    if name=='M_Terrain':
        # World position keeps grain at a constant scale across the 1.15 km terrain.
        pos=node(mat,'WorldPosition',-1100,300);mask=node(mat,'ComponentMask',-950,300,r=True,g=True)
        link(pos,mask,'Input');mul=node(mat,'Multiply',-800,300,const_b=.004);link(mask,mul,'A');uv=mul
        height=node(mat,'TextureSample',-580,20,texture=textures['Ground_Height'],sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR);link(uv,height,'Coordinates')
        mul2=node(mat,'Multiply',-340,0,const_b=.65);link(height,mul2,'A','R')
        add=node(mat,'Add',-160,0,const_b=.60);link(mul2,add,'A')
        final=node(mat,'Multiply',0,0);link(color,final,'A','RGB');link(add,final,'B');M.connect_material_property(final,'',unreal.MaterialProperty.MP_BASE_COLOR)
    normal=node(mat,'TextureSample',-500,300,texture=textures[key+'_Normal'],sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL);link(uv,normal,'Coordinates');M.connect_material_property(normal,'RGB',unreal.MaterialProperty.MP_NORMAL)
    rough=node(mat,'Constant',-200,500,r=.66 if name=='M_DinoSkin' else .88);M.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS);done(mat)
water=material('M_Water')
color=node(water,'Constant3Vector',constant=unreal.LinearColor(.018,.135,.16,1));M.connect_material_property(color,'',unreal.MaterialProperty.MP_BASE_COLOR)
rough=node(water,'Constant',r=.21);M.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
uv=node(water,'TextureCoordinate',-600,100,u_tiling=1,v_tiling=3);pan=node(water,'Panner',-400,100,speed_x=.02,speed_y=.013);link(uv,pan,'Coordinate')
normal=node(water,'TextureSample',-150,100,texture=textures['Ground_Normal'],sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL);link(pan,normal,'Coordinates');M.connect_material_property(normal,'RGB',unreal.MaterialProperty.MP_NORMAL);done(water)
for file in (ROOT/'Assets/Export/World').glob('*.fbx'):
    t=unreal.AssetImportTask();t.filename=str(file);t.destination_path='/Game/World';t.destination_name=file.stem;t.automated=True;t.replace_existing=True;t.save=True;t.factory=unreal.FbxFactory()
    o=unreal.FbxImportUI();o.automated_import_should_detect_type=False;o.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH;o.import_mesh=True;o.import_as_skeletal=False;o.import_materials=False;o.import_textures=False
    o.static_mesh_import_data.set_editor_property('combine_meshes',True);o.static_mesh_import_data.set_editor_property('auto_generate_collision',file.stem=='SM_Boulder');o.static_mesh_import_data.set_editor_property('vertex_color_import_option',unreal.VertexColorImportOption.REPLACE)
    t.options=o;A.import_asset_tasks([t]);mesh=unreal.load_asset('/Game/World/'+file.stem);mesh.set_material(0,unreal.load_asset('/Game/Materials/M_WorldVertex'));E.save_loaded_asset(mesh)
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True,True)
unreal.log('WORLD_IMPORT_SUCCESS')
