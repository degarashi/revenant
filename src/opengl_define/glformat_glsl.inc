const static std::pair<GLenum, GLSLFormatDesc> c_GLSLTypeList[] = {
	{GL_FLOAT, {GLSLType::FloatT, MakeDim(1), false, false}},
	{GL_FLOAT_VEC2, {GLSLType::FloatT, MakeDim(2), false, false}},
	{GL_FLOAT_VEC3, {GLSLType::FloatT, MakeDim(3), false, false}},
	{GL_FLOAT_VEC4, {GLSLType::FloatT, MakeDim(4), false, false}},
	{GL_INT, {GLSLType::IntT, MakeDim(1), false, false}},
	{GL_INT_VEC2, {GLSLType::IntT, MakeDim(2), false, false}},
	{GL_INT_VEC3, {GLSLType::IntT, MakeDim(3), false, false}},
	{GL_INT_VEC4, {GLSLType::IntT, MakeDim(4), false, false}},
	{GL_BOOL, {GLSLType::BoolT, MakeDim(1), false, false}},
	{GL_BOOL_VEC2, {GLSLType::BoolT, MakeDim(2), false, false}},
	{GL_BOOL_VEC3, {GLSLType::BoolT, MakeDim(3), false, false}},
	{GL_BOOL_VEC4, {GLSLType::BoolT, MakeDim(4), false, false}},
	{GL_FLOAT_MAT2, {GLSLType::MatrixT, MakeDim(2,2), false, false}},
	{GL_FLOAT_MAT3, {GLSLType::MatrixT, MakeDim(3,3), false, false}},
	{GL_FLOAT_MAT4, {GLSLType::MatrixT, MakeDim(4,4), false, false}},
	{GL_SAMPLER_2D, {GLSLType::TextureT, MakeDim(2), false, false}},
	{GL_SAMPLER_CUBE, {GLSLType::TextureT, MakeDim(2), false, true}},
#ifndef USE_OPENGLES2
	{GL_DOUBLE, {GLSLType::DoubleT, MakeDim(1), false, false}},
	{GL_DOUBLE_VEC2, {GLSLType::DoubleT, MakeDim(2), false, false}},
	{GL_DOUBLE_VEC3, {GLSLType::DoubleT, MakeDim(3), false, false}},
	{GL_DOUBLE_VEC4, {GLSLType::DoubleT, MakeDim(4), false, false}},
	{GL_UNSIGNED_INT, {GLSLType::IntT, MakeDim(1), true, false}},
	{GL_UNSIGNED_INT_VEC2, {GLSLType::IntT, MakeDim(2), true, false}},
	{GL_UNSIGNED_INT_VEC3, {GLSLType::IntT, MakeDim(3), true, false}},
	{GL_UNSIGNED_INT_VEC4, {GLSLType::IntT, MakeDim(4), true, false}},
	{GL_FLOAT_MAT2x3, {GLSLType::MatrixT, MakeDim(2,3), false, false}},
	{GL_FLOAT_MAT2x4, {GLSLType::MatrixT, MakeDim(2,4), false, false}},
	{GL_FLOAT_MAT3x2, {GLSLType::MatrixT, MakeDim(3,2), false, false}},
	{GL_FLOAT_MAT3x4, {GLSLType::MatrixT, MakeDim(3,4), false, false}},
	{GL_FLOAT_MAT4x2, {GLSLType::MatrixT, MakeDim(4,2), false, false}},
	{GL_FLOAT_MAT4x3, {GLSLType::MatrixT, MakeDim(4,3), false, false}},
	{GL_DOUBLE_MAT2, {GLSLType::DMatrixT, MakeDim(2,2), false, false}},
	{GL_DOUBLE_MAT3, {GLSLType::DMatrixT, MakeDim(3,3), false, false}},
	{GL_DOUBLE_MAT4, {GLSLType::DMatrixT, MakeDim(4,4), false, false}},
	{GL_DOUBLE_MAT2x3, {GLSLType::DMatrixT, MakeDim(2,3), false, false}},
	{GL_DOUBLE_MAT2x4, {GLSLType::DMatrixT, MakeDim(2,4), false, false}},
	{GL_DOUBLE_MAT3x2, {GLSLType::DMatrixT, MakeDim(3,2), false, false}},
	{GL_DOUBLE_MAT3x4, {GLSLType::DMatrixT, MakeDim(3,4), false, false}},
	{GL_DOUBLE_MAT4x2, {GLSLType::DMatrixT, MakeDim(4,2), false, false}},
	{GL_DOUBLE_MAT4x3, {GLSLType::DMatrixT, MakeDim(4,3), false, false}},
	{GL_SAMPLER_1D, {GLSLType::TextureT, MakeDim(1), false, false}},
	{GL_SAMPLER_3D, {GLSLType::TextureT, MakeDim(3), false, false}},
	{GL_SAMPLER_1D_SHADOW, {GLSLType::TextureT, MakeDim(1), false, false}},
	{GL_SAMPLER_2D_SHADOW, {GLSLType::TextureT, MakeDim(2), false, false}},
	{GL_SAMPLER_1D_ARRAY, {GLSLType::TextureT, MakeDim(1), false, false}},
	{GL_SAMPLER_2D_ARRAY, {GLSLType::TextureT, MakeDim(2), false, false}},
	{GL_SAMPLER_1D_ARRAY_SHADOW, {GLSLType::TextureT, MakeDim(1), false, false}},
	{GL_SAMPLER_2D_ARRAY_SHADOW, {GLSLType::TextureT, MakeDim(2), false, false}},
	{GL_SAMPLER_2D_MULTISAMPLE, {GLSLType::TextureT, MakeDim(2), false, false}},
	{GL_SAMPLER_2D_MULTISAMPLE_ARRAY, {GLSLType::TextureT, MakeDim(2), false, false}},
	{GL_SAMPLER_CUBE_SHADOW, {GLSLType::TextureT, MakeDim(2), false, true}},
	{GL_SAMPLER_BUFFER, {GLSLType::TextureT, MakeDim(1), false, false}},
	{GL_SAMPLER_2D_RECT, {GLSLType::TextureT, MakeDim(2), false, false}},
	{GL_SAMPLER_2D_RECT_SHADOW, {GLSLType::TextureT, MakeDim(2), false, false}},
	{GL_INT_SAMPLER_1D, {GLSLType::IntTextureT, MakeDim(1), false, false}},
	{GL_INT_SAMPLER_2D, {GLSLType::IntTextureT, MakeDim(2), false, false}},
	{GL_INT_SAMPLER_3D, {GLSLType::IntTextureT, MakeDim(3), false, false}},
	{GL_INT_SAMPLER_CUBE, {GLSLType::IntTextureT, MakeDim(2), false, true}},
	{GL_INT_SAMPLER_1D_ARRAY, {GLSLType::IntTextureT, MakeDim(1), false, false}},
	{GL_INT_SAMPLER_2D_ARRAY, {GLSLType::IntTextureT, MakeDim(2), false, false}},
	{GL_INT_SAMPLER_2D_MULTISAMPLE, {GLSLType::IntTextureT, MakeDim(2), false, false}},
	{GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY, {GLSLType::IntTextureT, MakeDim(2), false, false}},
	{GL_INT_SAMPLER_BUFFER, {GLSLType::IntTextureT, MakeDim(1), false, false}},
	{GL_INT_SAMPLER_2D_RECT, {GLSLType::IntTextureT, MakeDim(2), false, false}},
	{GL_UNSIGNED_INT_SAMPLER_1D, {GLSLType::IntTextureT, MakeDim(1), true, false}},
	{GL_UNSIGNED_INT_SAMPLER_2D, {GLSLType::IntTextureT, MakeDim(2), true, false}},
	{GL_UNSIGNED_INT_SAMPLER_3D, {GLSLType::IntTextureT, MakeDim(3), true, false}},
	{GL_UNSIGNED_INT_SAMPLER_CUBE, {GLSLType::IntTextureT, MakeDim(2), true, true}},
	{GL_UNSIGNED_INT_SAMPLER_1D_ARRAY, {GLSLType::IntTextureT, MakeDim(1), true, false}},
	{GL_UNSIGNED_INT_SAMPLER_2D_ARRAY, {GLSLType::IntTextureT, MakeDim(2), true, false}},
	{GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE, {GLSLType::IntTextureT, MakeDim(2), true, false}},
	{GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY, {GLSLType::IntTextureT, MakeDim(2), true, false}},
	{GL_UNSIGNED_INT_SAMPLER_BUFFER, {GLSLType::IntTextureT, MakeDim(1), true, false}},
	{GL_UNSIGNED_INT_SAMPLER_2D_RECT, {GLSLType::IntTextureT, MakeDim(2), true, false}}
#endif
};
