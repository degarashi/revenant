#include "gltf/gltf.hpp"
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
#include "../sdl_rw.hpp"
#include "../dir.hpp"

namespace rev::gltf {
	using namespace loader;
	GLTF GLTF::Make(const FileURI& file) {
		// ファイルのディレクトリをベースパスとする
		PathBlock base(file.pathblock());
		base.popBack();
		return GLTF(
			ParseJSON(mgr_rw.fromFile(file.pathblock(), Access::Read)->readAllAsString().c_str()),
			base
		);
	}
	#define LOAD_TQ(value, type, entry)	value(Optional<Dictionary<Shared<type>>>(v, entry, {}))
	GLTF::GLTF(const JValue& v, const PathBlock& basepath):
		_basepath(basepath),
		LOAD_TQ(accessor, Accessor, "accessors"),
		LOAD_TQ(animation, Animation, "animations"),
		asset(Optional<Asset>(v, "asset")),
		LOAD_TQ(buffer, Buffer, "buffers"),
		LOAD_TQ(bufferView, BufferView, "bufferViews"),
		LOAD_TQ(camera, Camera, "cameras"),
		LOAD_TQ(image, Image, "images"),
		LOAD_TQ(material, Material, "materials"),
		LOAD_TQ(mesh, Mesh, "meshes"),
		LOAD_TQ(program, Program, "programs"),
		LOAD_TQ(sampler, Sampler, "samplers"),
		defaultScene(Optional<TagScene>(v, "scene")),
		LOAD_TQ(scene, Scene, "scenes"),
		LOAD_TQ(shader, Shader, "shaders"),
		LOAD_TQ(skin, Skin, "skins"),
		LOAD_TQ(technique, Technique, "techniques"),
		LOAD_TQ(texture, Texture, "textures")
	{
		const auto nodes = Optional<Dictionary<loader::Node>>(v, "nodes", {});
		for(auto& nd : nodes)
			node.map.emplace(nd.first, nd.second);
		_mergeTags();
		resolve(*this);
	}

	#undef LOAD_TQ
	void GLTF::_mergeTags() {
		_query.emplace_back(&accessor);
		_query.emplace_back(&animation);
		_query.emplace_back(&buffer);
		_query.emplace_back(&bufferView);
		_query.emplace_back(&camera);
		_query.emplace_back(&image);
		_query.emplace_back(&material);
		_query.emplace_back(&mesh);
		_query.emplace_back(&node);
		_query.emplace_back(&program);
		_query.emplace_back(&sampler);
		_query.emplace_back(&scene);
		_query.emplace_back(&shader);
		_query.emplace_back(&skin);
		_query.emplace_back(&technique);
		_query.emplace_back(&texture);
	}

	void GLTF::resolve(const ITagQuery& q) {
		const auto rsv = [&q](auto& dst){
			for(auto& m : dst.map)
				m.second->resolve(q);
		};
		rsv(accessor);
		rsv(animation);
		rsv(buffer);
		rsv(bufferView);
		rsv(image);
		rsv(material);
		rsv(mesh);
		rsv(node);
		rsv(program);
		rsv(scene);
		rsv(shader);
		rsv(skin);
		rsv(technique);
		rsv(texture);
		if(defaultScene)
			defaultScene->resolve(q);
	}
	Void_SP GLTF::query(Resource::Type type, const Tag& tag) const noexcept {
		if(type == Resource::Type::Localfile) {
			_basepath <<= tag;
			struct Term {
				PathBlock& pb;
				~Term() {
					pb.popBack();
				}
			};
			const Term term{_basepath};
			if(_basepath.isFile()) {
				return std::make_shared<PathBlock>(_basepath);
			}
		}
		for(auto* q : _query) {
			if(const auto ret = q->query(type, tag))
				return ret;
		}
		return nullptr;
	}
}
#include <cereal/external/rapidjson/error/en.h>
namespace rev::gltf {
	JDocument ParseJSON(const char* json) {
		JDocument doc;
		doc.Parse(json);
		if(doc.HasParseError()) {
			throw SyntaxError(
				json,
				doc.GetErrorOffset(),
				rapidjson::GetParseError_En(doc.GetParseError())
			);
		}
		return doc;
	}
}
