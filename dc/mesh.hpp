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
			FWPrim		_primitive;
			HTech		_tech;
		protected:
			JointId		_jointId;
			Name		_userName;
			IMesh(const FWPrim& p, const HTech& t, JointId id, const Name& userName);
			void _draw(IEffect& e) const;
			void _applyTech(IEffect& e) const;
		public:
			virtual ~IMesh();
			virtual void draw(IEffect& e, const NodeParam& np) const = 0;
			const HTech& getTech() const noexcept;

			DEF_DEBUGGUI_PROP
	};
	class Mesh : public IMesh {
		private:
			SName			_jointName;
		public:
			Mesh(const FWPrim& p, const HTech& t, const Name& userName, const SName& jointName);
			void draw(IEffect& e, const NodeParam& np) const override;

			DEF_DEBUGGUI_SUMMARYSTR
	};
	class SkinMesh : public IMesh {
		private:
			SkinBindV_SP	_binding;
		public:
			SkinMesh(const FWPrim& p, const HTech& t, const Name& userName, const SkinBindV_SP& bind);
			void draw(IEffect& e, const NodeParam& np) const override;

			DEF_DEBUGGUI_SUMMARYSTR
	};
}
