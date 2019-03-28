#pragma once
#include "../gl/depend.hpp"
#include <boost/preprocessor.hpp>

#define SEQ_GLTYPE (void)(bool)(int)(float)(vec2)(vec3)(vec4)(ivec2)(ivec3)(ivec4)(bvec2)(bvec3)(bvec4)(mat2)(mat3)(mat4)(sampler2D)(samplerCube)
#define SEQ_PRECISION (highp)(mediump)(lowp)

#define SEQ_GLSETTING ((linewidth,glLineWidth,float))((frontface,glFrontFace,unsigned))((cullfacedir,glCullFace,unsigned))((depthrange,GLDEPTHRANGE,float,float))((viewport,glViewport,float,float,float,float))\
		((scissor,glScissor,float,float,float,float))((samplecoverage,glSampleCoverage,float,bool))((stencilfunc,glStencilFunc,unsigned,float,unsigned))((stencilfuncfront,stencilFuncFront,unsigned,float,unsigned))\
		((stencilfuncback,stencilFuncBack,unsigned,float,unsigned))((stencilop,glStencilOp,unsigned,unsigned,unsigned))((stencilopfront,stencilOpFront,unsigned,unsigned,unsigned))((stencilopback,stencilOpBack,unsigned,unsigned,unsigned))\
		((depthfunc,glDepthFunc,unsigned))((blendeq,glBlendEquation,unsigned))((blendeqca,glBlendEquationSeparate,unsigned,unsigned))((blendfunc,glBlendFunc,unsigned,unsigned))\
		((blendfuncca,glBlendFuncSeparate,unsigned,unsigned,unsigned,unsigned))((blendcolor,glBlendColor,float,float,float,float))\
		((colormask,glColorMask,bool,bool,bool,bool))((depthmask,glDepthMask,bool))\
		((stencilmask,glStencilMask,unsigned))((stencilmaskfront,stencilMaskFront,unsigned))((stencilmaskback,stencilMaskBack,unsigned))\
		((polygonmode,polygonMode,unsigned))
#define EMPTY
