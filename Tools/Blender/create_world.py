"""Original low-cost prehistoric vegetation and rock meshes. All units are centimetres."""
import bpy, math, random, pathlib, sys
from mathutils import Vector
ROOT=pathlib.Path(__file__).resolve().parents[2]
SRC=ROOT/'Assets/Source/World';OUT=ROOT/'Assets/Export/World'
SRC.mkdir(parents=True,exist_ok=True);OUT.mkdir(parents=True,exist_ok=True)
random.seed(771)
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
bpy.context.scene.unit_settings.system='METRIC';bpy.context.scene.unit_settings.scale_length=.01
mat=bpy.data.materials.new('World_Vertex');mat.use_nodes=True
nodes=mat.node_tree.nodes;bsdf=nodes.get('Principled BSDF');col=nodes.new('ShaderNodeVertexColor');col.layer_name='Color'
mat.node_tree.links.new(col.outputs['Color'],bsdf.inputs['Base Color']);bsdf.inputs['Roughness'].default_value=.83
V=[];F=[];C=[]
def vert(p,c):
    V.append(tuple(p));C.append((*c,1));return len(V)-1
def tri(a,b,c):F.append((a,b,c))
def tube(a,b,r1,r2,color,sides=9):
    a=Vector(a);b=Vector(b);d=(b-a).normalized();u=d.cross(Vector((0,1,0))).normalized()
    if u.length<.1:u=d.cross(Vector((1,0,0))).normalized()
    w=d.cross(u);rings=[]
    for p,r in [(a,r1),(b,r2)]:
        rings.append([vert(p+(u*math.cos(i*math.tau/sides)+w*math.sin(i*math.tau/sides))*r,tuple(x*random.uniform(.86,1.1) for x in color)) for i in range(sides)])
    for i in range(sides):j=(i+1)%sides;F.append((rings[0][i],rings[0][j],rings[1][j],rings[1][i]))
    F.append(tuple(reversed(rings[0])));F.append(tuple(rings[1]))
def leaf(a,b,width,color,curve=8):
    a=Vector(a);b=Vector(b);d=b-a;side=d.cross(Vector((0,0,1))).normalized()*width
    mid=a+d*.52+Vector((0,0,curve))
    ids=[vert(a,color),vert(mid+side,tuple(x*.82 for x in color)),vert(b,color),vert(mid-side,color),vert(mid+Vector((0,0,width*.16)),tuple(x*1.12 for x in color))]
    for i,j in [(0,1),(1,2),(2,3),(3,0)]:tri(ids[i],ids[j],ids[4])
def save(name):
    global V,F,C
    mesh=bpy.data.meshes.new(name);mesh.from_pydata(V,[],F);mesh.update();obj=bpy.data.objects.new(name,mesh);bpy.context.collection.objects.link(obj)
    mesh.materials.append(mat);a=mesh.color_attributes.new(name='Color',type='FLOAT_COLOR',domain='POINT')
    for i,c in enumerate(C):a.data[i].color=c
    for p in mesh.polygons:p.use_smooth=True
    bpy.ops.object.select_all(action='DESELECT');obj.select_set(True);bpy.context.view_layer.objects.active=obj
    bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.uv.smart_project();bpy.ops.object.mode_set(mode='OBJECT')
    collider=None
    if name=='SM_ConiferTrunk':
        bpy.ops.mesh.primitive_cylinder_add(vertices=10,radius=82,depth=1960,location=(0,0,980))
        collider=bpy.context.object;collider.name='UCX_SM_ConiferTrunk_00'
        obj.select_set(True)
    bpy.ops.export_scene.fbx(filepath=str(OUT/(name+'.fbx')),use_selection=True,object_types={'MESH'},axis_forward='-Y',axis_up='Z',apply_unit_scale=True,add_leaf_bones=False,bake_anim=False)
    if collider:collider.hide_set(True)
    obj.hide_set(True);V=[];F=[];C=[]
# Conifer bark and branches, narrow collision trunk (radius <= 82 cm).
bark=(.17,.105,.059)
for i in range(8):tube((0,0,i*245),(7*math.sin(i),5*math.cos(i),(i+1)*245),68-i*7.6,61-i*7.6,bark,12)
for tier in range(6):
    z=530+tier*215;r=440-tier*55
    for k in range(7):
        angle=k*math.tau/7+tier*.7;end=(r*math.cos(angle),r*math.sin(angle),z+40)
        tube((0,0,z),end,10,2,bark,6)
save('SM_ConiferTrunk')
# Radial sprays, with open gaps between needle branches.
for tier in range(8):
    z=530+tier*175;r=480-tier*51
    for k in range(9):
        angle=k*math.tau/9+tier*.72;d=Vector((math.cos(angle),math.sin(angle),.10));side=Vector((-math.sin(angle),math.cos(angle),0))
        base=Vector((0,0,z));color=(.065+.007*tier,.16+.005*tier,.085+.004*tier)
        for j in range(7):
            p=base+d*r*(j/8);length=r*(.35-.028*j)
            for sign in [-1,1]:leaf(p,p+d*length*.65+side*length*sign+Vector((0,0,-30)),length*.3,color,16)
save('SM_ConiferCanopy')
# Weathered original boulder, flattened buried base.
bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=3,radius=1)
obj=bpy.context.object
for v in obj.data.vertices:
    p=v.co;noise=1+.11*math.sin(p.x*13+p.y*9)+.05*math.cos(p.z*19)
    p.x*=235*noise;p.y*=200*noise;p.z=max(-40,p.z*215*noise+170)
V=[tuple(v.co) for v in obj.data.vertices];F=[tuple(p.vertices) for p in obj.data.polygons]
C=[(.235+.025*math.sin(p[2]/50),.19+.025*math.sin(p[0]/40),.13+.018*math.sin(p[1]/40),1) for p in V]
bpy.data.objects.remove(obj,do_unlink=True);save('SM_Boulder')
# Cycad/fern rosette with individual tapered leaflets.
tube((0,0,0),(0,0,58),17,10,(.22,.17,.065))
for k in range(12):
    angle=k*math.tau/12;d=Vector((math.cos(angle),math.sin(angle),0));side=Vector((-d.y,d.x,0))
    base=Vector((0,0,45));length=random.uniform(150,210);prev=base
    for j in range(1,11):
        t=j/10;p=base+d*(length*t)+Vector((0,0,100*math.sin(t*2.3)-40*t))
        tube(prev,p,3*(1-t)+.5,3*(1-t)+.3,(.15,.24,.065),5)
        width=length*.24*math.sin(t*math.pi)
        for sign in [-1,1]:leaf(p,p+side*width*sign-d*width*.23+Vector((0,0,-5)),width*.18,(.13,.28,.055),4)
        prev=p
save('SM_Fern')
# Small blade clumps, no collision or shadows in the game.
for k in range(60):
    x=random.uniform(-120,120);y=random.uniform(-120,120);a=random.uniform(0,math.tau);h=random.uniform(35,125)
    leaf((x,y,0),(x+math.cos(a)*h*.45,y+math.sin(a)*h*.45,h),random.uniform(3,7),(.23,.31,.085),4)
save('SM_Grass')
for obj in bpy.context.scene.objects:obj.hide_set(False)
bpy.ops.wm.save_as_mainfile(filepath=str(SRC/'LostValleyVegetation.blend'))
print('WORLD_ASSETS_CREATED')
