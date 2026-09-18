import bpy,pathlib,collections
ROOT=pathlib.Path(__file__).resolve().parents[2]
bpy.ops.wm.open_mainfile(filepath=str(ROOT/'Assets/Source/Dinosaurs/Trex.blend'))
mesh=bpy.data.objects['SK_Trex'].data
print('COLOR_ATTRIBUTES',[(a.name,a.domain,a.data_type,len(a.data)) for a in mesh.color_attributes])
for a in mesh.color_attributes:
    colors=[tuple(v.color) for v in a.data];print('COLOR',a.name,'first',colors[:5],'mean',[sum(c[k] for c in colors)/len(colors) for k in range(4)])
print('MATERIALS',[(i,m.name) for i,m in enumerate(mesh.materials)])
print('SLOTS',collections.Counter(p.material_index for p in mesh.polygons))
