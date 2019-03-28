#pragma once
#include "../types.hpp"
#include "../../drawcmd/queue_if.hpp"

namespace rev {
	class TextureFilter_In {
		protected:
			uint32_t		_iLinearMag,	//!< Linearの場合は1, Nearestは0
							_iLinearMin;
			WrapState		_wrapS,
							_wrapT;
			bool			_mipLinear;
			float			_coeff;

		public:
			TextureFilter_In(
				uint32_t	iLinearMag = 0,
				uint32_t	iLinearMin = 0,
				WrapState	wrapS	= WrapState::Repeat,
				WrapState	wrapT	= WrapState::Repeat,
				bool		mipLinear = false,
				float		coeff	= 0
			);

			static bool IsMipmap(MipState level);
			void setFilter(bool bLinearMag, bool bLinearMin);
			void setMagMinFilter(bool bLinear);
			void setAnisotropicCoeff(float coeff);
			void setUVWrap(WrapState s, WrapState t);
			void setWrap(WrapState st);
			void setMipLinear(bool bLinear);
	};
	class TextureFilter :
		public TextureFilter_In,
		public IDebugGui
	{
		private:
			struct DCmd_Filter;

		public:
			using TextureFilter_In::TextureFilter_In;

			void dcmd_filter(draw::IQueue& q, bool hasmip, GLenum texFlag) const;
			void imm_filter(bool hasmip, GLenum texFlag) const;

			DEF_DEBUGGUI_NAME
			DEF_DEBUGGUI_PROP
	};
}
