#pragma once
#include "gltf/v1/asset.hpp"
#include "gltf/v1/accessor.hpp"
#include "gltf/v1/animation.hpp"
#include "gltf/v1/buffer.hpp"
#include "gltf/v1/bufferview.hpp"
#include "gltf/v1/camera.hpp"
#include "gltf/v1/image.hpp"
#include "gltf/v1/material.hpp"
#include "gltf/v1/mesh.hpp"
#include "gltf/v1/node.hpp"
#include "gltf/v1/program.hpp"
#include "gltf/v1/sampler.hpp"
#include "gltf/v1/scene.hpp"
#include "gltf/v1/shader.hpp"
#include "gltf/v1/skin.hpp"
#include "gltf/v1/technique.hpp"
#include "gltf/v1/texture.hpp"
#include "../../fs/dir.hpp"

namespace rev {
	class FileURI;
}
namespace rev::gltf::v1 {
	class GLTF :
		public IDataQuery
	{
		private:
			using Asset_OP = spi::Optional<Asset>;
			using DRef_Scene_OP = spi::Optional<DRef_Scene>;
			constexpr static std::size_t NodeSize =
				std::max(
					std::max(sizeof(CameraNode), sizeof(MeshNode)),
					sizeof(SkinMeshNode)
				);
			using NodeMem = std::array<uint8_t, NodeSize>;

			mutable Dir			_basepath;
			GLTF(const JValue& v, const PathBlock& basepath);

		public:
			Asset_OP			asset;

			#define DEF_M(z, _, name)	std::unordered_map<Tag, BOOST_PP_CAT(name, _OP)> BOOST_PP_CAT(m_, name);
			BOOST_PP_SEQ_FOR_EACH(DEF_M, EMPTY, SEQ_V1_RES0)
			#undef DEF_M

			std::unordered_map<Tag, NodeMem>	m_Node;
			DRef_Scene_OP		defaultScene;

			// 現時点ではファイルのみの対応
			static GLTF Make(const FileURI& file);

			#define DEF_METHOD(z, ign, name) \
				const name& BOOST_PP_CAT(get, name)(const Tag& tag) const override;
			BOOST_PP_SEQ_FOR_EACH(DEF_METHOD, EMPTY, SEQ_V1_RES0(Node))
			#undef DEF_METHOD
			PathBlock getFilePath(const Path& p) const override;
			GLTF(GLTF&& g) = default;
			~GLTF();
	};
}
