"""Add six in-place stepping clips to COPIES of the modern rigs. Originals are untouched."""
import bpy,math,pathlib,json
from mathutils import Euler,Vector
root=pathlib.Path(__file__).resolve().parents[2]
out=root/'Assets/Export/Pivots';out.mkdir(parents=True,exist_ok=True)
sources=root/'Assets/Source/Pivots';sources.mkdir(parents=True,exist_ok=True)
report=[]
for kind,frames in [('Trex',34),('Raptor',22),('Trike',30)]:
    bpy.ops.wm.open_mainfile(filepath=str(root/'Assets/Source/DinosaursModern'/(kind+'.blend')))
    rig=bpy.data.objects['Rig_'+kind];rig.animation_data_create()
    original_actions=[a.name for a in bpy.data.actions]
    for direction,sign in [('Left',-1),('Right',1)]:
        action=bpy.data.actions.new(kind+'_Pivot'+direction);action.use_fake_user=True
        rig.animation_data.action=action
        for f in range(1,frames+1):
            t=(f-1)/(frames-1);cycle=t*2*math.pi
            for bone in rig.pose.bones:
                bone.rotation_mode='XYZ';bone.rotation_euler=(0,0,0);bone.location=(0,0,0)
            def rotate(name,pitch=0,roll=0,yaw=0):
                if name not in rig.pose.bones:return
                b=rig.pose.bones[name];basis=b.bone.matrix_local.to_quaternion()
                q=Euler(tuple(math.radians(a) for a in (roll,pitch,yaw)),'XYZ').to_quaternion()
                b.rotation_euler=(basis.inverted()@q@basis).to_euler('XYZ')
            # Alternating short steps and knee flexion, with the inside leg taking
            # a smaller step. Root stays in place; gameplay owns actor yaw.
            for side,phase in [('l',0),('r',math.pi)]:
                stroke=math.sin(cycle+phase);lift=max(0,stroke)
                outside=(side=='l')==(sign>0);stride=11 if outside else 6
                rotate('leg_'+side+'_upper',stride*stroke,0,sign*4*stroke)
                rotate('leg_'+side+'_lower',-19*lift)
                rotate('leg_'+side+'_foot',8*lift-stride*.3*stroke)
                if kind=='Trike':
                    rotate('arm_'+side+'_upper',-stride*stroke,0,sign*3*stroke)
                    rotate('arm_'+side+'_lower',-13*max(0,-stroke))
                    rotate('arm_'+side+'_foot',6*max(0,-stroke))
            rotate('spine',1.2*math.sin(cycle*2),sign*1.5,sign*2*math.sin(cycle))
            rotate('neck',0,0,-sign*2*math.sin(cycle))
            for j in range(1,5):rotate('tail_%02d'%j,0,0,-sign*(2+1.5*math.sin(cycle-j*.4)))
            for bone in rig.pose.bones:
                bone.keyframe_insert(data_path='rotation_euler',frame=f,group=bone.name)
                if bone.name=='root':bone.keyframe_insert(data_path='location',frame=f,group=bone.name)
        bpy.context.scene.render.fps=30;bpy.context.scene.frame_start=1;bpy.context.scene.frame_end=frames
        bpy.ops.object.select_all(action='DESELECT');rig.select_set(True);bpy.context.view_layer.objects.active=rig
        bpy.ops.export_scene.fbx(filepath=str(out/(action.name+'.fbx')),use_selection=True,object_types={'ARMATURE'},add_leaf_bones=False,axis_forward='-Y',axis_up='Z',apply_unit_scale=True,primary_bone_axis='Y',secondary_bone_axis='X',bake_anim=True,bake_anim_use_all_actions=False,bake_anim_use_nla_strips=False,bake_anim_simplify_factor=0)
        report.append(dict(species=kind,clip=action.name,frames=frames,bones=len(rig.data.bones),rootMotion=False,originalActionsRetained=all(bpy.data.actions.get(n) for n in original_actions)))
    rig.animation_data.action=bpy.data.actions.get(kind+'_Idle');bpy.context.scene.frame_set(1)
    bpy.ops.wm.save_as_mainfile(filepath=str(sources/(kind+'_Pivots.blend')))
(root/'Tests/Results/alpha03/pivot-assets.json').write_text(json.dumps(report,indent=2))
print('PIVOT_ASSETS_COMPLETE',flush=True)
