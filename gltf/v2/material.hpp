#pragma once
#include "gltf/v2/resource.hpp"
#include "gltf/v2/dataref.hpp"
#include "frea/vector.hpp"
#include "spine/optional.hpp"
#include "../../pbr_mr.hpp"

namespace rev::gltf::v2 {
	class Material :
		public Resource
	{
		public:
			struct TextureInfo {
				DRef_Texture	index;
				std::size_t		texCoord;		// Default: 0

				TextureInfo(const JValue& v, const IDataQuery& q);
			};
			using TextureInfo_OP = spi::Optional<TextureInfo>;

			struct PBR {
				frea::Vec4		baseColorFactor;
				TextureInfo_OP	baseColorTexture,
								metallicRoughnessTexture;
				float			metallicFactor,		// default: 1.0
								roughnessFactor;	// default: 1.0

				PBR();
				PBR(const JValue& v, const IDataQuery& q);
			};
			struct NormalT : TextureInfo {
				float			scale;		// default: 1.0

				NormalT(const JValue& v, const IDataQuery& q);
			};
			struct OcclusionT : TextureInfo {
				float			strengh;	// default: 1.0

				OcclusionT(const JValue& v, const IDataQuery& q);
			};
			using EmissiveT = TextureInfo;
			using NormalT_OP = spi::Optional<NormalT>;
			using OcclusionT_OP = spi::Optional<OcclusionT>;
			using EmissiveT_OP = spi::Optional<EmissiveT>;

			PBR						pbrMetallicRoughness;
			NormalT_OP				normalTexture;
			OcclusionT_OP			occlusionTexture;
			EmissiveT_OP			emissiveTexture;
			frea::Vec3				emissiveFactor;		// default: [0,0,0]

			AlphaMode				alphaMode;			// default: Opaque
			float					alphaCutoff;		// default: 0.5
			bool					doubleSided;		// default: false

			Material();
			Material(const JValue& v, const IDataQuery& q);
			Type getType() const noexcept override;

			mutable PBR_SP		cached_pbr;
			const PBR_SP& getPBR() const;

			static PBR_SP		default_pbr;
			static const PBR_SP& GetDefaultPBR();
	};
}
