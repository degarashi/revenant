#include "gltf/uniform_value.hpp"
#include "../drawtoken/make_uniform.hpp"
#include "gltf/texture.hpp"

namespace rev::gltf {
	UniformValue LoadUniformValue(const JValue& v) {
		const auto getDouble = [](auto& v){ return v.GetDouble(); };
		const auto getBool = [](auto& v){ return static_cast<uint8_t>(v.GetBool()); };
		const auto getTexture = [](auto& v) -> TagTexture { return v.GetString(); };

		// 配列要素数が1の時は[]で囲まなくても可
		if(!v.IsArray()) {
			if(v.IsNumber())
				return getDouble(v);
			else if(v.IsBool())
				return getBool(v);
			else if(v.IsString())
				return getTexture(v);
			else if(v.IsNull())
				return 0.;
		} else {
			const int n = v.Size();
			Assert(n > 0, "empty array is not allowed");
			const auto proc = [&v, n](auto&& getter, auto* type){
				using Type = std::remove_pointer_t<decltype(type)>;
				std::vector<Type> ret(n);
				for(int i=0 ; i<n ; i++)
					ret[i] = getter(v[rapidjson::SizeType(i)]);
				return ret;
			};
			if(v[0].IsNumber())
				return proc(getDouble, (double*)nullptr);
			else if(v[0].IsBool())
				return proc(getBool, (uint8_t*)nullptr);
			else if(v[0].IsString())
				return proc(getTexture, (TagTexture*)nullptr);
		}
		Assert(false, "unknown value type");
		throw 0;
	}

	using draw::Token_SP, draw::MakeUniform;
	namespace cnv {
		struct Base {
			std::size_t		_count;
			Base(const std::size_t c):
				_count(c)
			{}
		};
		template <class T>
		struct Single : Base {
			using Base::Base;

			Token_SP operator()(const TagTexture&) const { Assert0(false); return nullptr; }
			Token_SP operator()(const std::vector<TagTexture>&) const { Assert0(false); return nullptr; }

			template <class V>
			Token_SP operator()(const V& v) const {
				Assert0(_count == 1);
				return MakeUniform(T(v));
			}
			Token_SP operator()(const std::vector<T>& v) const {
				Assert0(_count <= v.size());
				return MakeUniform(v);
			}
			template <class V>
			Token_SP operator()(const std::vector<V>& v) const {
				Assert0(_count <= v.size());
				return MakeUniform(v.cbegin(), v.cbegin()+_count);
			}
		};
		template <class V>
		struct Vector : Base {
			using Base::Base;

			template <class T>
			Token_SP operator()(const T&) const { Assert0(false); return nullptr; }
			Token_SP operator()(const TagTexture&) const { Assert0(false); return nullptr; }
			Token_SP operator()(const std::vector<TagTexture>&) const { Assert0(false); return nullptr; }

			template <class T, std::size_t... Idx>
			static V _MakeVec(const T* value, std::index_sequence<Idx...>) {
				return V(value[Idx]...);
			}
			template <class T>
			Token_SP operator()(const std::vector<T>& v) const {
				const auto count = _count;
				constexpr std::size_t S = V::size;
				Assert0(count*S <= v.size());

				std::vector<V> vec(count);
				auto* src = v.data();
				for(std::size_t i=0 ; i<count ; i++) {
					vec[i] = _MakeVec(src, std::make_index_sequence<V::size>{});
					src += S;
				}
				return MakeUniform(vec);
			}
		};
		template <class M>
		struct Matrix : Base {
			using Base::Base;

			template <class V>
			Token_SP operator()(const V&) const { Assert0(false); return nullptr; }
			Token_SP operator()(const TagTexture&) const { Assert0(false); return nullptr; }
			Token_SP operator()(const std::vector<TagTexture>&) const { Assert0(false); return nullptr; }

			template <class V, std::size_t... Idx>
			static M _MakeMat(const V* value, std::index_sequence<Idx...>) {
				return M(value[Idx]...);
			}
			template <class V>
			Token_SP operator()(const std::vector<V>& v) const {
				const auto count = _count;
				constexpr std::size_t S = M::dim_m * M::dim_n;
				Assert0(count*S <= v.size());

				std::vector<M> mat(count);
				auto* src = v.data();
				for(std::size_t i=0 ; i<count ; i++) {
					mat[i] = _MakeMat(src, std::make_index_sequence<S>{});
					src += S;
				}
				return MakeUniform(mat);
			}
		};
		struct Texture : Base {
			using Base::Base;

			template <class T>
			Token_SP operator()(const T&) const { D_Assert0(false); return nullptr; }

			Token_SP operator()(const TagTexture& t) const {
				return MakeUniform(t.data()->getGLResource());
			}
			Token_SP operator()(const std::vector<TagTexture>& t) const {
				const auto s = std::min(_count, t.size());
				std::vector<HTexC> tex(s);
				for(std::size_t i = 0 ; i<s ; i++) {
					tex[i] = t[i].data()->getGLResource();
				}
				return draw::MakeUniform(tex);
			}
		};
	}
	Token_SP MakeUniformToken(const UniformValue& value, const GLenum type, const std::size_t count) {
		const GLSLFormatDesc& desc = *GLFormat::QueryGLSLInfo(type);
		uint32_t dx, dy;
		DecompDim(desc.dim, dx, dy);

		const auto procVec = [&value, count](const auto dim, auto* type) -> Token_SP {
			using Type = std::remove_pointer_t<decltype(type)>;
			switch(dim) {
				case 1:
					return boost::apply_visitor(cnv::Single<Type>(count), value);
				case 2:
					return boost::apply_visitor(cnv::Vector<frea::Vec_t<Type,2,false>>(count), value);
				case 3:
					return boost::apply_visitor(cnv::Vector<frea::Vec_t<Type,3,false>>(count), value);
				case 4:
					return boost::apply_visitor(cnv::Vector<frea::Vec_t<Type,4,false>>(count), value);
				default:
					AssertF("invalid value dimension");
			}
			return nullptr;
		};

		if(desc.type == GLSLType::IntT) {
			Assert(dy == 0, "invalid value dimension");
			if(desc.bUnsigned)
				return procVec(dx, (uint32_t*)nullptr);
			else
				return procVec(dx, (int32_t*)nullptr);
		} else if(desc.type == GLSLType::FloatT) {
			Assert(dy == 0, "invalid value dimension");
			return procVec(dx, (float*)nullptr);
		} else if(desc.type == GLSLType::MatrixT) {
			Assert(dx == dy, "only square matrix is supported");
			switch(dx) {
				case 2:
					return boost::apply_visitor(cnv::Matrix<frea::Mat_t<float, 2,2,false>>(count), value);
				case 3:
					return boost::apply_visitor(cnv::Matrix<frea::Mat_t<float, 3,3,false>>(count), value);
				case 4:
					return boost::apply_visitor(cnv::Matrix<frea::Mat_t<float, 4,4,false>>(count), value);
				default:
					AssertF("invalid value dimension");
			}
		} else if(desc.type == GLSLType::TextureT) {
			// 2Dなのでdx==2
			Assert(dx==2 && dy==0, "invalid value dimension");
			return boost::apply_visitor(cnv::Texture(count), value);
		} else
			AssertF("unknown value type");
		return nullptr;
	}
}
