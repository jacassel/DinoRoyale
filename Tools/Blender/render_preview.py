import bpy,sys,pathlib,math
from mathutils import Vector
root=pathlib.Path(__file__).resolve().parents[2]
species=sys.argv[sys.argv.index('--')+1] if '--' in sys.argv else 'Trex'
bpy.ops.wm.open_mainfile(filepath=str(root/'Assets/Source/Dinosaurs'/f'{species}.blend'))
skin=bpy.data.materials.get('Dino_Skin')
if skin:
    nodes=skin.node_tree.nodes;links=skin.node_tree.links
    col=nodes.new('ShaderNodeVertexColor');col.layer_name='SkinColor'
    p=nodes.get('Principled BSDF');links.new(col.outputs['Color'],p.inputs['Base Color'])
    noise=nodes.new('ShaderNodeTexNoise');noise.inputs['Scale'].default_value=75;noise.inputs['Detail'].default_value=2
    bump=nodes.new('ShaderNodeBump');bump.inputs['Strength'].default_value=.20;bump.inputs['Distance'].default_value=.08
    links.new(noise.outputs['Fac'],bump.inputs['Height']);links.new(bump.outputs['Normal'],p.inputs['Normal'])
scene=bpy.context.scene;scene.render.engine='CYCLES';scene.cycles.samples=24;scene.cycles.use_denoising=True
scene.world.color=(.2,.2,.2)
bpy.ops.mesh.primitive_plane_add(size=5000,location=(0,0,-2))
ground=bpy.context.object;m=bpy.data.materials.new('PreviewGround');m.diffuse_color=(.07,.085,.079,1);ground.data.materials.append(m)
for pos,power,size in [((200,-600,1100),25000000,600),((-500,300,800),16000000,650),((500,500,500),9000000,350)]:
    bpy.ops.object.light_add(type='AREA',location=pos);l=bpy.context.object;l.data.energy=power;l.data.shape='DISK';l.data.size=size
    l.rotation_euler=(Vector((0,0,200))-l.location).to_track_quat('-Z','Y').to_euler()
bpy.ops.object.camera_add(location=(820,-1400,700));cam=bpy.context.object
cam.rotation_euler=(Vector((-55,0,210 if species=='Trex' else 170))-cam.location).to_track_quat('-Z','Y').to_euler()
cam.data.type='ORTHO';cam.data.ortho_scale=1320 if species=='Trex' else 1180 if species=='Trike' else 700
cam.data.clip_end=10000
scene.camera=cam;scene.render.resolution_x=1200;scene.render.resolution_y=800;scene.render.resolution_percentage=100
scene.view_settings.view_transform='AgX'
out=root/'Tests/Art';out.mkdir(parents=True,exist_ok=True);scene.render.filepath=str(out/f'{species}_preview.png')
bpy.ops.render.render(write_still=True)
