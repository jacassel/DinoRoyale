"""Bake original layered PBR skin onto the retained rigs; preserve original sources.

Blender --background --python Tools/Blender/modernize_dinosaurs.py -- Trex Raptor Trike
Only the mesh is exported. Existing Unreal animation sequences are never replaced.
"""
import bpy, bmesh, math, pathlib, sys, json, random
from mathutils import Vector

ROOT=pathlib.Path(__file__).resolve().parents[2]
SRC=ROOT/'Assets/Source/DinosaursModern'
OUT=ROOT/'Assets/Export/DinosaursModern'
SRC.mkdir(parents=True,exist_ok=True);OUT.mkdir(parents=True,exist_ok=True)

def node(mat,kind,**attrs):
    n=mat.node_tree.nodes.new('ShaderNode'+kind)
    for k,v in attrs.items():setattr(n,k,v)
    return n

def connect(mat,a,out,b,inp):mat.node_tree.links.new(a.outputs[out],b.inputs[inp])

def mathn(mat,op,a,b=None):
    n=node(mat,'Math',operation=op)
    if isinstance(a,tuple):connect(mat,a[0],a[1],n,0)
    else:n.inputs[0].default_value=a
    if isinstance(b,tuple):connect(mat,b[0],b[1],n,1)
    elif b is not None:n.inputs[1].default_value=b
    return n

def ramp(mat,source,stops):
    r=node(mat,'ValToRGB')
    for i,(pos,col) in enumerate(stops):
        e=r.color_ramp.elements[i] if i<2 else r.color_ramp.elements.new(pos)
        e.position=pos;e.color=(*col,1)
    connect(mat,*source,r,'Fac');return r

def noise(mat,coords,scale,detail=3,rough=.7):
    n=node(mat,'TexNoise');n.inputs['Scale'].default_value=scale;n.inputs['Detail'].default_value=detail;n.inputs['Roughness'].default_value=rough
    connect(mat,*coords,n,'Vector');return n

def mix(mat,a,b,factor):
    n=node(mat,'MixRGB');n.blend_type='MIX'
    for inp,value in [(1,a),(2,b),(0,factor)]:
        if isinstance(value,tuple) and hasattr(value[0],'outputs'):connect(mat,value[0],value[1],n,inp)
        elif isinstance(value,(tuple,list)):n.inputs[inp].default_value=(*value,1)
        else:n.inputs[inp].default_value=value
    return n

def configure(mat,kind):
    mat.use_nodes=True;mat.node_tree.nodes.clear()
    out=node(mat,'OutputMaterial');p=node(mat,'BsdfPrincipled');connect(mat,p,'BSDF',out,'Surface')
    coords=node(mat,'TexCoord');coord=(coords,'Object')
    # Object coordinates are the retained centimetre-space mesh coordinates.
    broad=noise(mat,coord,.012,3)
    fine=noise(mat,coord,1.3,2)
    sep=node(mat,'SeparateXYZ');connect(mat,*coord,sep,'Vector')
    name=mat.name.split('.')[0]
    if name=='Dino_Skin':
        colors={'Trex':[(.067,.066,.044),(.125,.105,.066),(.20,.164,.108)],
                'Raptor':[(.043,.033,.020),(.125,.085,.044),(.24,.178,.093)],
                'Trike':[(.075,.069,.054),(.135,.117,.090),(.20,.174,.128)]}[kind]
        # Warped transverse bars and dapple, laid out in anatomy space rather than UV islands.
        warp=mathn(mat,'MULTIPLY',(broad,'Fac'),12.0)
        xpos=mathn(mat,'MULTIPLY',(sep,'X'),.058 if kind!='Raptor' else .115)
        phase=mathn(mat,'ADD',(xpos,0),(warp,0));wave=mathn(mat,'SINE',(phase,0))
        bars=mathn(mat,'MULTIPLY_ADD',(wave,0),.5);bars.inputs[2].default_value=.5
        stripe=ramp(mat,(bars,0),[(.32,colors[0]),(.57,colors[1]),(.85,colors[2])])
        dapple=ramp(mat,(broad,'Fac'),[(.23,colors[0]),(.51,colors[1]),(.76,colors[2])])
        body=mix(mat,(stripe,'Color'),(dapple,'Color'),.88 if kind=='Trike' else .65 if kind=='Trex' else .35)
        # Countershading uses original rest normals: subtle cream on downward-facing hide.
        geom=node(mat,'NewGeometry');norm=node(mat,'SeparateXYZ');connect(mat,geom,'Normal',norm,'Vector')
        under=mathn(mat,'MULTIPLY_ADD',(norm,'Z'),-.8);under.inputs[2].default_value=.05
        under.use_clamp=True
        base=mix(mat,(body,'Color'),(.30,.257,.181),(under,0))
        scales=node(mat,'TexVoronoi',feature='DISTANCE_TO_EDGE');scales.inputs['Scale'].default_value=.40 if kind!='Raptor' else .68
        scales.inputs['Randomness'].default_value=.85;connect(mat,*coord,scales,'Vector')
        scale_height=ramp(mat,(scales,'Distance'),[(.018,(.12,)*3),(.075,(.65,)*3),(.21,(.80,)*3)])
        # Broad, irregular creases remain visible at normal gameplay distances.
        foldcoords=node(mat,'VectorMath',operation='MULTIPLY');connect(mat,*coord,foldcoords,0);foldcoords.inputs[1].default_value=(.35,.7,2.5)
        folds=noise(mat,(foldcoords,0),.055,2)
        wrinkle=ramp(mat,(folds,'Fac'),[(.37,(.17,)*3),(.46,(.50,)*3),(.59,(.64,)*3)])
        bump1=node(mat,'Bump');bump1.inputs['Strength'].default_value=.32;bump1.inputs['Distance'].default_value=1.5
        connect(mat,wrinkle,'Color',bump1,'Height')
        bump2=node(mat,'Bump');bump2.inputs['Strength'].default_value=.42;bump2.inputs['Distance'].default_value=.5
        connect(mat,scale_height,'Color',bump2,'Height');connect(mat,bump1,'Normal',bump2,'Normal')
        bump3=node(mat,'Bump');bump3.inputs['Strength'].default_value=.13;bump3.inputs['Distance'].default_value=.14
        connect(mat,fine,'Fac',bump3,'Height');connect(mat,bump2,'Normal',bump3,'Normal');connect(mat,bump3,'Normal',p,'Normal')
        detail=ramp(mat,(scales,'Distance'),[(.015,(.45,)*3),(.085,(1,)*3)])
        mul=node(mat,'MixRGB',blend_type='MULTIPLY');mul.inputs[0].default_value=.43;connect(mat,base,'Color',mul,1);connect(mat,detail,'Color',mul,2)
        connect(mat,mul,'Color',p,'Base Color')
        rough=ramp(mat,(fine,'Fac'),[(.2,(.55,)*3),(.8,(.85,)*3)]);connect(mat,rough,'Color',p,'Roughness')
    elif name in ('Dino_Horn','Dino_Tooth','Dino_Claw'):
        palette={'Dino_Horn':[(.075,.063,.043),(.37,.285,.16),(.65,.55,.35)],
                 'Dino_Tooth':[(.24,.17,.082),(.62,.52,.32),(.83,.75,.55)],
                 'Dino_Claw':[(.015,.013,.009),(.065,.053,.034),(.20,.155,.082)]}[name]
        r=ramp(mat,(broad,'Fac'),[(.15,palette[0]),(.5,palette[1]),(.85,palette[2])]);connect(mat,r,'Color',p,'Base Color')
        grooves=node(mat,'VectorMath',operation='MULTIPLY');connect(mat,*coord,grooves,0);grooves.inputs[1].default_value=(.25,3.0,.6)
        grain=noise(mat,(grooves,0),.42,2)
        bump=node(mat,'Bump');bump.inputs['Distance'].default_value=.20;bump.inputs['Strength'].default_value=.23
        connect(mat,grain,'Fac',bump,'Height');connect(mat,bump,'Normal',p,'Normal');p.inputs['Roughness'].default_value=.42 if name=='Dino_Horn' else .3
    elif name=='Dino_Mouth':
        r=ramp(mat,(broad,'Fac'),[(.2,(.022,.003,.004)),(.8,(.18,.033,.027))]);connect(mat,r,'Color',p,'Base Color');p.inputs['Roughness'].default_value=.27
    elif name=='Dino_Eye':
        r=ramp(mat,(fine,'Fac'),[(.2,(.035,.018,.005)),(.48,(.25,.105,.016)),(.8,(.55,.285,.045))]);connect(mat,r,'Color',p,'Base Color');p.inputs['Roughness'].default_value=.12
    else:
        p.inputs['Base Color'].default_value=(.004,.003,.002,1);p.inputs['Roughness'].default_value=.18
    return p,out

def add_tongue(kind,mesh):
    if kind=='Trike':return mesh
    small=kind=='Raptor'
    bpy.ops.mesh.primitive_uv_sphere_add(segments=32,ring_count=16,location=(151 if small else 334,0,192 if small else 381))
    obj=bpy.context.object;obj.name='Tongue';obj.scale=(24,10,3) if small else (65,29,7)
    bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    obj.data.materials.append(bpy.data.materials['Dino_Mouth'])
    g=obj.vertex_groups.new(name='jaw');g.add(list(range(len(obj.data.vertices))),1,'REPLACE')
    for p in obj.data.polygons:p.use_smooth=True
    bpy.ops.object.select_all(action='DESELECT');obj.select_set(True);mesh.select_set(True);bpy.context.view_layer.objects.active=mesh;bpy.ops.object.join()
    return mesh

def refine_face(kind,mesh):
    """Small edits to the existing disconnected eye/socket/tooth pieces, never the jaw."""
    skin=next(i for i,m in enumerate(mesh.data.materials) if m.name.startswith('Dino_Skin'))
    # Original eye surrounds were black ellipsoids, giving every animal cartoon goggles.
    centers={'Trex':(274,66,455),'Raptor':(129,21,221),'Trike':(256,64,199)}
    ex,ey,ez=centers[kind]
    for face in mesh.data.polygons:
        material=mesh.data.materials[face.material_index].name
        center=face.center
        if material.startswith('Dino_Claw') and abs(center.x-ex)<30 and abs(abs(center.y)-ey)<15 and abs(center.z-ez)<23:
            face.material_index=skin
    parents=list(range(len(mesh.data.vertices)))
    def find(i):
        while parents[i]!=i:parents[i]=parents[parents[i]];i=parents[i]
        return i
    for edge in mesh.data.edges:
        a,b=edge.vertices;parents[find(a)]=find(b)
    groups={}
    for v in mesh.data.vertices:groups.setdefault(find(v.index),[]).append(v.index)
    face_material={}
    for face in mesh.data.polygons:face_material[find(face.vertices[0])]=mesh.data.materials[face.material_index].name
    rng=random.Random(142)
    for key,ids in groups.items():
        name=face_material.get(key,'');verts=[mesh.data.vertices[i] for i in ids]
        center=sum((v.co for v in verts),Vector())/len(verts)
        if name.startswith('Dino_Eye'):
            for v in verts:
                v.co=center+(v.co-center)*.83
                v.co.y+=(1 if center.y>0 else -1)*(1.7 if kind=='Trex' else .65)
        elif name.startswith('Dino_Tooth'):
            # Retain tooth bases and give tips nonidentical lengths and slight curvature.
            zlo=min(v.co.z for v in verts);zhi=max(v.co.z for v in verts)
            jaw_index=mesh.vertex_groups['jaw'].index
            lower=any(g.group==jaw_index and g.weight>.5 for g in verts[0].groups)
            anchor=zlo if lower else zhi;length=rng.uniform(.54,.82) if lower else rng.uniform(.78,1.10)
            for v in verts:
                delta=v.co.z-anchor;v.co.z=anchor+delta*length;v.co.x-=abs(delta)*.10
                if lower:v.co.x+=1.6 if kind=='Raptor' else 3.6
    mesh.data.update()
    bm=bmesh.new();bm.from_mesh(mesh.data)
    bmesh.ops.dissolve_degenerate(bm,dist=.00001,edges=list(bm.edges))
    bmesh.ops.recalc_face_normals(bm,faces=list(bm.faces))
    bm.to_mesh(mesh.data);bm.free();mesh.data.update()

def run(kind):
    bpy.ops.wm.open_mainfile(filepath=str(ROOT/'Assets/Source/DinosaursRefined'/(kind+'.blend')))
    mesh=bpy.data.objects['SK_'+kind];rig=bpy.data.objects['Rig_'+kind]
    signature=[(b.name,list(b.head_local),list(b.tail_local)) for b in rig.data.bones]
    rig.animation_data.action=None
    for p in rig.pose.bones:p.rotation_euler=(0,0,0);p.location=(0,0,0)
    bpy.context.scene.frame_set(1)
    before=len(mesh.data.vertices)
    # Smooth only the hide, keeping jaw/teeth silhouettes and all original rig weights.
    skin_indices={i for i,m in enumerate(mesh.data.materials) if m.name.startswith('Dino_Skin')}
    skin_vertices={v for p in mesh.data.polygons if p.material_index in skin_indices for v in p.vertices}
    jaw=mesh.vertex_groups.get('jaw')
    skin_vertices={i for i in skin_vertices if not any(g.group==jaw.index and g.weight>.5 for g in mesh.data.vertices[i].groups)}
    smooth_group=mesh.vertex_groups.new(name='VisualHideSmoothing');smooth_group.add(list(skin_vertices),1,'REPLACE')
    bpy.context.view_layer.objects.active=mesh
    mod=mesh.modifiers.new('Gentle_hide_relax','SMOOTH');mod.factor=.65;mod.iterations=3;mod.vertex_group=smooth_group.name
    bpy.ops.object.modifier_apply(modifier=mod.name)
    if mesh.vertex_groups.get('VisualHideSmoothing'):mesh.vertex_groups.remove(mesh.vertex_groups['VisualHideSmoothing'])
    refine_face(kind,mesh)
    mesh=add_tongue(kind,mesh)
    bpy.ops.object.select_all(action='DESELECT');mesh.select_set(True);bpy.context.view_layer.objects.active=mesh
    # Repack once to accommodate the new mouth geometry. The skeleton is untouched.
    bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.uv.smart_project(angle_limit=math.radians(70),island_margin=.006);bpy.ops.object.mode_set(mode='OBJECT')
    mats=list(dict.fromkeys(mesh.data.materials));shaders={m:configure(m,kind) for m in mats}
    scene=bpy.context.scene;scene.render.engine='CYCLES';scene.cycles.samples=4
    scene.render.bake.margin=12;scene.render.bake.use_clear=True
    scene.render.bake.normal_g='NEG_Y'
    for channel in ['BaseColor','Roughness','Normal']:
        img=bpy.data.images.new(kind+'_'+channel,width=2048,height=2048,alpha=False)
        img.colorspace_settings.name='sRGB' if channel=='BaseColor' else 'Non-Color'
        for m,(p,out) in shaders.items():
            target=node(m,'TexImage');target.image=img;m.node_tree.nodes.active=target
            if channel!='Normal':
                emit=node(m,'Emission')
                inp=p.inputs['Base Color' if channel=='BaseColor' else 'Roughness']
                if inp.is_linked:m.node_tree.links.new(inp.links[0].from_socket,emit.inputs['Color'])
                elif channel=='BaseColor':emit.inputs['Color'].default_value=inp.default_value
                else:emit.inputs['Color'].default_value=(*([inp.default_value]*3),1)
                connect(m,emit,'Emission',out,'Surface')
            else:connect(m,p,'BSDF',out,'Surface')
        bpy.ops.object.bake(type='NORMAL' if channel=='Normal' else 'EMIT')
        img.filepath_raw=str(OUT/(kind+'_'+channel+'.png'));img.file_format='PNG';img.save();img.pack()
        print('BAKED',kind,channel,flush=True)
    for m,(p,out) in shaders.items():connect(m,p,'BSDF',out,'Surface')
    bpy.ops.object.select_all(action='DESELECT');mesh.select_set(True);rig.select_set(True);bpy.context.view_layer.objects.active=rig
    bpy.ops.export_scene.fbx(filepath=str(OUT/(kind+'.fbx')),use_selection=True,object_types={'ARMATURE','MESH'},add_leaf_bones=False,axis_forward='-Y',axis_up='Z',apply_unit_scale=True,use_mesh_modifiers=True,mesh_smooth_type='FACE',primary_bone_axis='Y',secondary_bone_axis='X',bake_anim=False)
    assert signature==[(b.name,list(b.head_local),list(b.tail_local)) for b in rig.data.bones]
    rig.animation_data.action=bpy.data.actions.get(kind+'_Idle')
    bpy.ops.wm.save_as_mainfile(filepath=str(SRC/(kind+'.blend')))
    report=dict(species=kind,verticesBefore=before,verticesAfter=len(mesh.data.vertices),bones=len(signature),skeletonUnchanged=True,animationsReexported=False)
    (OUT/(kind+'_audit.json')).write_text(json.dumps(report,indent=2));print('MODERN_DINOSAUR',report,flush=True)

if __name__=='__main__':
    wanted=sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else ['Trex','Raptor','Trike']
    for kind in wanted:run(kind)
