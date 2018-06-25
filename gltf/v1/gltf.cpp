#include "gltf/v1/gltf.hpp"
#include "gltf/json_parse.hpp"
#include "../../sdl_rw.hpp"

namespace rev::gltf::v1 {
	using ::rev::gltf::loader::Required;
	using ::rev::gltf::loader::Optional;
	using ::rev::gltf::loader::OptionalDefault;
	using ::rev::gltf::loader::Dictionary;
	using ::rev::gltf::loader::GetRequiredEntry;
	using ::rev::gltf::loader::GetOptionalEntry;

	GLTF GLTF::Make(const FileURI& file) {
		// ファイルのディレクトリをベースパスとする
		PathBlock base(file.pathblock());
		base.popBack();
		return GLTF(
			ParseJSON(mgr_rw.fromFile(file.pathblock(), Access::Read)->readAllAsString().c_str()),
			base
		);
	}
	#define LOAD_ENT(type, entry)	BOOST_PP_CAT(m_, type)(OptionalDefault<Dictionary<type>>(v, entry, {}, *this))
	#define LOAD_ENT0(type, entry)	BOOST_PP_CAT(m_, type)(OptionalDefault<Dictionary<type>>(v, entry, {}))
	GLTF::GLTF(const JValue& v, const PathBlock& basepath):
		_basepath(basepath),
		asset(Optional<Asset>(v, "asset"))
	{
		// 最初にメモリを確保する
		const auto alloc = [&v](auto& dst, const char* name){
			if(const auto& e = GetOptionalEntry(v, name)) {
				auto& ent = *e;
				auto itr = ent.MemberBegin();
				const auto itrE = ent.MemberEnd();
				while(itr != itrE) {
					dst[itr->name.GetString()];
					++itr;
				}
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
		alloc(m_Program,		"programs");
		alloc(m_Sampler,		"samplers");
		alloc(m_Scene,			"scenes");
		alloc(m_Skin,			"skins");
		alloc(m_Shader,			"shaders");
		alloc(m_Technique,		"techniques");
		alloc(m_Texture,		"textures");
		alloc(m_Node,			"nodes");

		const auto load = [&v](auto& dst, const auto&... args) {
			using OP = typename std::remove_reference_t<decltype(dst)>::mapped_type;
			using Value = typename OP::value_t;
			if(auto m_op = Optional<Dictionary<Value>>(v, args...)) {
				auto& mm = *m_op;
				for(auto& m : mm) {
					dst.at(m.first) = std::move(m.second);
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
		load(m_Program,			"programs",		*this);
		load(m_Sampler,			"samplers");
		load(m_Scene,			"scenes",		*this);
		load(m_Skin,			"skins",		*this);
		load(m_Shader,			"shaders",		*this);
		load(m_Technique,		"techniques",	*this);
		load(m_Texture,			"textures",		*this);
		if(auto n = Optional<Dictionary<loader::Node>>(v, "nodes", *this)) {
			auto& nodes = *n;
			for(auto& nd : nodes) {
				auto& mem = m_Node.at(nd.first);
				nd.second->moveTo(mem.data());
			}
			for(auto& n : m_Node) {
				auto* np = reinterpret_cast<Node*>(n.second.data());
				np->setParent();
			}
		}
		defaultScene = Optional<DRef_Scene>(v, "scene", *this);
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
	#define DEF_QUERY(z, ign, name) \
		const name& GLTF::BOOST_PP_CAT(get, name)(const Tag& tag) const { \
			auto& m = BOOST_PP_CAT(m_, name); \
			const auto itr = m.find(tag); \
			if(itr != m.end()) \
				return *itr->second.pointer(); \
			return IDataQuery::BOOST_PP_CAT(get, name)(tag); \
		}
	BOOST_PP_SEQ_FOR_EACH(DEF_QUERY, EMPTY, SEQ_V1_RES0)
	#undef DEF_QUERY

	const Node& GLTF::getNode(const Tag& tag) const {
		const auto itr = m_Node.find(tag);
		if(itr != m_Node.end())
			return *reinterpret_cast<const Node*>(itr->second.data());
		return IDataQuery::getNode(tag);
	}
	GLTF::~GLTF() {
		for(auto& n : m_Node) {
			auto* np = reinterpret_cast<Node*>(n.second.data());
			np->~Node();
		}
	}
}
