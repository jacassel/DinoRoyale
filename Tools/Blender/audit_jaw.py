"""Read-only audit of retained Blender jaw clips; never exports or saves assets."""
import bpy,pathlib,json,math
from mathutils import Vector
root=pathlib.Path(__file__).resolve().parents[2]
bpy.ops.wm.open_mainfile(filepath=str(root/'Assets/Source/DinosaursRefined/Trex.blend'))
rig=bpy.data.objects['Rig_Trex'];jaw=rig.pose.bones['jaw'];head=rig.pose.bones['head'];rows=[]
local=jaw.bone.matrix_local.inverted()@Vector((jaw.bone.head_local.x+100,0,jaw.bone.head_local.z))
closed=head.bone.matrix_local.inverted()@(jaw.bone.matrix_local@local)
for name in ['Trex_Charge','Trex_Heavy','Trex_Quick']:
 action=bpy.data.actions[name];rig.animation_data.action=action;deltas=[]
 for f in range(int(action.frame_range[0]),int(action.frame_range[1])+1):
  bpy.context.scene.frame_set(f);actual=head.matrix.inverted()@(jaw.matrix@local)
  delta=(head.bone.matrix_local.to_3x3()@(actual-closed)).z;deltas.append(float(delta))
 rows.append(dict(clip=name,frames=len(deltas),maxUpward=max(deltas),maxOpening=-min(deltas),passed=max(deltas)<.01 and min(deltas)<-1))
(root/'Tests/Results/jaw-full-frame-audit.json').write_text(json.dumps(rows,indent=2));print('JAW_AUDIT',rows,flush=True)
