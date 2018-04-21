#pragma once
#if defined(UNIX)
	#include "dir_depLinux.hpp"
#elif defined(WIN32)
	#include "dir_depWin.hpp"
#else
	#error "unknown OS"
#endif
#include <regex>
#include <boost/variant.hpp>

namespace rev {
	//! ディレクトリ管理
	class Dir : public PathBlock {
		public:
			struct PathReset {
				PathStr 		cwd;

				PathReset();
				~PathReset();
			};
		private:
			const static char SC, DOT, EOS, *SC_P, LBK, RBK;

			using RegexL = std::vector<boost::variant<std::regex, std::string>>;
			using RegexItr = RegexL::const_iterator;
			using StrList = std::vector<std::string>;
			using EnumCB = std::function<void (const Dir&)>;
			using ModCB = std::function<bool (const PathBlock&, FStatus&)>;
			//! '/'で区切ったRegExの文法をリスト形式に直す
			static RegexL _ParseRegEx(const std::string& r);

			static void _EnumEntryRegEx(RegexItr itr, RegexItr itrE, std::string& lpath, std::size_t baseLen, EnumCB cb);
			static void _EnumEntry(const std::string& s, const std::string& path, EnumCB cb);
			void _chmod(PathBlock& lpath, ModCB cb);
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
			//! ファイル/ディレクトリ列挙
			/*! \param[in] path 検索パス(正規表現) 区切り文字を跨いでのマッチは不可<br>
								相対パスならDirが指しているディレクトリ以下の階層を探索 */
			static StrList EnumEntryRegEx(const std::string& r);
			/*! \param[in] path 検索パス(ワイルドカード) 区切り文字を跨いでのマッチは不可 */
			static StrList EnumEntryWildCard(const std::string& s);
			static void EnumEntryRegEx(const std::string& r, EnumCB cb);
			static void EnumEntryWildCard(const std::string& s, EnumCB cb);
			void chmod(ModCB cb);
			void chmod(uint32_t mode);
			bool ChMod(const PathBlock& pb, ModCB cb);
			FILE* openAsFP(const char* mode) const;
			/*! \return 前のカレントパス */
			std::string setCurrentDir() const;

			//! ワイルドカードから正規表現への書き換え
			static std::string ToRegEx(const std::string& s);
			static std::string GetCurrentDir();
			static std::string GetProgramDir();
			static void SetCurrentDir(const std::string& path);
			static void RemoveAll(const std::string& path);

			std::string plain_utf8(bool bAbs=true) const;
			std::u32string plain_utf32(bool bAbs=true) const;
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
