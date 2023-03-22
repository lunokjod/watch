#include "../src/UI/controls/View3D.hpp"
const LuI::RawVertex3D CubeVertex[] = { {-59.170410,-59.170414,-59.170414}, {-59.170410,-59.170414,59.170414}, {-59.170410,59.170414,-59.170414}, {-59.170410,59.170414,59.170414}, {59.170414,-59.170414,-59.170414}, {59.170414,-59.170414,59.170414}, {59.170414,59.170414,-59.170414}, {59.170414,59.170414,59.170414},  };
const LuI::RawFace3D CubeFace[] = { {tft->color565(204,204,204),false,{1,2,0,},},{tft->color565(204,204,204),false,{3,6,2,},},{tft->color565(204,204,204),false,{7,4,6,},},{tft->color565(204,204,204),false,{5,0,4,},},{tft->color565(204,204,204),false,{6,0,2,},},{tft->color565(204,204,204),false,{3,5,7,},},{tft->color565(204,204,204),false,{1,3,2,},},{tft->color565(204,204,204),false,{3,7,6,},},{tft->color565(204,204,204),false,{7,5,4,},},{tft->color565(204,204,204),false,{5,1,0,},},{tft->color565(204,204,204),false,{6,4,0,},},{tft->color565(204,204,204),false,{3,1,5,},},};
const LuI::RawNormal3D CubeNormal[] = { {-1.000000,0.000000,0.000000,},{0.000000,1.000000,-0.000000,},{1.000000,0.000000,-0.000000,},{0.000000,-1.000000,0.000000,},{0.000000,0.000000,-1.000000,},{0.000000,0.000000,1.000000,},{-1.000000,0.000000,0.000000,},{0.000000,1.000000,-0.000000,},{1.000000,0.000000,0.000000,},{0.000000,-1.000000,0.000000,},{0.000000,0.000000,-1.000000,},{0.000000,-0.000000,1.000000,},};
const LuI::Mesh3DDescriptor CubeMesh = {8,12,CubeVertex,CubeFace,CubeNormal};
