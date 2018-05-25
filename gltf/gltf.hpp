#pragma once
#include "gltf/asset.hpp"
#include "gltf/accessor.hpp"
#include "gltf/animation.hpp"
#include "gltf/buffer.hpp"
#include "gltf/bufferview.hpp"
#include "gltf/camera.hpp"
#include "gltf/image.hpp"
#include "gltf/material.hpp"
#include "gltf/mesh.hpp"
#include "gltf/node.hpp"
#include "gltf/program.hpp"
#include "gltf/sampler.hpp"
#include "gltf/scene.hpp"
#include "gltf/shader.hpp"
#include "gltf/skin.hpp"
#include "gltf/technique.hpp"
#include "gltf/texture.hpp"
#include "../dir.hpp"

namespace rev {
	class FileURI;
}
namespace rev::gltf {
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
			BOOST_PP_SEQ_FOR_EACH(DEF_M, EMPTY, SEQ_RES0)
			#undef DEF_M

			std::unordered_map<Tag, NodeMem>	m_Node;
			DRef_Scene_OP		defaultScene;

			// 現時点ではファイルのみの対応
			static GLTF Make(const FileURI& file);

			#define DEF_METHOD(z, ign, name) \
				const name& BOOST_PP_CAT(get, name)(const Tag& tag) const override;
			BOOST_PP_SEQ_FOR_EACH(DEF_METHOD, EMPTY, SEQ_RES0(Node))
			#undef DEF_METHOD
			PathBlock getFilePath(const Path& p) const override;
			GLTF(GLTF&& g) = default;
			~GLTF();
	};
	JDocument ParseJSON(const char* json);
}
