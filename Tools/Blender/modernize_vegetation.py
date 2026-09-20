"""Original fuller conifer sprays and curved grass; retained placement and trunk collision."""
import bpy,math,random,pathlib
from mathutils import Vector
root=pathlib.Path(__file__).resolve().parents[2]
out=root/'Assets/Export/WorldModern';out.mkdir(parents=True,exist_ok=True)
src=root/'Assets/Source/WorldModern';src.mkdir(parents=True,exist_ok=True)
random.seed(93621)
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
bpy.context.scene.unit_settings.system='METRIC';bpy.context.scene.unit_settings.scale_length=.01
mat=bpy.data.materials.new('Natural_Foliage');mat.diffuse_color=(.06,.13,.045,1)
V=[];F=[];C=[]
def leaf(start,end,width,col,bend=10):
    a=Vector(start);b=Vector(end);d=b-a;side=d.cross(Vector((0,0,1))).normalized()
    if side.length<.1:side=Vector((1,0,0))
    off=len(V)
    for j in range(5):
        t=j/4;c=a+d*t+Vector((0,0,bend*math.sin(t*math.pi)))
        w=width*(.3+math.sin(t*math.pi))*(1-t*.8)
        for sign in [-1,1]:
            V.append(tuple(c+side*sign*w));C.append((*[v*(.78+.22*t) for v in col],1))
    for j in range(4):F.append((off+j*2,off+j*2+1,off+j*2+3,off+j*2+2))
def save(name):
    global V,F,C
    mesh=bpy.data.meshes.new(name);mesh.from_pydata(V,[],F);mesh.update()
    o=bpy.data.objects.new(name,mesh);bpy.context.collection.objects.link(o);mesh.materials.append(mat)
    colors=mesh.color_attributes.new(name='Color',type='FLOAT_COLOR',domain='POINT')
    for i,c in enumerate(C):colors.data[i].color=c
    for p in mesh.polygons:p.use_smooth=True
    bpy.ops.object.select_all(action='DESELECT');o.select_set(True);bpy.context.view_layer.objects.active=o
    bpy.ops.export_scene.fbx(filepath=str(out/(name+'.fbx')),use_selection=True,object_types={'MESH'},axis_forward='-Y',axis_up='Z',apply_unit_scale=True,mesh_smooth_type='FACE',bake_anim=False)
    V=[];F=[];C=[]
for tier in range(12):
    z=480+tier*113;r=(1-tier/14)*490
    for branch in range(9):
        angle=branch*math.tau/9+tier*1.73+random.uniform(-.18,.18)
        d=Vector((math.cos(angle),math.sin(angle),0));side=Vector((-d.y,d.x,0))
        length=r*random.uniform(.75,1.12);base=Vector((0,0,z+random.uniform(-55,55)))
        for step in range(1,10):
            t=step/10;p=base+d*length*t+Vector((0,0,55*math.sin(t*math.pi)-t*70))
            spray=length*(.34-.20*t)
            for sign in [-1,1]:
                end=p+side*spray*sign+d*spray*.45+Vector((0,0,random.uniform(-28,22)))
                for k in range(5):
                    s=k/5;start=p.lerp(end,s)
                    tip=start+d*random.uniform(35,65)+side*sign*spray*(.75-s*.30)+Vector((0,0,random.uniform(-85,25)))
                    col=(random.uniform(.045,.10),random.uniform(.13,.23),random.uniform(.055,.10))
                    leaf(start,tip,random.uniform(13.0,22.0),col,random.uniform(10,28))
save('SM_ConiferCanopy')
for k in range(62):
    theta=random.uniform(0,math.tau);radius=random.uniform(0,65);h=random.uniform(25,92)
    a=Vector((math.cos(theta)*radius,math.sin(theta)*radius,0))
    end=a+Vector((math.cos(theta)*h*.52,math.sin(theta)*h*.52,h*.70))
    col=random.choice([(.17,.24,.075),(.20,.235,.10),(.11,.18,.050),(.29,.27,.13)])
    leaf(a,end,random.uniform(.8,2.1),col,h*.22)
save('SM_Grass')
bpy.ops.wm.save_as_mainfile(filepath=str(src/'NaturalVegetation.blend'))
print('NATURAL_VEGETATION_CREATED',flush=True)
