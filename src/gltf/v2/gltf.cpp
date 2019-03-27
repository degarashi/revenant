#include "gltf/v2/gltf.hpp"
#include "../../sdl/rw.hpp"
#include "../json_parse.hpp"

namespace rev::gltf::v2 {
	namespace L = gltf::loader;
	GLTF::GLTF(const JValue& v, const PathBlock& basepath):
		_basepath(basepath),
		asset(L::Required<Asset>(v, "asset"))
	{
		// 最初にメモリを確保する
		const auto alloc = [&v](auto& dst, const char* name){
			if(const auto& e = L::GetOptionalEntry(v, name)) {
				dst.resize(e->Size());
			}
		};
		alloc(m_Accessor,		"accessors");
		alloc(m_Animation,		"animations");
		alloc(m_Buffer,			"buffers");
		alloc(m_BufferView,		"bufferViews");
		alloc(m_Camera,			"cameras");
		alloc(m_Image,			"images");
		alloc(m_Mesh,			"meshes");
		alloc(m_Material,		"materials");
		alloc(m_Sampler,		"samplers");
		alloc(m_Scene,			"scenes");
		alloc(m_Skin,			"skins");
		alloc(m_Texture,		"textures");
		alloc(m_Node,			"nodes");

		const auto load = [&v](auto& dst, const auto&... args) {
			using OP = typename std::remove_reference_t<decltype(dst)>::value_type;
			using Value = typename OP::value_t;
			if(auto m_op = L::Optional<L::Array<Value>>(v, args...)) {
				auto& m = *m_op;
				const auto len = m.size();
				for(std::size_t i=0 ; i<len ; i++) {
					dst[i] = std::move(m[i]);
				}
			}
		};
		load(m_Accessor,		"accessors",	*this);
		load(m_Animation,		"animations",	*this);
		load(m_Buffer,			"buffers",		*this);
		load(m_BufferView,		"bufferViews",	*this);
		load(m_Camera,			"cameras");
		load(m_Image,			"images",		*this);
		load(m_Mesh,			"meshes",		*this);
		load(m_Material,		"materials",	*this);
		load(m_Sampler,			"samplers");
		load(m_Scene,			"scenes",		*this);
		load(m_Skin,			"skins",		*this);
		load(m_Texture,			"textures",		*this);
		if(auto n = L::Optional<L::Array<loader::Node>>(v, "nodes", *this)) {
			auto& nodes = *n;
			const auto len = m_Node.size();
			D_Assert0(len == nodes.size());
			for(std::size_t i=0 ; i<len ; i++) {
				nodes[i]->moveTo(m_Node[i].data());
			}
			for(auto& n : m_Node) {
				auto* np = reinterpret_cast<Node*>(n.data());
				np->setParent();
			}
		}
		defaultScene = L::Optional<DRef_Scene>(v, "scene", *this);
	}
	PathBlock GLTF::getFilePath(const Path& p) const {
		_basepath <<= p;
		struct Term {
			PathBlock& pb;
			~Term() {
				pb.popBack();
			}
		};
		const Term term{_basepath};
		if(_basepath.isFile()) {
			return _basepath;
		}
		throw UnknownID("file not found");
	}
	GLTF GLTF::Make(const FileURI& file) {
		// ファイルのディレクトリをベースパスとする
		PathBlock base(file.pathblock());
		base.popBack();
		return GLTF(
			ParseJSON(mgr_rw.fromFile(file.pathblock(), Access::Read)->readAllAsString().c_str()),
			base
		);
	}
	#define DEF_QUERY(z, ign, name) \
		const name& GLTF::BOOST_PP_CAT(get, name)(const DataId& id) const { \
			auto& m = BOOST_PP_CAT(m_, name); \
			if(id < m.size()) \
				return *m[id].pointer(); \
			return IDataQuery::BOOST_PP_CAT(get, name)(id); \
		}
	BOOST_PP_SEQ_FOR_EACH(DEF_QUERY, EMPTY, SEQ_V2_RES0)
	#undef DEF_QUERY
	const Node& GLTF::getNode(const DataId& id) const {
		if(id < m_Node.size())
			return *reinterpret_cast<const Node*>(m_Node[id].data());
		return IDataQuery::getNode(id);
	}
	GLTF::~GLTF() {
		for(auto& n : m_Node) {
			auto* np = reinterpret_cast<Node*>(n.data());
			np->~Node();
		}
	}
}
