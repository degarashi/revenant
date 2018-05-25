#pragma once
#include "gltf/exceptions.hpp"
#include "gltf/value_loader.hpp"
#include "gltf/resource.hpp"
#include "../handle/sdl.hpp"
#include "../handle/uri.hpp"
#include <unordered_map>

namespace rev {
	namespace gltf {
		using Tag = std::string;

		struct ITagQuery {
			virtual ~ITagQuery() {}
			virtual Void_SP query(Resource::Type type, const Tag& tag) const noexcept = 0;
		};
		template <class T, Resource::Type::e Type>
		struct TagQuery : ITagQuery {
			using Map = std::unordered_map<Tag, T>;
			Map		map;

			TagQuery() = default;
			TagQuery(Map&& m):
				map(std::move(m))
			{}
			Void_SP query(Resource::Type type, const Tag& tag) const noexcept override {
				if(type == Type) {
					const auto itr = map.find(tag);
					if(itr != map.end())
						return itr->second;
				}
				return nullptr;
			}
		};
		struct IResolvable {
			virtual ~IResolvable() {}
			virtual void resolve(const ITagQuery& q) = 0;
		};
		template <class T, Resource::Type::e Type>
		class IDTag {
			private:
				using Data_t = std::shared_ptr<T>;
			private:
				Tag			_tag;
				Data_t		_data;
			public:
				IDTag() = default;
				IDTag(const JValue& v) {
					_tag = loader::String(v);
				}
				IDTag(const char* tag):
					IDTag(Tag(tag))
				{}
				IDTag(const Tag& tag):
					_tag(tag)
				{
					if(tag.empty())
						throw InvalidID(tag);
				}
				static bool CanLoad(const JValue& v) {
					return loader::String::CanLoad(v);
				}
				void resolve(const ITagQuery& q) {
					const auto ret = q.query(Type, _tag);
					if(!ret)
						throw UnknownID(_tag);
					_data = std::static_pointer_cast<T>(ret);
				}
				bool resolved() const noexcept {
					return _data;
				}
				const Tag& tag() const noexcept {
					return _tag;
				}
				const Data_t& data() const noexcept {
					return _data;
				}
				auto operator * () const noexcept -> decltype(_data) {
					return _data;
				}
				auto operator ->() const -> decltype(_data.get()) {
					return _data.get();
				}
				bool operator == (const IDTag& a) const noexcept {
					return _tag == a._tag;
				}
		};
		class TagRW : IResolvable {
			private:
				HURI				_uri;
				mutable ByteV_OP	_buffer_cached;
			public:
				TagRW(const JValue& v);
				HRW getRW() const;
				const HURI& getUri() const noexcept;
				void resolve(const ITagQuery& q) override;

				const ByteV& getBuffer() const;
		};

		#define DEF_TAG(name)	\
			struct name; \
			using name##_SP = std::shared_ptr<name>; \
			using name##_WP = std::weak_ptr<name>; \
			using name##_SPC = std::shared_ptr<const name>; \
			using name##_TQ = TagQuery<name##_SP, Resource::Type::name>; \
			using Tag##name = IDTag<name, Resource::Type::name>; \
			using Tag##name##V = std::vector<Tag##name>;

		DEF_TAG(Accessor)
		DEF_TAG(Animation)
		DEF_TAG(Buffer)
		DEF_TAG(BufferView)
		DEF_TAG(Camera)
		DEF_TAG(Image)
		DEF_TAG(Material)
		DEF_TAG(Mesh)
		DEF_TAG(Node)
		DEF_TAG(Program)
		DEF_TAG(Sampler)
		DEF_TAG(Scene)
		DEF_TAG(Shader)
		DEF_TAG(Skin)
		DEF_TAG(Technique)
		DEF_TAG(Texture)

		#undef DEF_TAG
	}
}
