#include "gltf/v1/uniform_value.hpp"
#include "gltf/v1/texture.hpp"
#include "../../uniform_ent.hpp"

namespace rev::gltf::v1 {
	UniformValue LoadUniformValue(const JValue& v, const IDataQuery& q) {
		const auto getDouble = [](auto& v){ return v.GetDouble(); };
		const auto getBool = [](auto& v){ return static_cast<uint8_t>(v.GetBool()); };
		const auto getTexture = [&q](auto& v) -> DRef_Texture { return {v.GetString(), q}; };

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
			const std::size_t n = v.Size();
			Assert(n > 0, "empty array is not allowed");
			const auto proc = [&v, n](auto&& getter, auto* type){
				using Type = std::remove_pointer_t<decltype(type)>;
				std::vector<Type> ret;
				for(std::size_t i=0 ; i<n ; i++)
					ret.emplace_back(getter(v[rapidjson::SizeType(i)]));
				return ret;
			};
			if(v[0].IsNumber())
				return proc(getDouble, (double*)nullptr);
			else if(v[0].IsBool())
				return proc(getBool, (uint8_t*)nullptr);
			else if(v[0].IsString())
				return proc(getTexture, (DRef_Texture*)nullptr);
		}
		Assert(false, "unknown value type");
		throw 0;
	}

	namespace cnv {
		struct Base {
			std::size_t		_count;
			UniformEnt&		_u;
			const SName&	_uname;
			Base(const std::size_t c, UniformEnt& u, const SName& uname):
				_count(c),
				_u(u),
				_uname(uname)
			{}
		};
		template <class T>
		struct Single : Base {
			using Base::Base;

			void operator()(const DRef_Texture&) const { Assert0(false); }
			void operator()(const std::vector<DRef_Texture>&) const { Assert0(false); }

			template <class V>
			void operator()(const V& v) const {
				Assert0(_count == 1);
				_u.setUniform(_uname, T(v));
			}
			void operator()(const std::vector<T>& v) const {
				Assert0(_count <= v.size());
				_u.setUniform(_uname, v);
			}
			template <class V>
			void operator()(const std::vector<V>& v) const {
				Assert0(_count <= v.size());
				_u.setUniform(_uname, v.cbegin(), v.cbegin()+_count);
			}
		};
		template <class V>
		struct Vector : Base {
			using Base::Base;

			template <class T>
			void operator()(const T&) const { Assert0(false); }
			void operator()(const DRef_Texture&) const { Assert0(false); }
			void operator()(const std::vector<DRef_Texture>&) const { Assert0(false); }

			template <class T, std::size_t... Idx>
			static V _MakeVec(const T* value, std::index_sequence<Idx...>) {
				return V(value[Idx]...);
			}
			template <class T>
			void operator()(const std::vector<T>& v) const {
				const auto count = _count;
				constexpr std::size_t S = V::size;
				Assert0(count*S <= v.size());

				std::vector<V> vec(count);
				auto* src = v.data();
				for(std::size_t i=0 ; i<count ; i++) {
					vec[i] = _MakeVec(src, std::make_index_sequence<S>{});
					src += S;
				}
				_u.setUniform(_uname, vec);
			}
		};
		template <class M>
		struct Matrix : Base {
			using Base::Base;

			template <class V>
			void operator()(const V&) const { Assert0(false); }
			void operator()(const DRef_Texture&) const { Assert0(false); }
			void operator()(const std::vector<DRef_Texture>&) const { Assert0(false); }

			template <class V, std::size_t... Idx>
			static M _MakeMat(const V* value, std::index_sequence<Idx...>) {
				return M(value[Idx]...);
			}
			template <class V>
			void operator()(const std::vector<V>& v) const {
				const auto count = _count;
				constexpr std::size_t S = M::dim_m * M::dim_n;
				Assert0(count*S <= v.size());

				std::vector<M> mat(count);
				auto* src = v.data();
				for(std::size_t i=0 ; i<count ; i++) {
					mat[i] = _MakeMat(src, std::make_index_sequence<S>{});
					src += S;
				}
				_u.setUniform(_uname, mat);
			}
		};
		struct Texture : Base {
			using Base::Base;

			template <class T>
			void operator()(const T&) const { D_Assert0(false); }

			void operator()(const DRef_Texture& t) const {
				_u.setUniform(_uname, t.data()->getGLResource());
			}
			void operator()(const std::vector<DRef_Texture>& t) const {
				const auto s = std::min(_count, t.size());
				std::vector<HTexC> tex(s);
				for(std::size_t i = 0 ; i<s ; i++) {
					tex[i] = t[i].data()->getGLResource();
				}
				_u.setUniform(_uname, tex.begin(), tex.end());
			}
		};
	}
	void SetUniform(UniformEnt& u, const SName& uname, const UniformValue& value, const GLenum type, const std::size_t count) {
		const GLSLFormatDesc& desc = *GLFormat::QueryGLSLInfo(type);
		uint32_t dx, dy;
		DecompDim(desc.dim, dx, dy);

		const auto procVec = [&value, count, &u, &uname](const auto dim, auto* type) {
			using Type = std::remove_pointer_t<decltype(type)>;
			switch(dim) {
				case 1:
					boost::apply_visitor(cnv::Single<Type>(count, u, uname), value);
					break;
				case 2:
					boost::apply_visitor(cnv::Vector<frea::Vec_t<Type,2,false>>(count, u, uname), value);
					break;
				case 3:
					boost::apply_visitor(cnv::Vector<frea::Vec_t<Type,3,false>>(count, u, uname), value);
					break;
				case 4:
					boost::apply_visitor(cnv::Vector<frea::Vec_t<Type,4,false>>(count, u, uname), value);
					break;
				default:
					AssertF("invalid value dimension");
			}
		};

		if(desc.type == GLSLType::IntT) {
			Assert(dy == 0, "invalid value dimension");
			if(desc.bUnsigned) {
				procVec(dx, (uint32_t*)nullptr);
			} else {
				procVec(dx, (int32_t*)nullptr);
			}
		} else if(desc.type == GLSLType::FloatT) {
			Assert(dy == 0, "invalid value dimension");
			procVec(dx, (float*)nullptr);
		} else if(desc.type == GLSLType::MatrixT) {
			Assert(dx == dy, "only square matrix is supported");
			switch(dx) {
				case 2:
					boost::apply_visitor(cnv::Matrix<frea::Mat_t<float, 2,2,false>>(count, u, uname), value);
					break;
				case 3:
					boost::apply_visitor(cnv::Matrix<frea::Mat_t<float, 3,3,false>>(count, u, uname), value);
					break;
				case 4:
					boost::apply_visitor(cnv::Matrix<frea::Mat_t<float, 4,4,false>>(count, u, uname), value);
					break;
				default:
					AssertF("invalid value dimension");
			}
		} else if(desc.type == GLSLType::TextureT) {
			// 2Dなのでdx==2
			Assert(dx==2 && dy==0, "invalid value dimension");
			boost::apply_visitor(cnv::Texture(count, u, uname), value);
		} else
			AssertF("unknown value type");
	}
}
