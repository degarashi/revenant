#pragma once
#include "gl_texture.hpp"
#include "frea/src/vector.hpp"

namespace rev {
	//! デバッグ用テクスチャ模様生成インタフェース
	struct ITDGen {
		virtual ~ITDGen() {}
		virtual uint32_t getFormat() const = 0;
		virtual bool isSingle() const = 0;
		virtual ByteBuff generate(const lubee::SizeI& size, CubeFace face=CubeFace::PositiveX) const = 0;
	};
	using TDGen_UP = std::unique_ptr<ITDGen>;
	#define DEF_DEBUGGEN \
		uint32_t getFormat() const override; \
		bool isSingle() const override; \
		ByteBuff generate(const lubee::SizeI& size, CubeFace face) const override;

	//! 2色チェッカー
	class TDChecker : public ITDGen {
		private:
			frea::Vec4	_col[2];
			int			_nDivW, _nDivH;
		public:
			TDChecker(const frea::Vec4& col0, const frea::Vec4& col1, int nDivW, int nDivH);
			DEF_DEBUGGEN
	};
	//! カラーチェッカー
	/*! 準モンテカルロで色を決定 */
	class TDCChecker : public ITDGen {
		public:
			DEF_DEBUGGEN
	};
	//! ベタ地と1テクセル枠
	class TDBorder : public ITDGen {
		public:
			TDBorder(const frea::Vec4& col, const frea::Vec4& bcol);
			DEF_DEBUGGEN
	};
	//! デバッグ用のテクスチャ
	/*! DeviceLost時:
		再度生成し直す */
	class Texture_Debug :
		public TextureSource
	{
		private:
			// デバッグ用なので他との共有を考えず、UniquePtrとする
			TDGen_UP		_gen;
			bool			_mip,
							_cube;
		public:
			Texture_Debug(ITDGen* gen, const lubee::SizeI& size, bool bCube, bool mip);
			void onDeviceReset() override;
			bool isCubemap() const override;
	};
}
