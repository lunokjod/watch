#!/bin/python3
import bpy
import os

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



# https://gist.github.com/ajfisher/2834170
def rgb_hex565(red, green, blue):
    # take in the red, green and blue values (0-255) as 8 bit values and then combine
    # and shift them to make them a 16 bit hex value in 565 format. 
    return ("0x%0.4X" % ((int(red / 255 * 31) << 11) | (int(green / 255 * 63) << 5) | (int(blue / 255 * 31))))

#entrypoint
if __name__ == '__main__':
    obj = bpy.context.selected_objects[0]
    CMeshName=''.join([i for i in obj.name if i.isalpha() or i.isnumeric() ])
    mesh = obj.data

    filepath = bpy.data.filepath
    directory = os.path.dirname(filepath)
    outputFile = directory+"/"+CMeshName+".c"
    #mesh.location #Object position rotation, scale
    #mesh.rotation
    #mesh.scale
    with open(outputFile, 'w') as f:
        #f.write("#include \"../src/UI/controls/View3D.hpp\"\n")
        #f.write("myPointsNumber = %d;\n" % len(mesh.vertices))
        #f.write("myFacesNumber = %d;\n" % len(mesh.polygons))
        #f.write("myNormalsNumber = %d;\n" % len(mesh.polygons))
        counter=0
        f.write("const LuI::RawVertex3D %sVertex[%d] = { " % (CMeshName,len(mesh.vertices)))
        for vert in mesh.vertices:
            f.write("{%f," % (vert.co.x))
            f.write("%f," % (vert.co.y))
            f.write("%f}, " % (vert.co.z))
            counter+=1
        f.write(" };\n")
        faceCount=0
        f.write("const LuI::RawFace3D %sFace[%d] = { " % (CMeshName,len(mesh.polygons)))
        for face in mesh.polygons:
            slot = obj.material_slots[face.material_index]
            mat = slot.material
            f.write("{")
            if mat is not None:
                rc=255*mat.diffuse_color[0]
                gc=255*mat.diffuse_color[1]
                bc=255*mat.diffuse_color[2]
                rgb565c = rgb_hex565(rc,gc,bc)
                f.write(rgb565c + ",")
                #@TODO CONVERT TO RGB565
                #f.write("tft->color565(%d,%d,%d)," % (rc,gc,bc))
            else:
                f.write("TFT_DARKGREY,")
            if (face.use_smooth):
                f.write("true,")
            else:
                f.write("false,")
            vertCount=0
            f.write("{")
            for vert in face.vertices:
                f.write("%d," % (vert))
                vertCount+=1
            f.write("},")
            f.write("},")
        f.write("};\n")

        f.write("const LuI::RawNormal3D %sNormal[%d] = { " % (CMeshName,len(mesh.polygons)))
        normalCount=0
        for face in mesh.polygons:
            f.write("{")
            for vert in face.normal:
                f.write("%f," % vert)
            normalCount+=1
            f.write("},")
        f.write("};\n")
        f.write("const LuI::Mesh3DDescriptor %sMesh = {" % CMeshName)
        f.write("%d," % len(mesh.vertices))
        f.write("%d," % len(mesh.polygons))
        f.write("%sVertex," % CMeshName)
        f.write("%sFace," % CMeshName)
        f.write("%sNormal};\n" % CMeshName)
