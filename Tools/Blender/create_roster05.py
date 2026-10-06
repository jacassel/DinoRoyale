"""Original 0.5 herbivores: continuous skinned anatomy, articulated rigs and clips.
Run Blender --background --python Tools/Blender/create_roster05.py -- Anky
Coordinates are centimetres, +X forward; old source files are never opened.
"""
import bpy, bmesh, math, pathlib, sys, importlib.util, json
from mathutils import Vector, Euler
from math import sin, cos, pi
ROOT=pathlib.Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('base',ROOT/'Tools/Blender/create_dinosaurs.py')
g=importlib.util.module_from_spec(spec);spec.loader.exec_module(g)
OUT=ROOT/'Assets/Source/Roster05';EXP=ROOT/'Assets/Export/Roster05'
OUT.mkdir(parents=True,exist_ok=True);EXP.mkdir(parents=True,exist_ok=True)
PALETTES={'Anky':((.24,.29,.19),(.46,.42,.26),(.085,.12,.09)),
          'Pachy':((.36,.20,.12),(.62,.49,.30),(.12,.075,.05)),
          'Brachi':((.24,.31,.32),(.51,.53,.42),(.085,.15,.17))}
g.palette=lambda kind:PALETTES[kind]

def build(kind):
    bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
    for a in list(bpy.data.actions):bpy.data.actions.remove(a)
    g.PARTS=[];g.BONES={};g.SPECIES=kind
    scene=bpy.context.scene;scene.unit_settings.system='METRIC';scene.unit_settings.scale_length=.01;scene.render.fps=30
    skin=g.material('Dino_Skin',PALETTES[kind][0]);horn=g.material('Dino_Horn',(.31,.27,.17))
    eye=g.material('Dino_Eye',(.7,.31,.035),.25);pupil=g.material('Dino_Pupil',(.005,.008,.006),.18)
    claw=g.material('Dino_Claw',(.09,.085,.06),.5)
    g.bone('root',(0,0,0),(0,0,60))
    if kind=='Anky':
        hip=(-115,0,148);neck=(133,0,157);head=(226,0,133)
        pts=[(-200,0,146),(-140,0,164),(-35,0,171),(65,0,173),(140,0,157)]
        radii=[(55,52),(114,74),(150,86),(130,79),(65,50)]
        tail=[(-178,0,145),(-265,0,140),(-355,0,128),(-450,0,121),(-535,0,119)]
        tr=[(62,43),(35,27),(22,20),(16,15),(12,13)]
        legs=[(x,y,145 if x<0 else 150,28) for x in (-130,100) for y in (-101,101)]
        skull=(254,0,134);skullscale=(73,48,37)
    elif kind=='Pachy':
        hip=(-35,0,158);neck=(55,0,199);head=(119,0,237)
        pts=[(-93,0,162),(-38,0,165),(12,0,176),(61,0,199)]
        radii=[(30,34),(48,67),(45,58),(27,34)]
        tail=[(-75,0,164),(-151,0,155),(-238,0,138),(-315,0,124),(-383,0,118)]
        tr=[(32,31),(21,22),(13,15),(7,9),(1,2)]
        legs=[(-40,y,159,24) for y in (-39,39)]
        skull=(145,0,237);skullscale=(48,28,30)
    else:
        hip=(-215,0,342);neck=(165,0,412);head=(375,0,1080)
        pts=[(-300,0,324),(-210,0,350),(-60,0,386),(84,0,411),(195,0,423)]
        radii=[(65,75),(162,174),(192,194),(169,189),(95,116)]
        tail=[(-277,0,334),(-438,0,276),(-619,0,206),(-790,0,151),(-935,0,120)]
        tr=[(70,74),(44,48),(23,29),(11,14),(1,2)]
        legs=[(x,y,342 if x<0 else 415,48) for x in (-211,137) for y in (-128,128)]
        skull=(407,0,1093);skullscale=(68,38,49)
    g.bone('spine',hip,neck,'root')
    if kind=='Brachi':
        np=[neck,(230,0,618),(292,0,823),head]
        for i in range(3):g.bone('neck' if i==0 else 'neck_'+str(i+1),np[i],np[i+1],'spine' if i==0 else 'neck' if i==1 else 'neck_2')
        g.bone('head',head,(head[0]+95,0,head[2]),'neck_3')
        no=g.sweep('Long_muscular_neck',np,[(96,102),(67,73),(45,52),(29,34)],skin,sides=32)
        for v in no.data.vertices:
            z=v.co.z
            anchors=[(460,'neck'),(690,'neck_2'),(940,'neck_3')]
            distances=sorted((abs(z-a),n) for a,n in anchors)[:2]
            total=sum(1/max(20,d)**2 for d,n in distances)
            for d,n in distances:
                vg=no.vertex_groups.get(n) or no.vertex_groups.new(name=n);vg.add([v.index],(1/max(20,d)**2)/total,'REPLACE')
    else:
        g.bone('neck',neck,head,'spine');g.bone('head',head,(head[0]+70,0,head[2]),'neck')
        g.skin_joint('Neck',neck,head,48 if kind=='Anky' else 27,34 if kind=='Anky' else 25,skin,'neck')
    g.bone('jaw',(skull[0],0,skull[2]-20),(skull[0]+45,0,skull[2]-20),'head')
    body=g.sweep(kind+'_Torso',pts,radii,skin,'spine',32)
    for i in range(4):g.bone('tail_%02d'%(i+1),tail[i],tail[i+1],'spine' if i==0 else 'tail_%02d'%i)
    to=g.sweep('Tapered_tail',tail,tr,skin,sides=24)
    for v in to.data.vertices:
        anchors=[(Vector(tail[i]).lerp(Vector(tail[i+1]),.35).x,'tail_%02d'%(i+1)) for i in range(4)]
        ds=sorted((abs(v.co.x-a),n) for a,n in anchors)[:2];total=sum(1/max(10,d)**2 for d,n in ds)
        for d,n in ds:
            vg=to.vertex_groups.get(n) or to.vertex_groups.new(name=n);vg.add([v.index],(1/max(10,d)**2)/total,'REPLACE')
    g.ellipsoid('Skull',skull,skullscale,skin,'head',32,20)
    g.ellipsoid('Lower_jaw',(skull[0]+14,0,skull[2]-22),(skullscale[0]*.8,skullscale[1]*.8,14),skin,'jaw')
    for side in (-1,1):
        ex=skull[0]-12;ey=skullscale[1]*.91;ez=skull[2]+12
        g.ellipsoid('Eye',(ex,ey*side,ez),(7,3,6),eye,'head')
        g.ellipsoid('Pupil',(ex+1,(ey+2.4)*side,ez),(2,1.2,4),pupil,'head',16,8)
        g.ellipsoid('Brow',(ex-3,(ey-2)*side,ez+7),(16,7,6),skin,'head')
        g.ellipsoid('Nostril',(skull[0]+skullscale[0]*.78,side*skullscale[1]*.48,skull[2]+8),(5,2,3),pupil,'head',12,8)
    g.bone('head_impact',(skull[0]+skullscale[0]*.72,0,skull[2]+10),(skull[0]+skullscale[0],0,skull[2]+10),'head')
    for x,y,z,r in legs:
        name=('arm' if x>0 else 'leg')+('_l' if y<0 else '_r')
        a=(x,y,z);b=(x+(25 if kind=='Pachy' else 5),y*1.06,z*.55);c=(x-18,y*1.08,32 if kind!='Brachi' else 57);d=(x+20,y*1.08,12 if kind!='Brachi' else 23)
        for suffix,h,t,parent in [('_upper',a,b,'spine'),('_lower',b,c,name+'_upper'),('_foot',c,d,name+'_lower')]:g.bone(name+suffix,h,t,parent)
        g.ellipsoid(name+'_haunch',a,(r*1.35,r,r*1.65),skin,name+'_upper')
        g.skin_joint(name+'_thigh',a,b,r,r*.8,skin,name+'_upper');g.skin_joint(name+'_shin',b,c,r*.78,r*.56,skin,name+'_lower')
        g.ellipsoid(name+'_ankle',c,(r*.6,r*.58,r*.58),skin,name+'_foot')
        g.ellipsoid(name+'_pad',d,(r*1.08,r*.82,r*.44),skin,name+'_foot')
        for j in range(4 if kind!='Pachy' else 3):
            g.ellipsoid(name+'_nail',(d[0]+r*.9,d[1]+(j-1.5)*r*.34,d[2]-2),(r*.35,r*.19,r*.2),claw,name+'_foot',12,8)
    if kind=='Pachy':
        for side in (-1,1):
            n='arm_l' if side<0 else 'arm_r';a=(45,side*37,191);b=(66,side*44,150);c=(86,side*39,143)
            g.bone(n+'_upper',a,b,'spine');g.bone(n+'_lower',b,c,n+'_upper')
            g.skin_joint(n,a,b,9,6,skin,n+'_upper');g.skin_joint(n+'_fore',b,c,6,5,skin,n+'_lower')
        g.ellipsoid('Reinforced_dome',(134,0,268),(35,27,28),horn,'head',32,20)
        for j in range(11):
            a=pi*.3+pi*1.4*j/10;p=(128+32*cos(a),29*sin(a),257)
            g.ellipsoid('Skull_knob',p,(8,7,8),horn,'head',12,8)
            if j%2==0:g.sweep('Skull_spike',[p,(p[0]-12,p[1]*1.28,p[2]+7)],[6,.5],horn,'head',10)
    elif kind=='Anky':
        # Broad, low armor follows torso curvature; individual osteoderms stay rigid.
        for x in (-150,-100,-45,15,75,120):
            for y in (-94,-48,0,48,94):
                z=171+77*math.sqrt(max(.05,1-(y/157)**2))-(abs(x)/180)*17
                g.ellipsoid('Osteoderm',(x,y,z),(22,20,12),horn,'spine',16,10)
                if abs(y)>90:g.sweep('Flank_spike',[(x,y,z-16),(x-15,y*1.5,z+5)],[16,.5],horn,'spine',12)
        for j in range(3):
            for sign in (-1,1):g.ellipsoid('Neck_plate',(155+j*24,sign*28,188-j*9),(19,17,9),horn,'neck' if j<2 else 'head',16,10)
        for sign in (-1,1):g.sweep('Cheek_horn',[(223,sign*43,146),(202,sign*65,167)],[12,.3],horn,'head',12)
        g.ellipsoid('Tail_club',tail[-1],(58,71,38),horn,'tail_04',32,20)
        g.bone('club_tip',tail[-1],(-550,0,119),'tail_04')
    else:
        g.ellipsoid('Nasal_arch',(389,0,1129),(32,27,29),skin,'head')
        g.bone('stomp_l',(160,-138,24),(160,-138,35),'arm_l_foot')
        g.bone('stomp_r',(160,138,24),(160,138,35),'arm_r_foot')
    # Union only skin volumes; preserve armor/nails/eyes as rigid weighted surfaces.
    g.refine_skin(body)
    bpy.ops.object.select_all(action='DESELECT')
    for o in g.PARTS:o.select_set(True)
    bpy.context.view_layer.objects.active=g.PARTS[0];bpy.ops.object.join();mesh=bpy.context.object;mesh.name='SK_'+kind
    bpy.ops.object.transform_apply(location=True,rotation=True,scale=True)
    bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.uv.smart_project(island_margin=.015);bpy.ops.object.mode_set(mode='OBJECT')
    bpy.ops.object.select_all(action='DESELECT');ad=bpy.data.armatures.new(kind+'_Skeleton');rig=bpy.data.objects.new('Rig_'+kind,ad);bpy.context.collection.objects.link(rig)
    bpy.context.view_layer.objects.active=rig;rig.select_set(True);bpy.ops.object.mode_set(mode='EDIT')
    for name,(h,t,parent) in g.BONES.items():
        b=ad.edit_bones.new(name);b.head=h;b.tail=t
        if parent:b.parent=ad.edit_bones[parent]
    bpy.ops.object.mode_set(mode='OBJECT');mesh.parent=rig;mod=mesh.modifiers.new('Skin','ARMATURE');mod.object=rig;rig.show_in_front=True
    actions(rig,kind)
    rig.animation_data.action=None
    for pb in rig.pose.bones:pb.rotation_euler=(0,0,0);pb.location=(0,0,0)
    scene.frame_set(1);bpy.ops.object.select_all(action='DESELECT');rig.select_set(True);mesh.select_set(True)
    opts=dict(use_selection=True,object_types={'ARMATURE','MESH'},add_leaf_bones=False,axis_forward='-Y',axis_up='Z',apply_unit_scale=True,use_mesh_modifiers=True,mesh_smooth_type='FACE',primary_bone_axis='Y',secondary_bone_axis='X')
    bpy.ops.export_scene.fbx(filepath=str(EXP/(kind+'.fbx')),bake_anim=False,**opts)
    for a in bpy.data.actions:
        rig.animation_data.action=a;scene.frame_start=int(a.frame_range[0]);scene.frame_end=int(a.frame_range[1])
        bpy.ops.export_scene.fbx(filepath=str(EXP/(a.name+'.fbx')),bake_anim=True,bake_anim_use_all_actions=False,bake_anim_use_nla_strips=False,bake_anim_simplify_factor=0,**opts)
    rig.animation_data.action=bpy.data.actions[kind+'_Idle'];scene.frame_start=1;scene.frame_end=60;scene.frame_set(1)
    bpy.ops.wm.save_as_mainfile(filepath=str(OUT/(kind+'.blend')))
    print('ROSTER05_COMPLETE',kind,len(mesh.data.vertices),len(bpy.data.actions),flush=True)

def actions(rig,kind):
    rig.animation_data_create()
    lengths={'Idle':60,'Walk':40,'Run':30,'Swim':48,'Quick':30,'Quick2':30,'Quick3':30,'Charge':40,'Heavy':60,'Jump':30,'Brace':40,'Death':60,'Eat':60,'Hit':18,'PivotLeft':40,'PivotRight':40}
    for name,frames in lengths.items():
        act=bpy.data.actions.new(kind+'_'+name);act.use_fake_user=True;rig.animation_data.action=act
        for frame in range(1,frames+1):
            t=(frame-1)/(frames-1);phase=2*pi*t
            for pb in rig.pose.bones:pb.rotation_mode='XYZ';pb.rotation_euler=(0,0,0);pb.location=(0,0,0)
            def rot(n,pitch=0,roll=0,yaw=0):
                if n not in rig.pose.bones:return
                pb=rig.pose.bones[n];basis=pb.bone.matrix_local.to_quaternion();q=Euler(tuple(math.radians(a) for a in (roll,pitch,yaw)),'XYZ').to_quaternion();pb.rotation_euler=(basis.inverted()@q@basis).to_euler('XYZ')
            def loc(n,v):
                pb=rig.pose.bones[n];pb.location=pb.bone.matrix_local.to_quaternion().inverted()@Vector(v)
            # Root stays on ground; short strides and ankle counter-rotation limit sliding.
            if name in ('Walk','Run','Swim','PivotLeft','PivotRight'):
                swim=name=='Swim';amp=(23 if kind=='Pachy' else 12) if name=='Run' else 9
                if swim:amp=20
                for side,ph in [('l',0),('r',pi)]:
                    wave=sin(phase+ph)
                    for limb,offset in [('leg',1),('arm',-1)]:
                        if kind=='Pachy' and limb=='arm':continue
                        v=wave*offset;rot(limb+'_'+side+'_upper',amp*v);rot(limb+'_'+side+'_lower',-amp*.65*max(0,v));rot(limb+'_'+side+'_foot',-amp*v+amp*.65*max(0,v))
                for j in range(1,5):rot('tail_%02d'%j,yaw=(5 if swim else 1.5)*sin(phase-j*.5))
                rot('neck',-4 if swim else sin(phase)*.6)
            elif name=='Idle':
                rot('neck',.6*sin(phase));rot('head',.8*sin(phase));rot('tail_01',yaw=1.5*sin(phase))
            elif name.startswith('Quick') or name=='Heavy':
                heavy=name=='Heavy';hit=.4 if heavy else .32
                # Wind-up -> impact -> recovery. Peak coincides with configured hit time.
                pulse=max(0,1-abs(t-hit)/(.23 if heavy else .19));pulse=pulse*pulse*(3-2*pulse)
                if kind=='Anky':
                    sign=-1 if name=='Quick2' else 1
                    if name=='Quick3':rot('spine',roll=-7*pulse);rot('head',10*pulse)
                    else:
                        # One committed sweep, then a slow return; no oscillating recovery.
                        keys=[(0,-.65),(.15,-1),(.40,1),(.70,.42),(1,0)]
                        for (ta,va),(tb,vb) in zip(keys,keys[1:]):
                            if ta<=t<=tb:
                                u=(t-ta)/(tb-ta);u=u*u*(3-2*u);angle=(va+(vb-va)*u)*(48 if heavy else 31);break
                        rot('tail_01',yaw=sign*angle);rot('tail_02',yaw=sign*angle*.25);rot('tail_03',yaw=sign*angle*.12);rot('spine',yaw=-sign*angle*.045)
                elif kind=='Pachy':
                    rot('neck',28*pulse);rot('head',-7*pulse);loc('spine',(20*pulse,0,-9*pulse));rot('neck',28*pulse,yaw=(12 if name=='Quick2' else -9 if name=='Quick3' else 0)*pulse)
                else:
                    if name=='Quick2':rot('tail_01',yaw=32*pulse);rot('tail_02',yaw=12*pulse)
                    else:
                        lift=sin(pi*min(1,t/hit)) if t<hit else 0
                        for side in (['l','r'] if heavy else ['r' if name=='Quick3' else 'l']):
                            rot('arm_'+side+'_upper',-17*lift);rot('arm_'+side+'_lower',24*lift)
                        rot('neck',-2*lift);loc('root',(0,0,12*lift))
            elif name in ('Charge','Brace'):
                rot('neck',18 if kind=='Pachy' else 3);rot('head',-8 if kind=='Pachy' else 0)
                if kind=='Anky' and name=='Charge':rot('tail_01',yaw=-30)
            elif name=='Eat':
                if kind=='Brachi':rot('neck',32);rot('neck_2',47);rot('neck_3',42);rot('head',-60)
                else:rot('neck',24+2*sin(phase));rot('head',12)
                rot('jaw',7*(.5+.5*sin(phase*3)))
            elif name=='Hit':rot('spine',roll=4*sin(pi*t));rot('head',-5*sin(pi*t))
            elif name=='Jump':
                if kind=='Pachy':rot('leg_l_upper',-16*sin(pi*t));rot('leg_r_upper',-16*sin(pi*t))
            elif name=='Death':
                k=min(1,t*1.6);k=k*k*(3-2*k);rot('root',roll=78*k)
                loc('root',(0,0,(110 if kind=='Brachi' else 18 if kind=='Anky' else 20)*k));rot('neck',10*k)
            for pb in rig.pose.bones:
                pb.keyframe_insert(data_path='rotation_euler',frame=frame,group=pb.name)
                pb.keyframe_insert(data_path='location',frame=frame,group=pb.name)

if __name__=='__main__':
    for kind in (sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else ['Anky','Pachy','Brachi']):build(kind)
