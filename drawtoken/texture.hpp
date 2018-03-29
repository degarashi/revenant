#pragma once
#include "uniform.hpp"
#include "../gl_texture.hpp"
#include "../handle/opengl.hpp"

namespace rev::draw {
	// 連番テクスチャはUniformId + Indexとして設定
	class Texture :
		public IGLTexture,
		public Uniform<Texture>
	{
		private:
			HTex			_hTex;
			mutable int		_actId;
		public:
			Texture(const HTex& hTex);
			void exportToken(TokenDst& dst, GLint id, int activeTexId) const override;
			void setIds(GLint id, int activeTexId) const;
			virtual ~Texture();
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
			TextureA(const std::vector<HTex>& t);
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
