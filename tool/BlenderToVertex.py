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
    obj = bpy.data.objects["Sphere"]  # particular object by name
    #obj = bpy.context.scene.objects.active
    mesh = obj.data
    with open('/home/sharek/Documents/PlatformIO/Projects/LWatch2/static/meshData.c', 'w') as f:
        f.write("myPointsNumber = %d;\n" % len(mesh.vertices))
        f.write("myFacesNumber = %d;\n" % len(mesh.polygons))
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
            vertCount=0
            for vert in face.vertices:
                f.write("myFaces[%d].vertexData[%d] = " % (faceCount,vertCount))
                f.write("&myPoints[%d];\n" % vert)
                vertCount+=1
            faceCount+=1
