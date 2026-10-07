"""Original launch Albertosaurus: authored proportions, terracotta/charcoal skin and pursuit clips."""
import bpy, pathlib, importlib.util, math
from mathutils import Vector,Euler
ROOT=pathlib.Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('dino',ROOT/'Tools/Blender/create_dinosaurs.py')
g=importlib.util.module_from_spec(spec);spec.loader.exec_module(g)
g.OUT=ROOT/'Assets/Source/Launch06';g.EXPORT=ROOT/'Assets/Export/Launch06'
g.OUT.mkdir(parents=True,exist_ok=True);g.EXPORT.mkdir(parents=True,exist_ok=True);g.REFINE=True
g.palette=lambda kind:((.31,.16,.095),(.56,.43,.28),(.065,.075,.07))
original_material=g.material
def material(name,color,rough=.65,metal=0):
    mat=original_material(name,color,rough,metal)
    if name=='Dino_Skin':
        nt=mat.node_tree;p=nt.nodes.get('Principled BSDF');attr=nt.nodes.get('Skin_attribute')
        coord=nt.nodes.new('ShaderNodeTexCoord');cells=nt.nodes.new('ShaderNodeTexVoronoi');cells.feature='DISTANCE_TO_EDGE';cells.inputs['Scale'].default_value=.55
        nt.links.new(coord.outputs['Object'],cells.inputs['Vector'])
        ramp=nt.nodes.new('ShaderNodeValToRGB');ramp.color_ramp.elements[0].position=.012;ramp.color_ramp.elements[0].color=(.3,.3,.3,1);ramp.color_ramp.elements[1].position=.11
        nt.links.new(cells.outputs['Distance'],ramp.inputs['Fac'])
        mix=nt.nodes.new('ShaderNodeMixRGB');mix.blend_type='MULTIPLY';mix.inputs[0].default_value=.43
        nt.links.new(attr.outputs['Color'],mix.inputs[1]);nt.links.new(ramp.outputs['Color'],mix.inputs[2]);nt.links.new(mix.outputs[0],p.inputs['Base Color'])
        bump=nt.nodes.new('ShaderNodeBump');bump.inputs['Strength'].default_value=.30;bump.inputs['Distance'].default_value=.40
        nt.links.new(ramp.outputs[0],bump.inputs['Height']);nt.links.new(bump.outputs[0],p.inputs['Normal'])
    return mat
g.material=material
base_bake=g.bake_skin
def bake(mesh,kind):
    base_bake(mesh,kind)
    im=bpy.data.images.new(kind+'_SkinNormal',width=2048,height=2048,alpha=False);im.colorspace_settings.name='Non-Color'
    for mat in mesh.data.materials:
        n=mat.node_tree.nodes.new('ShaderNodeTexImage');n.image=im;mat.node_tree.nodes.active=n
    bpy.ops.object.bake(type='NORMAL',margin=12,use_clear=True)
    im.filepath_raw=str(g.EXPORT/(kind+'_SkinNormal.png'));im.file_format='PNG';im.save();im.pack()
g.bake_skin=bake
base_actions=g.make_actions
def actions(rig,kind):
    base_actions(rig,kind)
    for name in ['Quick','Quick2','Quick3','Heavy','Charge','Hit','PivotLeft','PivotRight']:
        old=bpy.data.actions.get(kind+'_'+name)
        if old:bpy.data.actions.remove(old)
        act=bpy.data.actions.new(kind+'_'+name);act.use_fake_user=True;rig.animation_data.action=act
        for frame in range(1,41):
            t=(frame-1)/39
            for pb in rig.pose.bones:pb.rotation_mode='XYZ';pb.rotation_euler=(0,0,0);pb.location=(0,0,0)
            def rot(n,pitch=0,roll=0,yaw=0):
                pb=rig.pose.bones[n];basis=pb.bone.matrix_local.to_quaternion()
                pb.rotation_euler=(basis.inverted()@Euler(tuple(math.radians(v) for v in (roll,pitch,yaw)),'XYZ').to_quaternion()@basis).to_euler('XYZ')
            if name.startswith('Quick') or name=='Heavy':
                heavy=name=='Heavy';hit=.25/1.12 if heavy else .14/.53
                # Open in anticipation, close through contact, recover without an extra bite.
                wind=math.sin(math.pi*min(1,t/hit)) if t<hit else 0
                strike=max(0,1-abs(t-(hit+.06))/.22);strike=strike*strike*(3-2*strike)
                rot('jaw',30*wind+5*strike);rot('neck',-7*wind+17*strike);rot('head',4*wind-12*strike)
                side=1 if name=='Quick2' else -1 if name=='Quick3' else 0
                rot('spine',-5*strike,yaw=side*7*strike);rot('neck',-7*wind+17*strike,yaw=side*12*strike)
                for j in range(1,5):rot('tail_%02d'%j,yaw=-side*3*strike)
                if heavy:
                    rot('leg_l_upper',-18*strike);rot('leg_r_upper',15*strike);rot('leg_l_lower',20*strike);rot('tail_01',6*strike)
            elif name=='Charge':
                rot('spine',-4);rot('neck',-6);rot('head',4);rot('jaw',14);rot('leg_l_upper',9);rot('leg_r_upper',-8)
            elif name=='Hit':rot('spine',roll=4*math.sin(math.pi*t));rot('head',-7*math.sin(math.pi*t))
            else:
                sign=1 if name=='PivotLeft' else -1
                rot('neck',yaw=sign*7);rot('tail_01',yaw=-sign*8)
                for side,phase in [('l',0),('r',math.pi)]:rot('leg_'+side+'_upper',10*math.sin(2*math.pi*t+phase));rot('leg_'+side+'_foot',-7*math.sin(2*math.pi*t+phase))
            for pb in rig.pose.bones:
                pb.keyframe_insert(data_path='rotation_euler',frame=frame,group=pb.name);pb.keyframe_insert(data_path='location',frame=frame,group=pb.name)
g.make_actions=actions
g.build('Alberto')
