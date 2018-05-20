#pragma once
#include "../handle/opengl.hpp"
#include "common.hpp"
#include "../debuggui_if.hpp"

namespace rev {
	class IEffect;
}
namespace rev::dc {
	using Name = std::string;
	class IMesh :
		public IDebugGui
	{
		private:
			HPrim		_primitive;
			HTech		_tech;
		protected:
			Name		_userName;
			IMesh(const HPrim& p, const HTech& t, const Name& userName);
			void _draw(IEffect& e) const;
			void _applyTech(IEffect& e) const;
		public:
			virtual ~IMesh();
			virtual void draw(IEffect& e, const NodeParam& np) const = 0;
			const HTech& getTech() const noexcept;

			DEF_DEBUGGUI_PROP
	};
}
