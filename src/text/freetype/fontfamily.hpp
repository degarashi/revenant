#pragma once
#include "wrap.hpp"
#include "../name.hpp"
#include "../../abstbuffer.hpp"
#include "spine/src/optional.hpp"

namespace rev {
	#define mgr_font static_cast<FontFamily&>(::rev::FontFamily::ref())
	//! 対象ディレクトリからフォントファイルを列挙しリストアップ
	class FontFamily : public FTLibrary {
		public:
			struct Item {
				int			faceIndex;
				HRW			hRW;
				using Path_OP = spi::Optional<std::string>;
				Path_OP		path;

				Item(Item&&) = default;
				Item(int fIdx, const HRW& hRW);
				Item(int fIdx, const std::string& p);
				HFT makeFont() const;

				Item& operator = (Item&&) = default;
			};
		private:
			//! [FamilyName -> FullPath]
			using FontMap = std::unordered_map<FontName, Item>;
			FontMap			_fontMap;
			lubee::SizeI	_dpi;

		public:
			// フォント生成に使うDPIを渡す
			FontFamily(lubee::SizeI dpi);
			void loadFamilyWildCard(To8Str pattern);
			void loadFamily(const HRW& hRW);

			//! FamilyNameからフォントを特定
			HFT fontFromFamilyName(const FontName &name) const;

			lubee::SizeI getDPI() const noexcept;
	};
}
namespace std {
	template <>
	struct hash<::rev::FontFamily::Item> {
		std::size_t operator()(const ::rev::FontFamily::Item& it) const {
			if(it.hRW)
				return reinterpret_cast<std::size_t>(it.hRW.get());
			return hash<std::string>()(it.path.get());
		}
	};
}
