#pragma once
#include DIR_HEADER
#include <regex>
#include <boost/variant.hpp>
#include <boost/blank.hpp>

namespace rev {
	//! ディレクトリ管理
	class Dir : public PathBlock {
		private:
			struct PathReset {
				PathStr 		cwd;

				PathReset();
				~PathReset();
			};
			const static char SC, DOT, EOS, *SC_P, LBK, RBK;

			struct SearchSeg {
				using Var = boost::variant<boost::blank, std::regex, std::string>;
				Var		var;
				bool	recursive;
			};
			using SearchL = std::vector<SearchSeg>;
			using SearchItr = SearchL::const_iterator;
			using StrList = std::vector<std::string>;
			using EnumCB = std::function<void (const Dir&)>;
			using ModCB = std::function<bool (const PathBlock&, FStatus&)>;
			using SegCB = std::function<void (const char*, const char*)>;
			using WcCB = std::function<void (SearchL, std::string)>;
			//! セグメントをSearchSegに読み込み
			static SearchSeg _SegmentRead(const char* s0, const char* s1);
			//! セグメントの切り出し
			static void _ExtractSegment(const char* s, const SegCB& cb);

			static void _EnumEntry(SearchItr itr, SearchItr itrE, std::string& lpath, const EnumCB& cb);
			//! ディレクトリ区切り文字で区切ったWildcardをSearchリスト形式に直す
			static SearchL _WildcardToSegment(const char* s);
			static std::pair<std::string, int> _CalcOfs(const std::string& r);
			void _chmod(PathBlock& lpath, const ModCB& cb);
		public:
			using PathBlock::PathBlock;
			Dir() = default;
			Dir(const Dir&) = default;
			Dir(Dir&& d) noexcept;
			Dir(const PathBlock& pb): PathBlock(pb) {}

			Dir& operator = (Dir&& d) noexcept;
			Dir& operator = (const Dir&) = default;
			using PathBlock::operator ==;

			bool isFile() const noexcept;
			bool isDirectory() const noexcept;
			void remove() const;
			void copy(const std::string& to) const;
			void move(const std::string& to) const;
			FStatus status() const;
			FTime time() const;
			//! パスまでのディレクトリ構造を作成
			/*! \return true=成功(既に同じ構造ができているケース含)
			*			false=失敗(ファイルが存在するなど) */
			void mkdir(uint32_t mode) const;

			void chmod(ModCB cb);
			void chmod(uint32_t mode);
			bool ChMod(const PathBlock& pb, ModCB cb);
			FILE* openAsFP(const char* mode) const;
			/*! \return 前のカレントパス */
			std::string setCurrentDir() const;

			static std::string GetCurrentDir();
			static std::string GetProgramDir();
			static void SetCurrentDir(const std::string& path);
			static void RemoveAll(const std::string& path);

			std::string plain_utf8(bool bAbs=true) const;
			std::u32string plain_utf32(bool bAbs=true) const;

			//! ファイル/ディレクトリ列挙
			static StrList EnumEntry(const SearchL& s);
			static StrList EnumEntryWildCard(const std::string& s);
			static void EnumEntry(const SearchL& s, const EnumCB& cb);
			static void EnumEntryWildCard(const std::string& s, const EnumCB& cb);
	};
}
namespace std {
	template <>
	struct hash<rev::Dir> {
		std::size_t operator()(const rev::Dir& d) const noexcept {
			return std::hash<rev::PathBlock>()(d);
		}
	};
}
