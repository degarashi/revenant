#pragma once
#include "exceptions.hpp"
#include "spine/enum.hpp"
#include "lubee/none.hpp"
#include "lubee/wrapper.hpp"
#include "spine/optional.hpp"
#include "beat/pose3d.hpp"
#include <cereal/external/rapidjson/document.h>
#include <vector>
#include <unordered_map>
#include <GL/gl.h>

namespace rev::gltf {
	DEF_HASTYPE_T(value_t)
	using JValue = rapidjson::Value;
	using JType = rapidjson::Type;
	using JSize = rapidjson::SizeType;
	using Tag = std::string;
	namespace loader {
		template <class T, ENABLE_IF(HasTypeT_value_t_t<T>{})>
		typename T::value_t UnwrapValue_TFunc(T*);
		template <class T, ENABLE_IF(!HasTypeT_value_t_t<T>{})>
		T UnwrapValue_TFunc(T*);
		template <class T>
		using UnwrapValue_T = decltype(UnwrapValue_TFunc((T*)nullptr));

		inline Tag g_dictEntName;
		// 必須エントリの読み込み
		// エントリが存在しない場合はLackOfPrerequisite例外を投げる
		// 型が一致しない場合はInvalidPropertyr例外を投げる
		const JValue& GetRequiredEntry(const JValue& v, const char* key);
		// Optionalエントリの読み込み
		// エントリが存在しない場合は引数の値が渡される
		// 型が一致しない場合はInvalidProperty例外を投げる
		const JValue& GetOptionalEntry(const JValue& v, const char* key, const JValue& def);

		template <class T>
		T Required(const JValue& v, const char* key) {
			return T(GetRequiredEntry(v, key));
		}
		template <class T>
		typename spi::Optional<UnwrapValue_T<T>> Optional(const JValue& v, const char* key) {
			const auto itr = v.FindMember(key);
			if(itr == v.MemberEnd())
				return spi::none;
			return T(itr->value);
		}
		template <class T>
		UnwrapValue_T<T> Optional(const JValue& v, const char* key, const UnwrapValue_T<T>& def) {
			if(auto ret = Optional<T>(v, key))
				return *ret;
			return def;
		}

		template <class T, bool AllowSingle=false>
		struct Array;
		#define DEF_GETTER(name, typ) \
			struct name : lubee::Wrapper<typ> { \
				using base_t = lubee::Wrapper<typ>; \
				name(const JValue& v); \
				static bool CanLoad(const JValue& v) noexcept; \
				using base_t::base_t; \
				using base_t::operator std::add_const_t<typ>&; \
			}; \
			using name##_A = Array<name>; \
			name FindLoader(typ);
		DEF_GETTER(Null, lubee::none_t);
		DEF_GETTER(Bool, bool);
		DEF_GETTER(Number, double);
		Number FindLoader(float);
		DEF_GETTER(Integer, int64_t);
		template <class T, ENABLE_IF(std::is_integral<T>{})>
		Integer FindLoader(T);
		DEF_GETTER(GLEnum, GLenum);
		DEF_GETTER(String, const char*);
		DEF_GETTER(StdString, std::string);
		#undef DEF_GETTER
		template <class T>
		using FindLoader_t = decltype(FindLoader(std::declval<T>()));

		template <class T>
		struct Shared : std::shared_ptr<UnwrapValue_T<T>> {
			using child_t = UnwrapValue_T<T>;
			using value_t = std::shared_ptr<child_t>;
			Shared(const JValue& v):
				value_t(
					new child_t(lubee::UnwrapValue(T{v}))
				)
			{}
			static bool CanLoad(const JValue& v) noexcept {
				return T::CanLoad(v);
			}
		};
		template <class T, bool AllowSingle>
		struct Array : std::vector<UnwrapValue_T<T>> {
			using child_t = UnwrapValue_T<T>;
			using value_t = std::vector<child_t>;
			Array(const JValue& v) {
				if(!CanLoad(v))
					throw InvalidProperty("can't read as array");
				if(!v.IsArray()) {
					if(AllowSingle) {
						this->emplace_back(T{v});
					} else
						throw InvalidProperty("not an array");
				} else {
					const auto sz = v.Size();
					for(JSize i=0 ; i<sz ; i++) {
						this->emplace_back(
							static_cast<const child_t&>(
								T{v[i]}
							)
						);
					}
				}
			}
			static bool CanLoad(const JValue& v) noexcept {
				if(v.IsArray()) {
					if(v.Size() == 0)
						return true;
					return T::CanLoad(v[0]);
				} else if(AllowSingle)
					return T::CanLoad(v);
				return false;
			}
		};
		template <class T>
		struct Dictionary : std::unordered_map<Tag, UnwrapValue_T<T>> {
			using child_t = UnwrapValue_T<T>;
			using value_t = std::unordered_map<Tag, child_t>;
			Dictionary(const JValue& v) {
				if(!CanLoad(v))
					throw InvalidProperty("not an object");
				const auto itrE = v.MemberEnd();
				for(auto itr = v.MemberBegin(); itr != itrE ; ++itr) {
					auto* str = itr->name.GetString();
					g_dictEntName = str;
					this->emplace(
						str,
						static_cast<const child_t&>(
							T{itr->value}
						)
					);
				}
				g_dictEntName.clear();
			}
			static bool CanLoad(const JValue& v) noexcept {
				return v.IsObject();
			}
		};
		template <int N>
		struct Vec : frea::Vec_t<float, N, true> {
			using value_t = frea::Vec_t<float, N, true>;
			Vec(const JValue& v) {
				if(!CanLoad(v))
					throw InvalidProperty("can't read as vector");
				const Array<Number> loader(v);
				for(int i=0 ; i<N ; i++)
					this->m[i] = loader[i];
			}
			static bool CanLoad(const JValue& v) noexcept {
				return v.IsArray() && v.Size()==N;
			}
		};
		using Vec2 = Vec<2>;
		using Vec3 = Vec<3>;
		using Vec4 = Vec<4>;

		template <int S>
		struct Mat : frea::Mat_t<float, S, S, true> {
			using value_t = frea::Mat_t<float, S, S, true>;
			Mat(const JValue& v) {
				if(!CanLoad(v))
					throw InvalidProperty("can't read as matrix");
				const Array<Number> loader(v);
				// column-major -> row-major
				for(int i=0 ; i<S ; i++) {
					for(int j=0 ; j<S ; j++) {
						this->m[i][j] = loader[j*S+i];
					}
				}
			}
			static bool CanLoad(const JValue& v) noexcept {
				return v.IsArray() && v.Size() == S*S;
			}
		};
		using Mat2 = Mat<2>;
		using Mat3 = Mat<3>;
		using Mat4 = Mat<4>;

		struct Quat : frea::AQuat {
			using value_t = frea::AQuat;
			Quat(const JValue& v);
			static bool CanLoad(const JValue& v) noexcept;
		};
		struct Pose3 : beat::g3::Pose {
			using value_t = beat::g3::Pose;
			Pose3(const JValue& v);
			static bool CanLoad(const JValue& v) noexcept;
		};
	}
}
