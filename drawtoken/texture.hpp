#pragma once
#include "uniform.hpp"
#include "../gl_texture.hpp"
#include "../handle/opengl.hpp"

namespace rev::draw {
	// 連番テクスチャはUniformId + Indexとして設定
	class Texture :
		public TextureBase,
		public Uniform<Texture>
	{
		private:
			HTexC			_hTex;
		public:
			Texture(const HTexC& hTex);
			virtual ~Texture() {}
			void exportToken(TokenDst& dst, GLint id, int activeTexId) const override;
			void setIds(GLint id, int activeTexId) const;
			void exec() override;
			bool isArray() const noexcept override;
			#ifdef DEBUGGUI_ENABLED
				void printValue() const override;
			#endif
	};
	class TextureA :
		public Uniform<TextureA>
	{
		private:
			using TexA = std::vector<Texture>;
			TexA	_texA;
		public:
			TextureA(const std::vector<HTexC>& t);
			template <class Itr>
			TextureA(Itr itr, const Itr itrE) {
				while(itr != itrE) {
					_texA.emplace_back(*itr);
					++itr;
				}
			}
			void exportToken(TokenDst& dst, GLint id, int activeTexId) const override;
			void exec() override;
			bool isArray() const noexcept override;
			#ifdef DEBUGGUI_ENABLED
				void printValue() const override;
			#endif
	};
}
