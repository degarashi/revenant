#include "texture_filter.hpp"
#include "gl_if.hpp"

namespace rev {
	namespace {
		const GLenum cs_wrap[WrapState::_Num] = {
			GL_CLAMP_TO_EDGE,
			GL_CLAMP_TO_BORDER,
			GL_MIRRORED_REPEAT,
			GL_REPEAT,
			GL_MIRROR_CLAMP_TO_EDGE
		};
	}
	using FI = TextureFilter_In;
	FI::TextureFilter_In(
		const uint32_t	iLinearMag,
		const uint32_t	iLinearMin,
		const WrapState	wrapS,
		const WrapState	wrapT,
		const bool mipLinear,
		const float		coeff
	):
		_iLinearMag(iLinearMag),
		_iLinearMin(iLinearMin),
		_wrapS(wrapS),
		_wrapT(wrapT),
		_mipLinear(mipLinear),
		_coeff(coeff)
	{}
	bool FI::IsMipmap(const MipState level) {
		return level >= MipState::MipmapNear;
	}
	void FI::setFilter(const bool bLinearMag, const bool bLinearMin) {
		_iLinearMag = bLinearMag ? 1 : 0;
		_iLinearMin = bLinearMin ? 1 : 0;
	}
	void FI::setMagMinFilter(const bool bLinear) {
		setFilter(bLinear, bLinear);
	}
	void FI::setAnisotropicCoeff(const float coeff) {
		_coeff = coeff;
	}
	void FI::setUVWrap(WrapState s, WrapState t) {
		_wrapS = s;
		_wrapT = t;
	}
	void FI::setWrap(WrapState st) {
		setUVWrap(st, st);
	}
	void FI::setMipLinear(const bool bLinear) {
		_mipLinear = bLinear;
	}

	// --------------- TextureFilter::DCmd_Filter ---------------
	using F = TextureFilter;
	struct F::DCmd_Filter :
		TextureFilter_In
	{
		bool	hasMip;
		GLuint	texFlag;

		DCmd_Filter(const FI& src, bool hasmip, const GLuint flag);
		static void Command(const void* p);
	};
	F::DCmd_Filter::DCmd_Filter(const FI& src, const bool hasmip, const GLuint flag):
		FI(src),
		hasMip(hasmip),
		texFlag(flag)
	{}
	namespace {
		// [mipLevel][Nearest / Linear]
		const GLuint cs_Filter[3][2] = {
			{GL_NEAREST, GL_LINEAR},
			{GL_NEAREST_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_NEAREST},
			{GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR}
		};
	}
	void F::DCmd_Filter::Command(const void* p) {
		auto& self = *static_cast<const F::DCmd_Filter*>(p);

		uint32_t mipflag;
		if(!self.hasMip) {
			mipflag = 0;
		} else {
			mipflag = static_cast<uint32_t>(self._mipLinear) + 1;
		}

		// setAnisotropic
		if(self._coeff > 0) {
			GLfloat aMax;
			GL.glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aMax);
			GL.glTexParameteri(self.texFlag, GL_TEXTURE_MAX_ANISOTROPY_EXT, std::max(aMax * self._coeff, 1.f));
		}
		// setUVWrap
		GL.glTexParameteri(self.texFlag, GL_TEXTURE_WRAP_S, cs_wrap[self._wrapS]);
		GL.glTexParameteri(self.texFlag, GL_TEXTURE_WRAP_T, cs_wrap[self._wrapT]);
		// setFilter
		GL.glTexParameteri(self.texFlag, GL_TEXTURE_MAG_FILTER, cs_Filter[0][self._iLinearMag]);
		GL.glTexParameteri(self.texFlag, GL_TEXTURE_MIN_FILTER, cs_Filter[mipflag][self._iLinearMin]);
	}
	void F::dcmd_filter(draw::IQueue& q, const bool hasmip, const GLenum texFlag) const {
		q.add(DCmd_Filter(*this, hasmip, texFlag));
	}
	void F::imm_filter(const bool hasmip, const GLenum texFlag) const {
		const auto f = DCmd_Filter(*this, hasmip, texFlag);
		DCmd_Filter::Command(&f);
	}
}
