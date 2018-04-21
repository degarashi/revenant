/*
#pragma once
#include "../glx_if.hpp"
#include "gltf/value_loader.hpp"
#include "gltf/idtag.hpp"
#include "../drawtoken/make_uniform.hpp"

namespace rev {
	namespace gltf {
		using JValue = rapidjson::Value;
		using JSize = rapidjson::SizeType;
		using JType = rapidjson::Type;
		struct IUniformValue : IResolvable {
			void resolve(const ITagQuery&) override {}
			virtual draw::Token_SP makeToken() const = 0;
		};
		using UniformValue_UP = std::unique_ptr<IUniformValue>;

		// NMaxはあくまで最大要素数
		template <class This, class T, int NMax>
		struct UniformBase : IUniformValue {
			using raw_t = typename T::value_t;
			using array_t = std::array<raw_t, NMax>;
			array_t		value;
			// 実際に格納されている要素数
			int			nvalue;

			// 二番目の引数は"配列か否か"を示す
			UniformBase(const JValue& v, std::false_type) {
				value[0] = T{v};
				nvalue = 1;
			}
			UniformBase(const JValue& v, std::true_type) {
				const auto s = v.Size();
				for(JSize i=0 ; i<s ; i++)
					value[i] = T{v[i]};
				nvalue = s;
			}
			static UniformValue_UP Load(const JValue& v) {
				// 配列要素数が1の時は[]で囲まなくても可
				if(!v.IsArray()) {
					if(T::CanLoad(v)) {
						return std::make_unique<This>(v, std::false_type{});
					}
				} else if(T::CanLoad(v[0])) {
					const auto s = v.Size();
					if(s > NMax)
						return nullptr;
					return std::make_unique<This>(v, std::true_type{});
				}
				return nullptr;
			}
		};
		template <class T, int NMax>
		struct UniformArray : UniformBase<UniformArray<T,NMax>, T, NMax> {
			using value_t = UniformArray;
			using UniformBase<UniformArray<T,NMax>, T, NMax>::UniformBase;
			using raw_t = typename T::value_t;
			// 実行時使える形式に変換
			draw::Token_SP makeToken() const override {
				return draw::MakeUniform(
					this->value.data(),
					this->nvalue
				);
			}
		};
		// タグ参照を用いてOpenGLリソースハンドルを生成する物
		template <class T, int NMax>
		struct UniformTag : UniformBase<UniformTag<T,NMax>, T, NMax> {
			using value_t = UniformTag;
			using UniformBase<UniformTag<T,NMax>, T, NMax>::UniformBase;
			void resolve(const ITagQuery& q) override {
				// IDTag参照後の値を保持
				const auto nv = this->nvalue;
				for(int i=0 ; i<nv ; i++) {
					auto& d = this->value[i];
					d.resolve(q);
				}
			}
			draw::Token_SP makeToken() const override {
				using gl_t = std::decay_t<decltype(std::declval<T>().data()->getGLResource())>;
				gl_t tmp[NMax];
				const auto nv = this->nvalue;
				for(int i=0 ; i<nv ; i++)
					tmp[i] = this->value[i].data()->getGLResource();
				return draw::MakeUniform(tmp, nv);
			}
		};
		// 配列サイズは常に1
		template <int Size>
		struct UniformMat : UniformBase<UniformMat<Size>, loader::Number, Size*Size> {
			using value_t = UniformMat;
			using UniformBase<UniformMat<Size>, loader::Number, Size*Size>::UniformBase;
			draw::Token_SP makeToken() const override {
				using mat_t = frea::Mat_t<float, Size, Size, false>;
				return draw::MakeUniform(
					*reinterpret_cast<const mat_t*>(this->value.data())
				);
			}
		};
		UniformValue_UP LoadUniformValue(const JValue& v);
	}
}
*/
