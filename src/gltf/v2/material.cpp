#include "gltf/v2/material.hpp"
#include "gltf/v2/texture.hpp"
#include "gltf/value_loader.hpp"
#include "gltf/check.hpp"
#include "../../gl_resource.hpp"
#include "../../gl_texture.hpp"
#include "../../texturesrc_mem.hpp"

namespace rev::gltf::v2 {
	namespace L = gltf::loader;
	using M = Material;
	// --------------- M::TextureInfo ---------------
	M::TextureInfo::TextureInfo(const JValue& v, const IDataQuery& q):
		index(L::Required<DRef_Texture>(v, "index", q)),
		texCoord(L::OptionalDefault<L::Integer>(v, "texCoord", 0))
	{}

	// --------------- M::PBR ---------------
	PBR_SP M::default_pbr;
	M::PBR::PBR():
		baseColorFactor(1,1,1,1),
		metallicFactor(1.f),
		roughnessFactor(1.f)
	{}
	M::PBR::PBR(const JValue& v, const IDataQuery& q):
		baseColorFactor(L::OptionalDefault<L::Vec4>(v, "baseColorFactor", {1,1,1,1})),
		baseColorTexture(L::Optional<TextureInfo>(v, "baseColorTexture", q)),
		metallicRoughnessTexture(L::Optional<TextureInfo>(v, "metallicRoughnessTexture", q)),
		metallicFactor(L::OptionalDefault<L::Float>(v, "metallicFactor", 1.f)),
		roughnessFactor(L::OptionalDefault<L::Float>(v, "roughnessFactor", 1.f))
	{}

	// --------------- M::NormalT ---------------
	M::NormalT::NormalT(const JValue& v, const IDataQuery& q):
		TextureInfo(v, q),
		scale(L::OptionalDefault<L::Float>(v, "scale", 1.f))
	{}

	// --------------- M::OcclusionT ---------------
	M::OcclusionT::OcclusionT(const JValue& v, const IDataQuery& q):
		TextureInfo(v, q),
		strengh(L::OptionalDefault<L::Float>(v, "strength", 1.f))
	{}

	// --------------- Material ---------------
	M::Material():
		Resource(nullptr),
		emissiveFactor(0,0,0),
		alphaMode(AlphaMode::Opaque),
		alphaCutoff(0.5f),
		doubleSided(false)
	{}
	namespace {
		const std::pair<std::string, AlphaMode> c_mode[] = {
			{"OPAQUE", AlphaMode::Opaque},
			{"MASK", AlphaMode::Mask},
			{"BLEND", AlphaMode::Blend},
		};
	}
	M::Material(const JValue& v, const IDataQuery& q):
		Resource(v),
		pbrMetallicRoughness(L::OptionalDefault<PBR>(v, "pbrMetallicRoughness", {}, q)),
		normalTexture(L::Optional<NormalT>(v, "normalTexture", q)),
		occlusionTexture(L::Optional<OcclusionT>(v, "occlusionTexture", q)),
		emissiveTexture(L::Optional<EmissiveT>(v, "emissiveTexture", q)),
		emissiveFactor(L::OptionalDefault<L::Vec3>(v, "emissiveFactor", {0,0,0})),
		alphaMode(AlphaMode::Opaque),
		alphaCutoff(L::OptionalDefault<L::Float>(v, "alphaCutoff", 0.5f)),
		doubleSided(L::OptionalDefault<L::Bool>(v, "doubleSided", false))
	{
		if(const auto mode = L::Optional<L::String>(v, "alphaMode")) {
			const auto& m = CheckEnum(c_mode, *mode, [](auto& c, auto& m){
				return c.first == m;
			});
			alphaMode = m.second;
		}
	}
	Resource::Type M::getType() const noexcept {
		return Type::Material;
	}

	const PBR_SP& M::getPBR() const {
		if(!cached_pbr) {
			cached_pbr = std::make_shared<PBRMetallicRoughness>();
			const auto toTex = [](const TextureInfo& info) {
				PBRMetallicRoughness::Tex ret;
				ret.tex = info.index->getGLResource();
				ret.coordId = info.texCoord;
				return ret;
			};
			auto& dst = *cached_pbr;
			{
				// Color
				auto& mr = pbrMetallicRoughness;
				if(auto& t = mr.baseColorTexture)
					dst.color = toTex(*t);
				dst.colorFactor = mr.baseColorFactor;
				if(auto& t = mr.metallicRoughnessTexture)
					dst.mr = toTex(*t);
				dst.mrFactor = {mr.metallicFactor, mr.roughnessFactor};
			}
			// Normal
			if(auto& t = normalTexture) {
				dst.normal = toTex(*t);
				dst.normalScale = t->scale;
			} else {
				dst.normalScale = 1.f;
			}
			// Occlusion
			if(auto& t = occlusionTexture) {
				dst.occlusion = toTex(*t);
				dst.occlusionStrength = t->strengh;
			} else {
				dst.occlusionStrength = 1.f;
			}
			// Emissive
			if(auto& t = emissiveTexture)
				dst.emissive = toTex(*t);
			dst.emissiveFactor = emissiveFactor;

			dst.alphaMode = alphaMode;
			dst.alphaCutoff = alphaCutoff;
			dst.doubleSided = doubleSided;
		}
		return cached_pbr;
	}
	const PBR_SP& M::GetDefaultPBR() {
		if(!default_pbr) {
			default_pbr = std::make_shared<PBRMetallicRoughness>();
		}
		return default_pbr;
	}
}
