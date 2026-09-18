"""Original procedural dinosaur assets for Dinosaur Battle.

Blender 5.2: blender -b --python Tools/Blender/create_dinosaurs.py
All shapes, vertex skin colours, rigs, and actions are authored here; no downloaded art.
Centimetres, +X forward, +Z up. The exported skeleton has gameplay-friendly bone names.
"""
import bpy, bmesh, math, random, pathlib, json
from mathutils import Vector, Euler
from math import sin, cos, pi, exp

ROOT=pathlib.Path(__file__).resolve().parents[2]
OUT=ROOT/'Assets/Source/Dinosaurs'
EXPORT=ROOT/'Assets/Export/Dinosaurs'
OUT.mkdir(parents=True,exist_ok=True);EXPORT.mkdir(parents=True,exist_ok=True)
random.seed(42)

def material(name,color,rough=.65,metal=0):
    m=bpy.data.materials.get(name) or bpy.data.materials.new(name)
    m.diffuse_color=(*color,1);m.use_nodes=True
    p=m.node_tree.nodes.get('Principled BSDF');p.inputs['Base Color'].default_value=(*color,1)
    p.inputs['Roughness'].default_value=rough;p.inputs['Metallic'].default_value=metal
    if name=='Dino_Skin' and not m.node_tree.nodes.get('Skin_attribute'):
        col=m.node_tree.nodes.new('ShaderNodeVertexColor');col.name='Skin_attribute';col.layer_name='SkinColor'
        m.node_tree.links.new(col.outputs['Color'],p.inputs['Base Color'])
    return m

def palette(kind):
    if kind=='Trex':return (.24,.32,.18),(.59,.54,.32),(.095,.14,.082)
    if kind=='Raptor':return (.15,.30,.32),(.54,.59,.49),(.075,.105,.12)
    if kind=='Trike':return (.39,.25,.16),(.62,.52,.34),(.17,.11,.085)
    return (.33,.41,.17),(.61,.62,.32),(.15,.21,.08)

PARTS=[];BONES={};SPECIES=''
def weights(obj, bone):
    if bone:
        g=obj.vertex_groups.new(name=bone);g.add(list(range(len(obj.data.vertices))),1,'REPLACE')

def skin_color(obj,kind):
    base,belly,dark=palette(kind)
    attr=obj.data.color_attributes.new(name='SkinColor',type='FLOAT_COLOR',domain='POINT')
    for v in obj.data.vertices:
        p=obj.matrix_world@v.co
        wave=.5+.5*sin(p.x*.052 + 1.3*sin(p.z*.039)+.7*sin(p.y*.069))
        mottling=.90+.10*sin(p.x*.61+p.y*.45)*sin(p.z*.53)
        stripe=max(0,(wave-.58)/.42)*.72
        underside=max(0,min(1,(.23-v.normal.z)*.68))
        c=[((1-stripe)*base[k]+stripe*dark[k])*(1-underside)+belly[k]*underside for k in range(3)]
        attr.data[v.index].color=(*[max(.01,a*mottling) for a in c],1)

def finish(obj,name,mat,bone=None):
    obj.name=name
    bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    obj.data.materials.append(mat)
    for p in obj.data.polygons:p.use_smooth=True
    weights(obj,bone)
    if mat.name=='Dino_Skin':skin_color(obj,SPECIES)
    PARTS.append(obj)
    return obj

def ellipsoid(name,loc,scale,mat,bone=None,segments=24,rings=14):
    bpy.ops.mesh.primitive_uv_sphere_add(segments=segments,ring_count=rings,location=loc)
    o=bpy.context.object;o.scale=scale
    return finish(o,name,mat,bone)

def sweep(name,points,radii,mat,bone=None,sides=16):
    """Tube with elliptical cross-sections perpendicular to a sampled centreline."""
    verts=[];faces=[]
    for i,point in enumerate(points):
        p=Vector(point)
        tangent=(Vector(points[min(i+1,len(points)-1)])-Vector(points[max(0,i-1)])).normalized()
        side=tangent.cross(Vector((0,0,1)))
        if side.length<.01:side=Vector((1,0,0))
        else:side.normalize()
        up=side.cross(tangent).normalized()
        r=radii[i];ry,rz=(r,r) if isinstance(r,(int,float)) else r
        for j in range(sides):
            a=2*pi*j/sides;verts.append(p+side*(cos(a)*ry)+up*(sin(a)*rz))
        if i:
            for j in range(sides):faces.append(((i-1)*sides+j,(i-1)*sides+(j+1)%sides,i*sides+(j+1)%sides,i*sides+j))
    faces.append(tuple(reversed(range(sides))))
    faces.append(tuple((len(points)-1)*sides+j for j in range(sides)))
    mesh=bpy.data.meshes.new(name);mesh.from_pydata(verts,[],faces);mesh.update()
    bm=bmesh.new();bm.from_mesh(mesh);bmesh.ops.recalc_face_normals(bm,faces=bm.faces);bm.to_mesh(mesh);bm.free();mesh.update()
    o=bpy.data.objects.new(name,mesh);bpy.context.collection.objects.link(o)
    bpy.context.view_layer.objects.active=o;o.select_set(True)
    finish(o,name,mat,bone)
    o.select_set(False)
    return o

def bone(name,head,tail,parent=None):BONES[name]=(head,tail,parent)

def skin_joint(name,a,b,r1,r2,mat,bone_name):
    mid=Vector(a).lerp(Vector(b),.48)
    return sweep(name,[a,mid,b],[r1,(r1+r2)*.55,r2],mat,bone_name,20)

def assign_body(o):
    for v in o.data.vertices:
        x=v.co.x
        if x<(-65 if SPECIES in ('Raptor','Prey') else -160):
            anchors=[(-180,'tail_01'),(-310,'tail_02'),(-450,'tail_03'),(-580,'tail_04')]
            if SPECIES in ('Raptor','Prey'):anchors=[(-65,'tail_01'),(-155,'tail_02'),(-250,'tail_03'),(-335,'tail_04')]
            if SPECIES=='Trike':anchors=[(-195,'tail_01'),(-275,'tail_02'),(-350,'tail_03'),(-425,'tail_04')]
            bn=min(anchors,key=lambda a:abs(x-a[0]))[1]
        else:bn='spine'
        g=o.vertex_groups.get(bn) or o.vertex_groups.new(name=bn);g.add([v.index],1,'REPLACE')

def build(kind):
    global PARTS,BONES,SPECIES
    bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
    PARTS=[];BONES={};SPECIES=kind
    scene=bpy.context.scene;scene.unit_settings.system='METRIC';scene.unit_settings.scale_length=.01
    scene.render.fps=30
    skin=material('Dino_Skin',palette(kind)[0]);horn=material('Dino_Horn',(.62,.54,.37),.48)
    claw=material('Dino_Claw',(.12,.105,.072),.48);mouth=material('Dino_Mouth',(.09,.025,.023),.6)
    eye=material('Dino_Eye',(.95,.39,.025),.23);pupil=material('Dino_Pupil',(.008,.011,.008),.18)
    tooth=material('Dino_Tooth',(.82,.77,.60),.43)
    bone('root',(0,0,0),(0,0,60))
    is_trike=kind=='Trike';small=kind in ('Raptor','Prey')
    if kind=='Trex':
        hip=(-95,0,260);neck=(115,0,345);head=(235,0,400);jaw=(237,0,372)
        body_pts=[(-665,0,135),(-580,0,158),(-450,0,200),(-310,0,246),(-180,0,273),(-95,0,280),(10,0,285),(90,0,309),(147,0,352)]
        body_rad=[(2,3),(12,14),(27,30),(47,43),(88,88),(107,121),(100,125),(76,95),(52,63)]
        tail=[(-180,0,273),(-310,0,246),(-450,0,200),(-580,0,158),(-665,0,135)]
        leg_y=79;legs=[('leg_l',(-100,-leg_y,270),(-15,-leg_y-8,156),(-85,-leg_y-5,51),(-18,-leg_y-5,14)),('leg_r',(-100,leg_y,270),(-15,leg_y+8,156),(-85,leg_y+5,51),(-18,leg_y+5,14))]
    elif small:
        hip=(-28,0,128);neck=(58,0,155);head=(122,0,205);jaw=(120,0,193)
        body_pts=[(-380,0,111),(-335,0,116),(-250,0,123),(-155,0,135),(-65,0,138),(-20,0,136),(30,0,139),(68,0,164)]
        body_rad=[(1,1),(5,6),(10,11),(15,15),(27,32),(32,46),(31,42),(19,28)]
        tail=[(-65,0,138),(-155,0,135),(-250,0,123),(-335,0,116),(-380,0,111)]
        leg_y=26;legs=[('leg_l',(-30,-leg_y,128),(16,-leg_y-3,75),(-27,-leg_y,28),(22,-leg_y,7)),('leg_r',(-30,leg_y,128),(16,leg_y+3,75),(-27,leg_y,28),(22,leg_y,7))]
    else:
        hip=(-110,0,178);neck=(145,0,188);head=(214,0,185);jaw=(231,0,146)
        body_pts=[(-452,0,105),(-425,0,116),(-350,0,137),(-275,0,163),(-195,0,185),(-110,0,197),(-10,0,210),(95,0,205),(160,0,183)]
        body_rad=[(1,2),(10,12),(24,25),(38,36),(66,67),(118,100),(140,115),(118,105),(65,57)]
        tail=[(-195,0,185),(-275,0,163),(-350,0,137),(-425,0,116),(-452,0,105)]
        legs=[]
        for x,n in [(-125,'leg'),(100,'arm')]:
            for y,s in [(-93,'l'),(93,'r')]:legs.append((n+'_'+s,(x,y,189),(x+12,y*1.07,100),(x-5,y*1.09,31),(x+20,y*1.09,11)))
    bone('spine',hip,neck,'root');bone('neck',neck,head,'spine');bone('head',head,(head[0]+80,0,head[2]),'neck');bone('jaw',jaw,(jaw[0]+80,0,jaw[2]),'head')
    for i in range(4):bone('tail_%02d'%(i+1),tail[i],tail[i+1],'spine' if i==0 else 'tail_%02d'%i)
    body=sweep(kind+'_Body',body_pts,body_rad,skin,sides=32);assign_body(body)
    skin_joint('Neck',neck,head,24 if small else 60,21 if small else 66,skin,'neck')
    if not is_trike:
        if small:
            upper=[(105,0,206),(135,0,215),(173,0,207),(187,0,204)]
            rad=[(18,23),(23,25),(15,13),(6,7)]
            lower=[(118,0,188),(147,0,187),(180,0,195)];lr=[(14,9),(16,7),(6,4)]
        else:
            upper=[(216,0,406),(260,0,436),(323,0,435),(400,0,421),(431,0,412)]
            rad=[(49,52),(72,65),(69,60),(59,41),(32,29)]
            lower=[(244,0,365),(303,0,362),(380,0,375),(423,0,385)];lr=[(46,27),(55,23),(48,18),(25,12)]
        sweep('Cranium',upper,rad,skin,'head',32);sweep('Lower_jaw',lower,lr,skin,'jaw',28)
        for sign in [-1,1]:
            ex,ey,ez=(129,21,221) if small else (274,66,455)
            ellipsoid('Eye_socket',(ex,ey*sign,ez),(9,4,8) if small else (24,7,19),claw,'head')
            ellipsoid('Amber_eye',(ex+1,(ey+3)*sign,ez),(5.5,2.7,5.7) if small else (12,4.4,12),eye,'head')
            ellipsoid('Eye_slit',(ex+2,(ey+5)*sign,ez),(1.5,1.2,4.5) if small else (3,2,9),pupil,'head',16,10)
            ellipsoid('Brow',(ex-2,(ey-1)*sign,ez+(8 if small else 22)),(15,7,6) if small else (33,14,13),skin,'head')
            nx,ny,nz=(176,11,212) if small else (402,51,431)
            ellipsoid('Nostril',(nx,ny*sign,nz),(4,1.5,2.5) if small else (10,3,5),pupil,'head',16,8)
            count=10 if small else 14
            for i in range(count):
                t=i/(count-1);x=(136+43*t) if small else (280+132*t);y=(17-8*t) if small else (57-12*t)
                z=(199-2*t) if small else (392+5*t)
                length=(6+2*sin(pi*t)) if small else (17+10*sin(pi*t))
                sweep('Upper_tooth',[(x,sign*y,z),(x+1,sign*y,z-length)],[2 if small else 4.8,.2],tooth,'head',8)
                sweep('Lower_tooth',[(x,sign*y,z-length-4),(x+1,sign*y,z-5)],[1.5 if small else 3.5,.15],tooth,'jaw',8)
        ellipsoid('Mouth_cavity',((155 if small else 335),0,(194 if small else 389)),(31,14,5) if small else (87,45,12),mouth,'head')
    else:
        sweep('Ceratopsian_skull',[(170,0,191),(221,0,201),(288,0,158),(341,0,133)],[(67,60),(78,66),(53,40),(22,22)],skin,'head',32)
        ellipsoid('Beak',(340,0,129),(35,26,22),claw,'head')
        ellipsoid('Jaw',(277,0,126),(56,42,22),skin,'jaw')
        # Scalloped, solid frill; angled back and raised above shoulders.
        verts=[(158,0,265)];faces=[]
        count=40
        for ring,r in enumerate([.48,1]):
            for j in range(count):
                a=2*pi*j/count;sc=1+.035*cos(a*10)
                verts.append((158-58*sin(a)*r,155*cos(a)*r*sc,263+138*sin(a)*r*sc))
        for j in range(count):
            faces.append((0,1+j,1+(j+1)%count));faces.append((1+j,41+j,41+(j+1)%count,1+(j+1)%count))
        mesh=bpy.data.meshes.new('Frill');mesh.from_pydata(verts,[],faces);mesh.update()
        fr=bpy.data.objects.new('Trike_Frill',mesh);bpy.context.collection.objects.link(fr)
        bpy.context.view_layer.objects.active=fr;fr.select_set(True)
        mod=fr.modifiers.new('Frill_thickness','SOLIDIFY');mod.thickness=13;bpy.ops.object.modifier_apply(modifier=mod.name)
        finish(fr,'Trike_Frill',skin,'head');fr.select_set(False)
        for j in range(13):
            a=-.10+pi*1.20*j/12;start=(158-58*sin(a),155*cos(a),263+138*sin(a))
            end=(start[0]-9,start[1]*1.10,start[2]+16)
            sweep('Frill_ossicle',[start,end],[12,.6],horn,'head',10)
        for sign in [-1,1]:
            sweep('Brow_horn',[(218,sign*54,248),(260,sign*59,281),(308,sign*62,319),(356,sign*62,351),(396,sign*59,365)],[26,22,16,9,.3],horn,'head',20)
            ellipsoid('Eye_socket',(256,sign*64,199),(20,9,16),claw,'head')
            ellipsoid('Amber_eye',(259,sign*71,201),(9,4,9),eye,'head')
            ellipsoid('Eye_slit',(261,sign*74,201),(2.5,1.5,7),pupil,'head',16,10)
            ellipsoid('Nostril',(318,sign*32,151),(10,3,6),pupil,'head')
        sweep('Nasal_horn',[(310,0,188),(337,0,226),(348,0,255)],[18,11,.3],horn,'head',18)
    # Articulated legs, padded joints, foot pads and visible keratin claws.
    for name,a,b,c,d in legs:
        bone(name+'_upper',a,b,'spine');bone(name+'_lower',b,c,name+'_upper');bone(name+'_foot',c,d,name+'_lower')
        r=(21 if small else 49) if not is_trike else 38
        ellipsoid(name+'_haunch',a,(r*1.3,r*.86,r*1.53),skin,name+'_upper')
        skin_joint(name+'_thigh',a,b,r,r*.66,skin,name+'_upper')
        ellipsoid(name+'_knee',b,(r*.68,r*.62,r*.67),skin,name+'_lower')
        skin_joint(name+'_shin',b,c,r*.58,r*.29,skin,name+'_lower')
        skin_joint(name+'_metatarsal',c,d,r*.29,r*.35,skin,name+'_foot')
        ellipsoid(name+'_footpad',d,(r*.83,r*.66,r*.28),skin,name+'_foot')
        n_toes=4 if is_trike else 3
        for j in range(n_toes):
            oy=(j-(n_toes-1)/2)*r*.42
            toe=(d[0]+r*.88,d[1]+oy,d[2]-r*.09)
            skin_joint(name+'_toe',d,toe,r*.19,r*.12,skin,name+'_foot')
            tip=(toe[0]+r*.40,toe[1],max(2,toe[2]-r*.14))
            sweep(name+'_claw',[toe,tip],[r*.16,.2],claw,name+'_foot',10)
        if kind=='Raptor':
            sweep('Sickle_claw',[(d[0]+4,d[1]*.66,14),(d[0]+17,d[1]*.66,23),(d[0]+29,d[1]*.66,16),(d[0]+29,d[1]*.66,8)],[4,3,1.8,.15],claw,name+'_foot',12)
    if not is_trike:
        for sign,side in [(-1,'l'),(1,'r')]:
            if small:a=(43,sign*28,152);b=(56,sign*46,113);c=(102,sign*41,122);r=8
            else:a=(107,sign*66,311);b=(135,sign*86,267);c=(169,sign*77,282);r=13
            bone('arm_'+side+'_upper',a,b,'spine');bone('arm_'+side+'_lower',b,c,'arm_'+side+'_upper')
            skin_joint('Upper_arm',a,b,r,r*.72,skin,'arm_'+side+'_upper');skin_joint('Forearm',b,c,r*.72,r*.43,skin,'arm_'+side+'_lower')
            for j in range(3 if small else 2):
                tip=(c[0]+(24 if small else 22),c[1]+sign*(j-1)*6,c[2]-10)
                sweep('Hand_digit',[c,tip],[3 if small else 4,.3],claw,'arm_'+side+'_lower',10)
            if kind=='Raptor':
                for j in range(8):
                    a2=Vector(b).lerp(Vector(c),j/8)
                    sweep('Arm_quill',[a2,a2+Vector((-21-j*1.8,sign*(15+j),-5))],[(2,3),(.1,.2)],skin,'arm_'+side+'_lower',6)
    # Dorsal scutes catch the silhouette light.
    for j in range(12):
        x=(-155+j*28) if not small else (-70+j*12)
        z=(368-abs(x)*.06) if kind=='Trex' else (177-abs(x)*.08) if small else (301-abs(x)*.15)
        if is_trike and x>100:continue
        ellipsoid('Dorsal_scute',(x,0,z),(9 if not small else 4,5 if not small else 3,6 if not small else 4),skin,'spine',12,8)
    # Consolidate, retain material slots and explicit skin groups, then rig.
    bpy.ops.object.select_all(action='DESELECT')
    for o in PARTS:o.select_set(True)
    bpy.context.view_layer.objects.active=body;bpy.ops.object.join();mesh=bpy.context.object
    mesh.name='SK_'+kind
    bpy.ops.object.transform_apply(location=True,rotation=True,scale=True)
    bm=bmesh.new();bm.from_mesh(mesh.data);bmesh.ops.recalc_face_normals(bm,faces=bm.faces);bm.to_mesh(mesh.data);bm.free();mesh.data.update()
    # Simple unwrap suitable for procedural detail materials and later hand painting.
    bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.uv.smart_project(island_margin=.015);bpy.ops.object.mode_set(mode='OBJECT')
    bpy.ops.object.select_all(action='DESELECT')
    arm_data=bpy.data.armatures.new(kind+'_Skeleton');rig=bpy.data.objects.new('Rig_'+kind,arm_data);bpy.context.collection.objects.link(rig)
    bpy.context.view_layer.objects.active=rig;rig.select_set(True);bpy.ops.object.mode_set(mode='EDIT')
    for n,(h,t,p) in BONES.items():
        b=arm_data.edit_bones.new(n);b.head=h;b.tail=t
        if p:b.parent=arm_data.edit_bones[p]
    bpy.ops.object.mode_set(mode='OBJECT');rig.show_in_front=True
    mesh.parent=rig;mod=mesh.modifiers.new('Dinosaur_rig','ARMATURE');mod.object=rig
    # Rig in centimetres. Small prey uses the raptor skeleton at a smaller overall scale.
    if kind=='Prey':
        mesh.scale=(.68,.68,.68);rig.scale=(.68,.68,.68)
        mesh.scale=(1,1,1)
    make_actions(rig,kind)
    # Main FBX is rig+mesh only. Actions exported separately to avoid ambiguous takes.
    rig.animation_data.action=None
    for pb in rig.pose.bones:pb.rotation_euler=(0,0,0);pb.location=(0,0,0)
    bpy.context.scene.frame_set(1)
    bpy.ops.object.select_all(action='DESELECT');rig.select_set(True);mesh.select_set(True);bpy.context.view_layer.objects.active=rig
    export_options=dict(use_selection=True,object_types={'ARMATURE','MESH'},add_leaf_bones=False,axis_forward='-Y',axis_up='Z',apply_unit_scale=True,use_mesh_modifiers=True,mesh_smooth_type='FACE',primary_bone_axis='Y',secondary_bone_axis='X')
    bpy.ops.export_scene.fbx(filepath=str(EXPORT/(kind+'.fbx')),bake_anim=False,**export_options)
    for action in list(bpy.data.actions):
        if not action.name.startswith(kind+'_'):continue
        rig.animation_data.action=action
        bpy.context.scene.frame_start=int(action.frame_range[0]);bpy.context.scene.frame_end=int(action.frame_range[1])
        bpy.ops.export_scene.fbx(filepath=str(EXPORT/(action.name+'.fbx')),bake_anim=True,bake_anim_use_all_actions=False,bake_anim_use_nla_strips=False,bake_anim_simplify_factor=0,**export_options)
    rig.animation_data.action=bpy.data.actions.get(kind+'_Idle');scene.frame_start=1;scene.frame_end=60;scene.frame_set(1)
    bpy.ops.wm.save_as_mainfile(filepath=str(OUT/(kind+'.blend')))
    print('DINOSAUR_ASSET_COMPLETE',kind,len(mesh.data.vertices),len(mesh.data.polygons),flush=True)

def make_actions(rig,kind):
    rig.animation_data_create()
    small=kind in ('Raptor','Prey');trike=kind=='Trike'
    lengths={'Idle':60,'Walk':30,'Run':24,'Quick':20,'Charge':36,'Heavy':30,'Jump':30,'Brace':40,'Death':45,'Eat':48}
    for name,frames in lengths.items():
        action=bpy.data.actions.new(kind+'_'+name);action.use_fake_user=True;rig.animation_data.action=action
        for f in range(1,frames+1):
            t=(f-1)/(frames-1);cycle=2*pi*t
            for pb in rig.pose.bones:pb.rotation_mode='XYZ';pb.rotation_euler=(0,0,0);pb.location=(0,0,0)
            def rot(n,x=0,y=0,z=0):
                if n in rig.pose.bones:
                    pb=rig.pose.bones[n];basis=pb.bone.matrix_local.to_quaternion()
                    q=Euler(tuple(math.radians(a) for a in (y,x,z)),'XYZ').to_quaternion()
                    pb.rotation_euler=(basis.inverted()@q@basis).to_euler('XYZ')
            if name in ('Idle','Walk','Run'):
                moving=name!='Idle';run=name=='Run';ampl=(24 if run else 16) if moving else 0
                for side,phase in [('l',0),('r',pi)]:
                    s=sin(cycle+phase);c=cos(cycle+phase)
                    rot('leg_'+side+'_upper',ampl*s);rot('leg_'+side+'_lower',-ampl*.9*max(0,s));rot('leg_'+side+'_foot',-ampl*.3*s)
                    rot('arm_'+side+'_upper',(-ampl*.6*s) if trike else 4*sin(cycle+phase));rot('arm_'+side+'_lower',ampl*.4*max(0,-s) if trike else 4)
                rig.pose.bones['root'].location.z=(3 if small else 6)*(1-cos(cycle*2)) if moving else 0
                rot('spine',(2 if moving else .8)*sin(cycle*2));rot('neck',-2*sin(cycle*2));rot('head',1.5*sin(cycle))
                for j in range(1,5):rot('tail_%02d'%j,0,0,(3 if moving else 1.5)*sin(cycle-j*.65))
            elif name=='Quick':
                punch=sin(pi*t)**2
                rot('neck',(-16 if trike else 14)*punch);rot('head',(-18 if trike else -12)*punch);rot('jaw',(-26 if not trike else -5)*sin(pi*t))
                rot('spine',-4*punch)
                if small:rot('arm_l_upper',-25*punch);rot('arm_r_upper',25*punch)
            elif name=='Charge':
                rot('spine',-6);rot('neck',-22 if trike else 10);rot('head',-12 if trike else 5);rot('jaw',-13 if not trike else 0)
                for s in ['l','r']:rot('leg_'+s+'_upper',12);rot('leg_'+s+'_lower',-18)
                for j in range(1,5):rot('tail_%02d'%j,0,0,2*sin(cycle*2+j))
            elif name=='Heavy':
                punch=sin(pi*t)**2
                rot('spine',-10*punch);rot('neck',(-32 if trike else 24)*punch);rot('head',(-24 if trike else -15)*punch);rot('jaw',-38*sin(pi*t) if not trike else 0)
                for s in ['l','r']:rot('arm_'+s+'_upper',-35*punch if small else 0)
            elif name=='Jump':
                tuck=sin(pi*t)
                for s in ['l','r']:rot('leg_'+s+'_upper',-25*tuck);rot('leg_'+s+'_lower',55*tuck);rot('arm_'+s+'_upper',20*tuck)
                rot('spine',5*tuck);rot('tail_01',-8*tuck)
            elif name=='Brace':
                rot('neck',-25 if trike else 12);rot('head',-22 if trike else -14);rot('spine',-5)
                for s in ['l','r']:rot('leg_'+s+'_upper',8);rot('leg_'+s+'_lower',-12)
            elif name=='Death':
                k=min(1,t*1.6);rot('root',0,78*k,0)
                rig.pose.bones['root'].location.z=-(45 if small else 100)*k
                rot('neck',15*k);rot('jaw',-25*k);rot('leg_l_upper',-26*k);rot('leg_r_upper',12*k)
            elif name=='Eat':
                rot('neck',35+6*sin(cycle*2));rot('head',22);rot('jaw',-12*(.5+.5*sin(cycle*4)))
            for pb in rig.pose.bones:
                pb.keyframe_insert(data_path='rotation_euler',frame=f,group=pb.name)
                if pb.name=='root':pb.keyframe_insert(data_path='location',frame=f,group=pb.name)

if __name__=='__main__':
    import sys
    wanted=sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else ['Trex','Raptor','Trike','Prey']
    for species in wanted:build(species)
