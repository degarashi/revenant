#pragma once
#include "gl_types.hpp"
#include "drawcmd/queue_if.hpp"

namespace rev {
	class TextureFilter {
		private:
			uint32_t		_iLinearMag,	//!< Linearの場合は1, Nearestは0
							_iLinearMin;
			WrapState		_wrapS,
							_wrapT;
			MipState		_mipLevel;
			float			_coeff;

			struct DCmd_Filter;

		public:
			TextureFilter(
				uint32_t	iLinearMag = 0,
				uint32_t	iLinearMin = 0,
				WrapState	wrapS	= WrapState::Repeat,
				WrapState	wrapT	= WrapState::Repeat,
				MipState	mipLevel= MipState::NoMipmap,
				float		coeff	= 0
			);
			static bool IsMipmap(MipState level);
			bool isMipmap() const;
			void setFilter(bool bLinearMag, bool bLinearMin);
			void setMagMinFilter(bool bLinear);
			void setAnisotropicCoeff(float coeff);
			void setUVWrap(WrapState s, WrapState t);
			void setWrap(WrapState st);

			void dcmd_filter(draw::IQueue& q, GLenum texFlag) const;
			void imm_filter(GLenum texFlag) const;

			const char* getDebugName() const noexcept;
			bool property(bool edit);
	};
}
