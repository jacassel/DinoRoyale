import unreal
M=unreal.MaterialEditingLibrary
mat=unreal.load_asset('/Game/Materials/M_DinoSkin');M.delete_all_material_expressions(mat)
def n(kind,x,y,**args):
    a=M.create_material_expression(mat,getattr(unreal,'MaterialExpression'+kind),x,y)
    for k,v in args.items():a.set_editor_property(k,v)
    return a
def link(a,b,k,out=''):M.connect_material_expressions(a,out,b,k)
tint=n('VectorParameter',-600,0,parameter_name='BaseTint',default_value=unreal.LinearColor(.27,.35,.19,1))
uv=n('TextureCoordinate',-700,200,u_tiling=4,v_tiling=4)
tex=n('TextureSample',-480,200,texture=unreal.load_asset('/Game/Materials/Textures/Ground_Height'),sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR);link(uv,tex,'Coordinates')
mul=n('Multiply',-250,150,const_b=.65);link(tex,mul,'A','R');add=n('Add',-100,100,const_b=.65);link(mul,add,'A')
base=n('Multiply',100,0);link(tint,base,'A');link(add,base,'B');M.connect_material_property(base,'',unreal.MaterialProperty.MP_BASE_COLOR)
normal=n('TextureSample',-250,350,texture=unreal.load_asset('/Game/Materials/Textures/Scales_Normal'),sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL);link(uv,normal,'Coordinates');M.connect_material_property(normal,'RGB',unreal.MaterialProperty.MP_NORMAL)
rough=n('Constant',100,400,r=.82);M.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
mat.set_editor_property('used_with_skeletal_mesh',True);M.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat)
unreal.log('SKIN_MATERIAL_FIXED')
