"""Regenerate original clips without rebuilding the saved meshes or their UV textures."""
import bpy,pathlib,importlib.util,json
root=pathlib.Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('dino_generator',root/'Tools/Blender/create_dinosaurs.py')
gen=importlib.util.module_from_spec(spec);spec.loader.exec_module(gen)
checks=[]
for folder in ['Dinosaurs','DinosaursRefined']:
    for kind in ['Trex','Raptor','Trike','Prey']:
        source=root/'Assets/Source'/folder/(kind+'.blend')
        if not source.exists():continue
        bpy.ops.wm.open_mainfile(filepath=str(source))
        rig=bpy.data.objects['Rig_'+kind];mesh=next(o for o in bpy.data.objects if o.type=='MESH' and o.parent==rig)
        rig.animation_data_clear()
        for action in list(bpy.data.actions):bpy.data.actions.remove(action)
        gen.make_actions(rig,kind)
        out=root/'Assets/Export'/folder
        bpy.ops.object.select_all(action='DESELECT');rig.select_set(True);mesh.select_set(True);bpy.context.view_layer.objects.active=rig
        for action in list(bpy.data.actions):
            rig.animation_data.action=action
            bpy.context.scene.frame_start=int(action.frame_range[0]);bpy.context.scene.frame_end=int(action.frame_range[1])
            bpy.context.scene.frame_set(round((action.frame_range[0]+action.frame_range[1])*.5))
            # Bone-relative jaw opening must rotate its forward axis toward the floor.
            if action.name.endswith(('_Charge','_Heavy','_Quick')):
                from mathutils import Vector
                jaw=rig.pose.bones['jaw'];head=rig.pose.bones['head']
                relative=head.matrix.inverted()@jaw.matrix
                # Compare a local jaw tip transformed through the animated skeleton with closed jaw.
                local=jaw.bone.matrix_local.inverted()@Vector((jaw.bone.head_local.x+100,0,jaw.bone.head_local.z))
                actual=head.matrix.inverted()@(jaw.matrix@local)
                closed=head.bone.matrix_local.inverted()@(jaw.bone.matrix_local@local)
                opening=(head.bone.matrix_local.to_3x3()@(actual-closed)).z
                checks.append(dict(species=kind,source=folder,clip=action.name,tipDeltaZ=opening,passed=opening<-1 if kind!='Trike' else True))
            bpy.ops.export_scene.fbx(filepath=str(out/(action.name+'.fbx')),use_selection=True,object_types={'ARMATURE','MESH'},add_leaf_bones=False,axis_forward='-Y',axis_up='Z',apply_unit_scale=True,use_mesh_modifiers=True,mesh_smooth_type='FACE',primary_bone_axis='Y',secondary_bone_axis='X',bake_anim=True,bake_anim_use_all_actions=False,bake_anim_use_nla_strips=False,bake_anim_simplify_factor=0)
        rig.animation_data.action=bpy.data.actions.get(kind+'_Idle');bpy.context.scene.frame_start=1;bpy.context.scene.frame_end=60;bpy.context.scene.frame_set(1)
        bpy.ops.wm.save_as_mainfile(filepath=str(source));print('ANIMATION_UPDATE_COMPLETE',folder,kind,flush=True)
(root/'Tests/Results/blender-jaw-checks.json').write_text(json.dumps(checks,indent=2))
print('JAW_CHECKS',sum(r['passed'] for r in checks),'/',len(checks),flush=True)
