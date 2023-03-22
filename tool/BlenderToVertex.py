#!/bin/python3
import bpy

def dump(obj, level=0):
   for attr in dir(obj):
       if hasattr( obj, "attr" ):
           print( "obj.%s = %s" % (attr, getattr(obj, attr)))
       else:
           print( attr )
# https://docs.blender.org/manual/en/latest/modeling/modifiers/generate/decimate.html
# https://blender.stackexchange.com/questions/6139/how-to-iterate-through-all-vertices-of-an-object-that-contains-multiple-meshes

#obj = bpy.context.scene.objects.active # active object

# dump(obj.data)

#print("# of faces=%d" % len(mesh.polygons))

#for face in mesh.polygons:
#    print('face')
#    #dump(face)
#    for vert in face.vertices:
#        print(vert)


#entrypoint
if __name__ == '__main__':
    obj = bpy.data.objects["head"]  # particular object by name
    #obj = bpy.context.scene.objects.active
    mesh = obj.data
    #mesh.location #Object position rotation, scale
    #mesh.rotation
    #mesh.scale
    with open('/home/sharek/Documents/PlatformIO/Projects/LWatch2/static/meshData.c', 'w') as f:
        f.write("myPointsNumber = %d;\n" % len(mesh.vertices))
        f.write("myFacesNumber = %d;\n" % len(mesh.polygons))
        f.write("myNormalsNumber = %d;\n" % len(mesh.polygons))
        #f.write("myVectorsNumber=%d;\n" % len(mesh.polygons))
        counter=0
        for vert in mesh.vertices:
            f.write("myPoints[%d].x=%f;\n" % (counter,vert.co.x))
            f.write("myPoints[%d].y=%f;\n" % (counter,vert.co.y))
            f.write("myPoints[%d].z=%f;\n" % (counter,vert.co.z))
            counter+=1
        faceCount=0
        for face in mesh.polygons:
            f.write("myFaces[%d].vertexNum = %d;\n" % (faceCount,len(face.vertices)))
            #face.material_index
            slot = obj.material_slots[face.material_index]
            mat = slot.material
            if mat is not None:
                rc=255*mat.diffuse_color[0]
                gc=255*mat.diffuse_color[1]
                bc=255*mat.diffuse_color[2]
                f.write("myFaces[%d].color = tft->color565(%d,%d,%d);\n" % (faceCount,rc,gc,bc))
            if (face.use_smooth):
                f.write("myFaces[%d].smooth = true;\n" % faceCount)
            else:
                f.write("myFaces[%d].smooth = false;\n" % faceCount)
            #f.write("myFaces[%d].color = %d;\n" % face.color)
            vertCount=0
            for vert in face.vertices:
                f.write("myFaces[%d].vertexData[%d] = " % (faceCount,vertCount))
                f.write("&myPoints[%d];\n" % vert)
                vertCount+=1
            normalCount=0
            for vert in face.normal:
                f.write("myNormals[%d].vertexData[%d] = " % (faceCount,normalCount))
                #obj.data.polygons[0].normal[0]
                f.write("%f;\n" % vert)
                normalCount+=1
            faceCount+=1
