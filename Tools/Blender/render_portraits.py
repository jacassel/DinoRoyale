"""Render original refined dinosaur portraits for the opening selector."""
import bpy,pathlib,math
from mathutils import Vector
root=pathlib.Path(__file__).resolve().parents[2]
for kind in ['Trex','Raptor','Trike']:
    bpy.ops.wm.open_mainfile(filepath=str(root/'Assets/Source/DinosaursRefined'/(kind+'.blend')))
    scene=bpy.context.scene;scene.render.engine='CYCLES';scene.cycles.samples=12;scene.cycles.use_denoising=True;scene.render.film_transparent=True
    scene.world.color=(.17,.17,.17)
    small=kind=='Raptor';target=Vector((-45,0,170 if small else 210))
    for pos,power,size in [((300,-700,1200),12000000,800),((-500,500,700),9000000,700),((500,500,500),5000000,450)]:
        bpy.ops.object.light_add(type='AREA',location=pos);o=bpy.context.object;o.data.energy=power;o.data.shape='DISK';o.data.size=size;o.rotation_euler=(target-o.location).to_track_quat('-Z','Y').to_euler()
    bpy.ops.object.camera_add(location=(760,-1600,660));camera=bpy.context.object;camera.rotation_euler=(target-camera.location).to_track_quat('-Z','Y').to_euler();camera.data.type='ORTHO';camera.data.ortho_scale=740 if small else 1250 if kind=='Trex' else 1100;camera.data.clip_end=10000
    scene.camera=camera;scene.render.resolution_x=768;scene.render.resolution_y=432;scene.render.resolution_percentage=100;scene.view_settings.view_transform='AgX'
    scene.render.image_settings.file_format='PNG';scene.render.image_settings.color_mode='RGBA';scene.render.filepath=str(root/'Assets/Export/UI'/('T_'+kind+'Portrait.png'));bpy.ops.render.render(write_still=True)
    print('PORTRAIT_COMPLETE',kind,flush=True)
