"""Rebuild the existing terrain graph with checked connections; preserve its palette."""
import unreal
M=unreal.MaterialEditingLibrary
mat=unreal.load_asset('/Game/Materials/M_ValleyTerrain')
assert mat
M.delete_all_material_expressions(mat)
def node(kind,**props):
    n=M.create_material_expression(mat,getattr(unreal,'MaterialExpression'+kind))
    for k,v in props.items(): n.set_editor_property(k,v)
    return n
def link(a,b,p='',out=''):
    if p in ('Input','Coordinates'): p=''
    assert M.connect_material_expressions(a,out,b,p), (a.get_name(),b.get_name(),p)
pos=node('WorldPosition');xy=node('ComponentMask',r=True,g=True);link(pos,xy,'Input')
uv=node('Multiply',const_b=.002);link(xy,uv,'A')
tex=node('TextureSample',texture=unreal.load_asset('/Game/Materials/Textures/Ground_Height'),sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR);link(uv,tex,'Coordinates')
shade=node('Multiply',const_b=.40);link(tex,shade,'A','R')
offset=node('Add',const_b=.72);link(shade,offset,'A')
height=node('ComponentMask',b=True);link(pos,height,'Input')
minus=node('Subtract',const_b=450);link(height,minus,'A')
divide=node('Divide',const_b=1500);link(minus,divide,'A')
sat=node('Saturate');link(divide,sat)
grass=node('Constant3Vector',constant=unreal.LinearColor(.15,.235,.065,1))
soil=node('Constant3Vector',constant=unreal.LinearColor(.30,.20,.105,1))
biome=node('LinearInterpolate');link(grass,biome,'A');link(soil,biome,'B');link(sat,biome,'Alpha')
base=node('Multiply');link(biome,base,'A');link(offset,base,'B')
assert M.connect_material_property(base,'',unreal.MaterialProperty.MP_BASE_COLOR)
rough=node('Constant',r=.9)
assert M.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
M.recompile_material(mat)
assert unreal.EditorAssetLibrary.save_loaded_asset(mat)
unreal.log('TERRAIN_COOK_REPAIR_SUCCESS')
