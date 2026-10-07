"""Render actual original 0.5 models and animation review sheets."""
import bpy,pathlib,sys,math,os
from mathutils import Vector
root=pathlib.Path(__file__).resolve().parents[2]
collection=os.environ.get('DINO_ART_COLLECTION','Roster05')
for kind in (sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else ['Anky','Pachy','Brachi']):
    bpy.ops.wm.open_mainfile(filepath=str(root/'Assets/Source'/collection/(kind+'.blend')))
    scene=bpy.context.scene;scene.render.engine='CYCLES';scene.cycles.samples=12;scene.cycles.use_denoising=True;scene.render.film_transparent=True;scene.world.color=(.23,.23,.23)
    target=Vector((-80,0,480 if kind=='Brachi' else 230 if kind=='Alberto' else 145));size=1800 if kind=='Brachi' else 1100 if kind=='Alberto' else 1000 if kind=='Anky' else 760
    for pos,power in [((600,-1100,1900),22000000),((-700,500,1300),14000000),((500,700,600),10000000)]:
        bpy.ops.object.light_add(type='AREA',location=pos);o=bpy.context.object;o.data.energy=power;o.data.size=900;o.rotation_euler=(target-o.location).to_track_quat('-Z','Y').to_euler()
    bpy.ops.object.camera_add(location=target+Vector((800,-2000,700)));cam=bpy.context.object;cam.rotation_euler=(target-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.type='ORTHO';cam.data.ortho_scale=size;cam.data.clip_end=10000;scene.camera=cam
    scene.render.resolution_x=960;scene.render.resolution_y=640;scene.render.resolution_percentage=100;scene.render.image_settings.file_format='PNG';scene.render.image_settings.color_mode='RGBA';scene.view_settings.view_transform='AgX'
    out=root/'Tests/Art'/collection;out.mkdir(parents=True,exist_ok=True)
    rig=bpy.data.objects['Rig_'+kind]
    for clip,frame in [('Idle',1),('Heavy',24),('Walk',10),('Eat',15),('Death',60)]:
        rig.animation_data.action=bpy.data.actions[kind+'_'+clip];scene.frame_set(frame);scene.render.filepath=str(out/(kind+'_'+clip+'.png'));bpy.ops.render.render(write_still=True)
    print('ROSTER05_REVIEW_RENDERED',kind,flush=True)
