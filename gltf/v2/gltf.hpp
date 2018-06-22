#pragma once
#include "gltf/v2/asset.hpp"
#include "gltf/v2/accessor.hpp"
#include "gltf/v2/animation.hpp"
#include "gltf/v2/buffer.hpp"
#include "gltf/v2/bufferview.hpp"
#include "gltf/v2/camera.hpp"
#include "gltf/v2/image.hpp"
#include "gltf/v2/material.hpp"
#include "gltf/v2/mesh.hpp"
#include "gltf/v2/sampler.hpp"
#include "gltf/v2/scene.hpp"
#include "gltf/v2/skin.hpp"
#include "gltf/v2/node.hpp"
#include "gltf/v2/texture.hpp"
#include "../../dir.hpp"

namespace rev {
	class FileURI;
}
namespace rev::gltf::v2 {
	class GLTF :
		public IDataQuery
	{
		private:
			constexpr static std::size_t NodeSize =
				std::max(
					std::max(sizeof(CameraNode), sizeof(MeshNode)),
					sizeof(SkinMeshNode)
				);
			using NodeMem = std::array<uint8_t, NodeSize>;
			template <class T>
			using DataV = std::vector<spi::Optional<T>>;

			mutable Dir			_basepath;
			GLTF(const JValue& v, const PathBlock& basepath);

		public:
			using StringV = std::vector<std::string>;
			StringV				extensionsUsed,
								extensionsRequired;
			Asset				asset;

			#define DEF_M(z, _, name)	DataV<name> BOOST_PP_CAT(m_, name);
			BOOST_PP_SEQ_FOR_EACH(DEF_M, EMPTY, SEQ_V2_RES0)
			#undef DEF_M

			std::vector<NodeMem>	m_Node;
			DRef_Scene_OP			defaultScene;

			static GLTF Make(const FileURI& file);
			#define DEF_METHOD(z, ign, name) \
				const name& BOOST_PP_CAT(get, name)(const DataId& id) const override;
			BOOST_PP_SEQ_FOR_EACH(DEF_METHOD, EMPTY, SEQ_V2_RES0(Node))
			#undef DEF_METHOD

			PathBlock getFilePath(const Path& p) const override;
			GLTF(GLTF&& g) = default;
			~GLTF();
	};
}
