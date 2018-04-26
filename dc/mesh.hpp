#pragma once
#include "../handle/opengl.hpp"
#include "common.hpp"
#include "../debuggui_if.hpp"

namespace rev {
	class IEffect;
}
namespace rev::dc {
	class IMesh :
		public IDebugGui
	{
		private:
			HPrim		_primitive;
			HTech		_tech;
		protected:
			JointId		_jointId;
			Name		_userName;
			IMesh(const HPrim& p, const HTech& t, JointId id, const Name& userName);
			void _draw(IEffect& e) const;
			void _applyTech(IEffect& e) const;
		public:
			virtual ~IMesh();
			virtual void draw(IEffect& e, const NodeParam& np) const = 0;

			DEF_DEBUGGUI_PROP
	};
	class Mesh : public IMesh {
		private:
			Name_SP			_jointName;
		public:
			Mesh(const HPrim& p, const HTech& t, const Name& userName, const Name_SP& jointName);
			void draw(IEffect& e, const NodeParam& np) const override;

			DEF_DEBUGGUI_SUMMARYSTR
	};
	class SkinMesh : public IMesh {
		private:
			SkinBindV_SP	_binding;
		public:
			SkinMesh(const HPrim& p, const HTech& t, const Name& userName, const SkinBindV_SP& bind);
			void draw(IEffect& e, const NodeParam& np) const override;

			DEF_DEBUGGUI_SUMMARYSTR
	};
}