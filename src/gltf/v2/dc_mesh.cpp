#include "gltf/v2/dc_mesh.hpp"
#include "gltf/v2/dc_mesh_module.hpp"
#include "../../techmgr.hpp"
#include "../../gl/resource.hpp"
#include "../../texture_filter.hpp"
#include "../../glx_if.hpp"
#include "../../u_pbr_mr.hpp"
#include "../../gl/bstate.hpp"
#include "../../drawcmd/aux.hpp"
#include "../../primitive.hpp"
#include "../../u_matrix3d.hpp"

namespace rev::gltf::v2 {
	GLTFMesh::GLTFMesh(
		const HPrim& p, const PBR_SP& pbr,
		const Name& name, dc::BBox_Op bbox,
		SkinModule_U skin, Module_V module
	):
		_primitive(p),
		_pbr(pbr),
		_name(name),
		_bbox(bbox),
		_skin(std::move(skin)),
		_module(std::move(module))
	{
		std::vector<std::string> flag;
		for(auto& m : _module) {
			flag.emplace_back(m->getFlagString());
		}
		std::sort(flag.begin(), flag.end());

		std::string flagC;
		for(auto& f : flag) {
			if(!flagC.empty())
				flagC += "_";
			flagC += f;
		}
		if(flagC.empty())
			flagC = "Default";
		_tech = _tp->getTechnique(_skin->getTechName(), flagC);
	}
	HTex GLTFMesh::MakeData(Environment*) {
		auto f = mgr_gl.createTexFilter();
		f->setFilter(true, true);
		f->setMipLinear(true);
		f->setWrap(WrapState::ClampToEdge);
		auto src = mgr_gl.loadCubeTexture(
			MipState::MipmapLinear,
			spi::none,
			UserURI("morning/morning_lf.tga"),
			UserURI("morning/morning_rt.tga"),
			UserURI("morning/morning_up.tga"),
			UserURI("morning/morning_dn.tga"),
			UserURI("morning/morning_ft.tga"),
			UserURI("morning/morning_bk.tga")
		);
		return mgr_gl.attachTexFilter(src, f);
	}
	HTP GLTFMesh::MakeData(TP*) {
		return mgr_tech.loadTechPass("gltf2.glx");
	}
	HTech GLTFMesh::getTech() const {
		return _tech;
	}
	void GLTFMesh::draw(IEffect& e, const dc::IQueryMatrix& qm, dc::BBox* bb) const {
		e.setTechnique(_tech);
		e.setPrimitive(_primitive);
		{
			const frea::AMat4 m(_skin->proc(dynamic_cast<U_Matrix3D&>(e), qm));
			if(!_flip)
				_flip = m.calcDeterminant() < 0;
			if(_bbox && bb)
				bb->append(_bbox->transform(m));
		}
		for(auto& m : _module)
			m->applyUniform(e);
		{
			auto& pbr = dynamic_cast<U_PBRMetallicRoughness&>(e);
			pbr.info = *_pbr;
			pbr.vdecl = _primitive->getVDecl();
			pbr.environment = _env;
		}
		{
			auto& q = e.refQueue();
			if(_pbr->doubleSided)
				GL_BState(false, GL_CULL_FACE).dcmd_export(q);
			if(_pbr->alphaMode != AlphaMode::Blend)
				GL_BState(false, GL_BLEND).dcmd_export(q);
		}
		if(*_flip)
			e.refQueue().add(draw::FlipFace());
		e.draw();
		if(*_flip)
			e.refQueue().add(draw::FlipFace());
		{
			auto& q = e.refQueue();
			if(_pbr->doubleSided)
				GL_BState(true, GL_CULL_FACE).dcmd_export(q);
			if(_pbr->alphaMode != AlphaMode::Blend)
				GL_BState(true, GL_BLEND).dcmd_export(q);
		}
	}
}
