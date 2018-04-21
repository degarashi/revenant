#pragma once
#include "idtag.hpp"
#include "asset.hpp"
#include "../dir.hpp"

namespace rev {
	class FileURI;
}
namespace rev::gltf {
	class GLTF :
		IResolvable,
		ITagQuery
	{
		private:
			using Asset_OP = spi::Optional<Asset>;
			using TagScene_OP = spi::Optional<TagScene>;
			using TagQV = std::vector<ITagQuery*>;

			TagQV				_query;
			mutable Dir			_basepath;
			void _mergeTags();
			GLTF(const JValue& v, const PathBlock& basepath);

		public:
			Accessor_TQ			accessor;
			Animation_TQ		animation;
			Asset_OP			asset;
			Buffer_TQ			buffer;
			BufferView_TQ		bufferView;
			Camera_TQ			camera;
			Image_TQ			image;
			Material_TQ			material;
			Mesh_TQ				mesh;
			Node_TQ				node;
			Program_TQ			program;
			Sampler_TQ			sampler;
			TagScene_OP			defaultScene;
			Scene_TQ			scene;
			Shader_TQ			shader;
			Skin_TQ				skin;
			Technique_TQ		technique;
			Texture_TQ			texture;

			// 現時点ではファイルのみの対応
			static GLTF Make(const FileURI& file);

			void resolve(const ITagQuery& q) override;
			Void_SP query(Resource::Type type, const Tag& tag) const noexcept override;
	};
	JDocument ParseJSON(const char* json);
}
