#pragma once
#include "spine/optional.hpp"
#include "abstbuffer.hpp"
#include <deque>
#include <functional>

namespace rev {
	template <class T>
	struct CharConst;
	template <>
	struct CharConst<char> {
		constexpr static char SC = '/',
							DOT = '.',
							EOS = '\0',
							CLN = ':';
	};
	template <>
	struct CharConst<char32_t> {
		constexpr static char32_t SC = U'/',	//!< セパレート文字
							DOT = U'.',		//!< 拡張子の前の記号
							EOS = U'\0',	//!< 終端記号
							CLN = U':';		//!< コロン :
	};

	using Int_OP = spi::Optional<int>;
	class PathBlock {
		public:
			enum {
				Beg = 0,
				End = 0xffff
			};
			using CItr = std::string::const_iterator;
			using CItr_OP = spi::Optional<CItr>;
			using ChkStringPos = std::function<bool (CItr)>;
			using CBStringNum = std::function<Int_OP (Int_OP)>;
			static Int_OP ExtGetNum(const std::string& path);
			static Int_OP PathGetNum(const std::string& path);
			static void ExtSetNum(std::string& path, const CBStringNum& cb);
			static void PathSetNum(std::string& path, const CBStringNum& cb);
		private:
			static CItr_OP _StringGetPos(CItr itrB, CItr itrE, const ChkStringPos& chk);
			static CItr_OP _StringGetExtPos(CItr itrB, CItr itrE);
			static CItr_OP _StringGetNumPos(CItr itrB, CItr itrE);
			//! 文字列末尾の数値を取り出す
			/*! 数値が無ければnoneを返す */
			static Int_OP _StringGetNum(CItr itrB, CItr itrE);
			static Int_OP _StringToNum(CItr itrB, CItr itrE);
			static void _StringSetNum(std::string& str, int beg_pos, int end_pos, const CBStringNum& cb);
		protected:
			using Path = std::deque<char32_t>;
			using Segment = std::deque<int>;
			using OPChar = spi::Optional<char>;
			//! 絶対パスにおける先頭スラッシュを除いたパス
			Path		_path;
			//! 区切り文字の前からのオフセット
			Segment		_segment;
			bool		_bAbsolute;
			OPChar		_driveLetter;		//!< ドライブ文字(Windows用。Linuxでは無視)　\0=無効

			static bool _IsSC(char32_t c) noexcept;
			//! パスを分解しながらセグメント長をカウントし、コールバック関数を呼ぶ
			/*! fromからtoまで1文字ずつ見ながら区切り文字を直す */
			template <class Itr, class CB>
			static bool _ReWriteSC(Itr from, Itr to, char32_t sc, CB cb);
			template <class CB>
			void _iterateSegment(const char32_t* c, int len, char32_t sc, CB cb);
			template <class Itr>
			static auto _GetDriveLetter(Itr from, Itr to) -> spi::Optional<typename std::decay_t<decltype(*from)>>;
			void _outputHeader(std::u32string& dst, bool bAbs) const;
        private:
			template <class Ar>
			friend void save(Ar&, const PathBlock&);
			template <class Ar>
			friend void load(Ar&, PathBlock&);
		public:
			template <class C>
			struct StripResult {
				using OpC = spi::Optional<C>;
				bool 	bAbsolute;
				OpC		driveLetter;
				int		nread;
			};
			template <class C>
			using OPStripResult = spi::Optional<StripResult<C>>;
			//! 前後の余分な区切り文字を省く
			/*! \return [NeedOperation, AbsoluteFlag] */
			template <class Itr>
			static auto StripSC(Itr from, Itr to) -> OPStripResult<typename std::decay<decltype(*from)>::type>;
			/*!
				Windowsの場合は何もせずfromを返す
				Linuxの場合は先頭のドライブ文字を削った後のポインタを返す
			*/
			static const char* RemoveDriveLetter(const char* from, const char* to);

			PathBlock() noexcept;
			PathBlock(const PathBlock&) = default;
			PathBlock(PathBlock&& p) noexcept;
			PathBlock(To32Str p);

			bool operator == (const PathBlock& p) const noexcept;
			bool operator != (const PathBlock& p) const noexcept;
			PathBlock& operator = (const PathBlock&) = default;
			PathBlock& operator = (PathBlock&& p) noexcept;
			PathBlock& operator <<= (To32Str elem);
			PathBlock& operator <<= (const PathBlock& p);

			//! パスを後ろに追加。引数が絶対パスの時は置き換える
			void pushBack(To32Str elem);
			void pushBack(const PathBlock& p);
			void popBack();
			//! pushBack(elem) と同じ
			PathBlock& operator += (To32Str elem);
			//! パスを前に追加。thisが絶対パスの時は置き換える
			void pushFront(To32Str elem);
			void pushFront(const PathBlock& p);
			void popFront();

			std::string plain_utf8(bool bAbs=true) const;
			std::string getFirst_utf8(bool bAbs=true) const;
			std::string getLast_utf8() const;
			std::string getSegment_utf8(int beg, int end, bool bAbs=true) const;
			std::string getHeader_utf8() const;
			std::u32string plain_utf32(bool bAbs=true) const;
			std::u32string getFirst_utf32(bool bAbs=true) const;
			std::u32string getLast_utf32() const;
			std::u32string getSegment_utf32(int beg, int end, bool bAbs=true) const;
			std::u32string getHeader_utf32() const;

			OPChar getDriveLetter() const noexcept;

			int size() const noexcept;
			int segments() const noexcept;
			void setPath(To32Str p);
			bool isAbsolute() const noexcept;
			bool hasExtention() const;
			void setExtension(To32Str ext);
			std::string getExtension(bool bRaw=false) const;
			//! 拡張子末尾の数値を取得
			/*! filename.ext123 -> 123
				filename.ext -> 0 */
			Int_OP getExtNum() const;
			//! 拡張子末尾の数値を加算
			void setExtNum(const CBStringNum& cb);
			//! ファイル名末尾の数値を取得
			Int_OP getPathNum() const;
			//! ファイル名末尾の数値を加算
			void setPathNum(const CBStringNum& cb);
			void clear() noexcept;
			bool empty() const noexcept;
			explicit operator bool () const noexcept;
	};
}
namespace std {
	template <>
	struct hash<rev::PathBlock> {
		std::size_t operator()(const rev::PathBlock& pb) const {
			return std::hash<std::u32string>()(pb.plain_utf32());
		}
	};
}
