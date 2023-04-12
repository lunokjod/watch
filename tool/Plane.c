const LuI::RawVertex3D PlaneVertex[4] = { {-33.957581,-33.957581,0.000000}, {33.957581,-33.957581,0.000000}, {-33.957581,33.957581,0.000000}, {33.957581,33.957581,0.000000},  };
const LuI::RawFace3D PlaneFace[2] = { {0xC658,false,{1,2,0,},},{0xC658,false,{1,3,2,},},};
const LuI::RawNormal3D PlaneNormal[2] = { {-0.000000,0.000000,1.000000,},{-0.000000,0.000000,1.000000,},};
const LuI::Mesh3DDescriptor PlaneMesh = {4,2,PlaneVertex,PlaneFace,PlaneNormal};
