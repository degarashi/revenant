#include "gltf/v2/dc_mesh_module.hpp"
#include "../../dc/qm_if.hpp"
#include "../../effect/u_morph.hpp"
#include "../../effect/u_matrix3d.hpp"
#include "../../effect/if.hpp"

namespace rev::gltf::v2 {
	void IModule::applyUniform(IEffect&) const {}
	std::string IModule::getFlagString() const {
		return std::string();
	}
	// -------------- NoSkinModule --------------
	NoSkinModule::NoSkinModule(const dc::JointId id):
		_jointId(id)
	{}
	std::string NoSkinModule::getTechName() const {
		return "Mesh";
	}
	frea::AMat4 NoSkinModule::proc(U_Matrix3D& m3, const dc::IQueryMatrix& qm) const {
		const frea::AMat4 m(qm.getGlobal(_jointId));
		m3.setWorld(m);
		return m;
	}
	const char* NoSkinModule::typeString() const noexcept {
		return "NormalMesh";
	}

	// -------------- SkinModule --------------
	SkinModule::SkinModule(const SkinBindSet_SP& bind):
		_bind(bind)
	{}
	std::string SkinModule::getTechName() const {
		return "Skin";
	}
	frea::AMat4 SkinModule::proc(U_Matrix3D& m3, const dc::IQueryMatrix& qm) const {
		const frea::AMat4 root_m(qm.getGlobal(_bind->rootId));
		// set skinning parameters...
		using MV = std::vector<frea::AMat4, lubee::AlignedPool<frea::AMat4>>;
		MV mv;
		for(auto& b : _bind->bind) {
			frea::AMat4 m = b.invmat * qm.getGlobal(b.jointId);
			if(!b.hasRoot)
				m *= root_m;
			mv.emplace_back(m);
		}
		// OpenGLではcolumn-majorな関係でtranspose = true
		m3.setJoint(mv);
		// あくまで目安なので、とりあえずこれでOK
		return root_m;
	}
	const char* SkinModule::typeString() const noexcept {
		return "SkinMesh";
	}

	// -------------- BumpModule --------------
	std::string BumpModule::getFlagString() const {
		return "Bump";
	}

	// -------------- MorphModule --------------
	MorphModule::MorphModule(const Morph& m, const WeightV_S& w):
		_morph(m),
		_weight(w)
	{
		D_Assert0(w);
	}
	void MorphModule::applyUniform(IEffect& e) const {
		MorphCoeffMask mask;
		_morph.makeMask(MorphTarget::Position, mask, *_weight);
		auto& m = dynamic_cast<U_Morph&>(e);
		m.coeff.pos.ar_f = mask;
	}
	std::string MorphModule::getFlagString() const {
		return "Morph";
	}
}
