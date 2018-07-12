#include "u_pbr_mr.hpp"
#include "gl_program.hpp"
#include "uniform_ent.hpp"
#include "vdecl.hpp"

namespace rev {
	namespace {
		using UniformName = spi::FlyweightItem<std::string>;
		namespace texture {
			const UniformName
				Color("u_colorTex"),
				MR("u_mrTex"),
				Normal("u_normalTex"),
				Occlusion("u_occlusionTex"),
				Emissive("u_emissiveTex"),
				Environment("u_environment");
		}
		namespace factor {
			const UniformName
				Color("u_colorFactor"),
				MR("u_mrFactor"),
				Normal("u_normalFactor"),
				Occlusion("u_occlusionFactor"),
				Emissive("u_emissiveFactor");
		}
		namespace levels {
			const UniformName
				Environment("u_environmentLevels");
		}
		namespace coord {
			const UniformName
				Color("u_colorCoord"),
				MR("u_mrCoord"),
				Normal("u_normalCoord"),
				Occlusion("u_occlusionCoord"),
				Emissive("u_emissiveCoord");
		}
		namespace unif {
			const UniformName
				AlphaCutoff("u_alphaCutoff"),
				LightDir("u_lightDir");
		}
		namespace has {
			const UniformName
				Normal("u_hasNormal");
		}
	}
	UniformSetF U_PBRMetallicRoughness::getUniformF(const GLProgram& prog) const {
		UniformSetF_V fv;

		const auto set = [&fv, &prog](const auto& name, const auto& cb){
			if(const auto id = prog.getUniformId(name)) {
				fv.emplace_back([id=*id, &cb](const void* p, UniformEnt& u){
					auto* self = static_cast<const U_PBRMetallicRoughness*>(p);
					u.setUniformById(id, cb(*self));
				});
			}
		};
		set(texture::Color, [](auto& s)-> const HTex& {
			if(s.info.color.tex)
				return s.info.color.tex;
			return s._colorTex;
		});
		set(coord::Color, [](auto& s)->float { return s.info.color.coordId; });
		set(factor::Color, [](auto& s)->frea::Vec4 { return s.info.colorFactor; });

		set(texture::MR, [](auto& s)-> const HTex& {
			if(s.info.mr.tex)
				return s.info.mr.tex;
			return s._mrTex;
		});
		set(coord::MR, [](auto& s)->float { return s.info.mr.coordId; });
		set(factor::MR, [](auto& s){ return s.info.mrFactor; });

		set(texture::Normal, [](auto& s)-> const HTex& {
			if(s.info.normal.tex)
				return s.info.normal.tex;
			return s._normalTex;
		});
		set(coord::Normal, [](auto& s)->float { return s.info.normal.coordId; });
		set(factor::Normal, [](auto& s){ return s.info.normalScale; });

		set(texture::Occlusion, [](auto& s)-> const HTex& {
			if(s.info.occlusion.tex)
				return s.info.occlusion.tex;
			return s._occlusionTex;
		});
		set(coord::Occlusion, [](auto& s)->float { return s.info.occlusion.coordId; });
		set(factor::Occlusion, [](auto& s){ return s.info.occlusionStrength; });

		set(texture::Emissive, [](auto& s)-> const HTex& {
			if(s.info.emissive.tex)
				return s.info.emissive.tex;
			return s._emissiveTex;
		});
		set(coord::Emissive, [](auto& s)->float{ return s.info.emissive.coordId; });
		set(factor::Emissive, [](auto& s)->frea::Vec3{ return s.info.emissiveFactor; });

		set(unif::AlphaCutoff, [](auto& s){ return s.info.getAlphaCutoff(); });
		set(unif::LightDir, [](auto& s){ return s.lightDir; });

		set(has::Normal, [](auto& s){
			if(s.vdecl)
				return s.vdecl->hasAttribute(VSemantic{VSemEnum::NORMAL, 0});
			return false;
		});
		set(texture::Environment, [](auto& s){ return s.environment; });
		set(levels::Environment, [](auto& s) -> int32_t { return s.environment->texture()->getMipLevels(); });

		if(!fv.empty()) {
			return [fv = std::move(fv)](const void* p, UniformEnt& u){
				for(auto& f : fv)
					f(p, u);
			};
		}
		return nullptr;
	}
}

#include "gl_resource.hpp"
#include "texturesrc_mem.hpp"
namespace rev {
	namespace {
		const auto c_filter = std::make_shared<TextureFilter>();
	}
	using MR = U_PBRMetallicRoughness;
	HTex MR::MakeData(ColorTex*) {
		ByteBuff b = {255, 255, 255, 255};
		return mgr_gl.attachTexFilter(
			mgr_gl.createTextureInit({1,1}, GL_RGBA, false, true, GL_UNSIGNED_BYTE, std::move(b)),
			c_filter
		);
	}
	HTex MR::MakeData(MRTex*) {
		ByteBuff b = {255, 255, 255};
		return mgr_gl.attachTexFilter(
			mgr_gl.createTextureInit({1, 1}, GL_RGB, false, true, GL_UNSIGNED_BYTE, std::move(b)),
			c_filter
		);
	}
	HTex MR::MakeData(NormalTex*) {
		ByteBuff b = {128,128,255};
		return mgr_gl.attachTexFilter(
			mgr_gl.createTextureInit({1,1}, GL_RGB, false, true, GL_UNSIGNED_BYTE, std::move(b)),
			c_filter
		);
	}
	HTex MR::MakeData(OcclusionTex*) {
		ByteBuff b = {255, 255, 255};
		return mgr_gl.attachTexFilter(
			mgr_gl.createTextureInit({1,1}, GL_RGB, false, true, GL_UNSIGNED_BYTE, std::move(b)),
			c_filter
		);
	}
	HTex MR::MakeData(EmissiveTex*) {
		ByteBuff b = {0,0,0};
		return mgr_gl.attachTexFilter(
			mgr_gl.createTextureInit({1,1}, GL_RGB, false, true, GL_UNSIGNED_BYTE, std::move(b)),
			c_filter
		);
	}
}
