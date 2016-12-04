#pragma once
#include "lv_common.hpp"
#include "gl_format.hpp"
#include "clock.hpp"
#include <iostream>
#include <unordered_map>
#include <boost/blank.hpp>
#include "lubee/fwd.hpp"
#include "frea/fwd.hpp"
#include "handle.hpp"
#include "luaimport_types.hpp"
#include "rewindtop.hpp"

namespace rev {
	template <class T>
	struct IsSPtr : std::false_type {};
	template <class T>
	struct IsSPtr<std::shared_ptr<T>> : std::true_type {};

	class Pose2D;
	class Pose3D;
	using LPointerSP = std::unordered_map<const void*, LCValue>;

	// 値型の場合はUserdataにデータを格納
	template <class T>
	struct LCV_In;
	template <class T>
	struct LCV_In<T*>;
	template <class T>
	struct LCV_In<T&>;
	template <class T>
	struct LCV;
	template <>
	struct LCV<void> {};

	namespace detail {
		// Enum型はlua_Integerに読み替え
		template <class T>
		lua_Integer IsEnum(std::true_type);
		template <class T>
		T IsEnum(std::false_type);
		template <class T>
		using GetLCVType = decltype(IsEnum<T>(typename std::is_enum<T>::type{}));
	}
	template <class T>
	using GetLCVType = LCV<detail::GetLCVType<T>>;

	#define DEF_LCV_OSTREAM_PAIR(typ, name)	std::ostream& operator << (std::ostream& os, LCV<typ>) { return os << #name; }
	#define DEF_LCV_OSTREAM(typ)		DEF_LCV_OSTREAM_PAIR(typ, typ)

	std::ostream& operator << (std::ostream& os, LCV<void>);
	template <class T, bool A>
	std::ostream& operator << (std::ostream& os, LCV<frea::PlaneT<T,A>>) {
		if(A)
			os << "A";
		return os << "Plane";
	}
	template <class T, bool A>
	std::ostream& operator << (std::ostream& os, LCV<frea::QuatT<T,A>>) {
		if(A)
			os << "A";
		return os << "Quat";
	}
	template <class W, class D, int N>
	std::ostream& operator << (std::ostream& os, LCV<frea::VecT_spec<W,D,N>>) {
		using vec_t = frea::VecT_spec<W,D,N>;
		if(vec_t::align)
			os << "A";
		return os << "Vec" << N;
	}
	template <class V, int M, int N>
	std::ostream& operator << (std::ostream& os, LCV<frea::MatT_spec<V,M,N>>) {
		using mat_t = frea::MatT_spec<V,M,N>;
		if(mat_t::align)
			os << "A";
		return os << "Mat" << M << N;
	}
	// typ: 対象の型
	// rtyp: Luaから受け取る型
	// argtype: C++からLuaへ渡す型
	#define DEF_LCV_PAIR(typ, rtyp, argtyp) \
		template <> \
		struct LCV<typ> { \
			int operator()(lua_State* ls, argtyp t) const; \
			rtyp operator()(int idx, lua_State* ls, LPointerSP* spm=nullptr) const; \
			std::ostream& operator()(std::ostream& os, argtyp t) const; \
			LuaType operator()() const; \
		}; \
		std::ostream& operator << (std::ostream& os, LCV<typ>);
	// rtyp = typ
	#define DEF_LCV(typ, argtyp) DEF_LCV_PAIR(typ, typ, argtyp)
	// ntypをbtypと同列に扱う
	#define DERIVED_LCV(ntyp, btyp)	\
		template <> \
		struct LCV<ntyp> : LCV<btyp> {}; \
		std::ostream& operator << (std::ostream& os, LCV<ntyp>);

	DEF_LCV(boost::blank, boost::blank)
	DEF_LCV(LuaNil, LuaNil)
	DEF_LCV(bool, bool)
	DEF_LCV(const char*, const char*)
	DEF_LCV(std::string, const std::string&)
	DEF_LCV(frea::DegF, const frea::DegF&)
	DEF_LCV(frea::RadF, const frea::RadF&)
	DERIVED_LCV(const std::string&, std::string)
	DEF_LCV(lua_State*, lua_State*)
	DEF_LCV(lubee::SizeF, const lubee::SizeF&)
	DERIVED_LCV(lubee::SizeI, lubee::SizeF)
	DEF_LCV(lubee::RectF, const lubee::RectF&)
	DERIVED_LCV(lubee::RectI, lubee::RectF)
	DEF_LCV(Lua_SP, const Lua_SP&)
	DEF_LCV(void*, const void*)
	DEF_LCV(HRes, const HRes&)
	DEF_LCV(WRes, const WRes&)
	DEF_LCV(lua_CFunction, lua_CFunction)
	DEF_LCV(Timepoint, const Timepoint&)
	DEF_LCV(LCTable_SP, const LCTable_SP&)
	DEF_LCV(LCValue, const LCValue&)
	DEF_LCV(LValueG, const LValueG&)
	DEF_LCV(LValueS, const LValueS&)
	DEF_LCV(lua_Number, const lua_Number)
	DERIVED_LCV(lua_OtherNumber, lua_Number)
	DEF_LCV(lua_Integer, const lua_Integer)
	DERIVED_LCV(lua_IntegerU, lua_Integer)
	DERIVED_LCV(lua_OtherInteger, lua_Integer)
	DERIVED_LCV(lua_OtherIntegerU, lua_OtherInteger)
	DERIVED_LCV(long, lua_Integer)

	DERIVED_LCV(GLFormat, lua_Integer)
	DERIVED_LCV(GLDepthFmt, GLFormat)
	DERIVED_LCV(GLStencilFmt, GLFormat)
	DERIVED_LCV(GLDSFmt, GLFormat)
	DERIVED_LCV(GLInFmt, GLFormat)
	DERIVED_LCV(GLInSizedFmt, GLFormat)
	DERIVED_LCV(GLInCompressedFmt, GLFormat)
	DERIVED_LCV(GLInRenderFmt, GLFormat)
	DERIVED_LCV(GLInReadFmt, GLFormat)
	DERIVED_LCV(GLTypeFmt, GLFormat)

	#undef DEF_LCV
	#undef DEF_LCV_PAIR
	#undef DERIVED_LCV

	// AlignedタイプはUnAlignedと同じ扱いにする
	// VecT_spec(alignment = false)
	template <class W, class DT, int N>
	struct LCV<frea::VecT_spec<W, frea::Data<DT,N,true>, N>>:
			LCV<frea::VecT_spec<W, frea::Data<DT,N,false>, N>>
	{};
	// VecT -> VecT_spec
	template <class W, class D, class S>
	struct LCV<frea::VecT<W,D,S>>: LCV<S> {};
	// wrap -> VecT_spec
	template <class R, int D, class S>
	struct LCV<frea::wrap<R,D,S>>:
		LCV<
			frea::VecT_spec<
				typename frea::wrap<R,D,S>::template type_cn<D>,
				typename frea::wrap<R,D,S>::template data_t<false>,
				D
			>
		>
	{};
	// wrap_spec -> VecT_spec
	template <class R, int N>
	struct LCV<frea::wrap_spec<R,N>>:
		LCV<typename frea::wrap_spec<R,N>::base_t>
	{};
	// MatT(alignment = false)
	template <class W, class DT, int M, int N>
	struct LCV<frea::MatT_spec<frea::VecT_spec<W, frea::Data<DT,N,true>, N>, M, N>>:
			LCV<frea::MatT_spec<frea::VecT_spec<W, frea::Data<DT,N,false>, N>, M, N>>
	{};
	// MatT -> MatT_spec
	template <class V, int M, class S>
	struct LCV<frea::MatT<V,M,S>>: LCV<S> {};
	// MatT_dspec -> MatT_spec
	template <class V, int M, int N, class S>
	struct LCV<frea::MatT_dspec<V,M,N,S>>: LCV<S> {};
	// wrapM -> MatT_spec
	template <class VW, int M, class S>
	struct LCV<frea::wrapM<VW,M,S>>:
		LCV<
			frea::MatT_spec<
				frea::Vec_t<typename VW::value_t, VW::size, false>,
				M,
				VW::size
			>
		>
	{};
	// wrapM_spec -> MatT_spec
	template <class VW, int M, int N>
	struct LCV<frea::wrapM_spec<VW,M,N>>: LCV<typename frea::wrapM_spec<VW,M,N>::base_t> {};

	template <class T>
	struct LCV<frea::QuatT<T,true>>:
			LCV<frea::QuatT<T,false>>
	{};
	template <class T>
	struct LCV<frea::ExpQuatT<T,true>>:
			LCV<frea::ExpQuatT<T,false>>
	{};
	template <class T>
	struct LCV<frea::PlaneT<T,true>>:
			LCV<frea::PlaneT<T,false>>
	{};
	// Angleは内部数値をすべてfloatに変換
	template <class Ang, class Rep>
	struct LCV<frea::Angle<Ang, Rep>>:
		LCV<frea::Angle<Ang,float>>
	{};

	// [LCV<Optional<T>> = LCV<T>]
	template <class T>
	struct LCV<spi::Optional<T>> {
		using opt_t = spi::Optional<T>;
		int operator()(lua_State* ls, const opt_t& op) const {
			if(!op)
				return GetLCVType<LuaNil>()(ls, LuaNil());
			else
				return GetLCVType<T>()(ls, *op);
		}
		opt_t operator()(const int idx, lua_State* ls) const {
			if(lua_type(ls, idx) == LUA_TNIL)
				return spi::none;
			return GetLCVType<T>()(idx, ls);
		}
		std::ostream& operator()(std::ostream& os, const opt_t& t) const {
			if(t)
				return GetLCVType<T>()(os, *t);
			else
				return os << "(none)";
		}
		LuaType operator()() const {
			return GetLCVType<T>()();
		}
	};
	template <class T>
	std::ostream& operator << (std::ostream& os, LCV<spi::Optional<T>>) {
		return os << "Optional<" << LCV<T>() << '>';
	}

	// [LCV<Duration> = LUA_TNUMBER]
	template <class Rep, class Period>
	struct LCV<std::chrono::duration<Rep,Period>> {
		using Dur = std::chrono::duration<Rep,Period>;
		int operator()(lua_State* ls, const Dur& d) const {
			return LCV<lua_Integer>()(ls, std::chrono::duration_cast<Microseconds>(d).count());
		}
		Dur operator()(const int idx, lua_State* ls) const {
			return Microseconds(LCV<lua_Integer>()(idx, ls));
		}
		std::ostream& operator()(std::ostream& os, const Dur& d) const {
			return os << d;
		}
		LuaType operator()() const {
			return LuaType::Number;
		}
	};
	template <class... Ts>
	std::ostream& operator << (std::ostream& os, LCV<std::chrono::duration<Ts...>>) {
		return os << "duration";
	}

	// [LCV<std::vector<T>> = array]
	template <class T, class A>
	struct LCV<std::vector<T, A>> {
		using Vec_t = std::vector<T,A>;
		int operator()(lua_State* ls, const Vec_t& v) const {
			GetLCVType<T> lcv;
			const auto sz = v.size();
			lua_createtable(ls, sz, 0);
			for(std::size_t i=0 ; i<sz ; i++) {
				lua_pushinteger(ls, i+1);
				lcv(ls, v[i]);
				lua_settable(ls, -3);
			}
			return 1;
		}
		Vec_t operator()(int idx, lua_State* ls) const {
			idx = lua_absindex(ls, idx);
			GetLCVType<T> lcv;
			const int stk = lua_gettop(ls);
			lua_len(ls, idx);
			const int sz = lua_tointeger(ls, -1);
			lua_pop(ls, 1);
			Vec_t ret(sz);
			for(int i=0 ; i<sz ; i++) {
				lua_pushinteger(ls, i+1);
				lua_gettable(ls, idx);
				ret[i] = static_cast<T>(lcv(-1, ls));
				lua_pop(ls, 1);
			}
			lua_settop(ls, stk);
			return ret;
		}
		std::ostream& operator()(std::ostream& os, const Vec_t& v) const {
			return os << "(vector size=" << v.size() << ")";
		}
		LuaType operator()() const {
			return LuaType::Table;
		}
	};
	template <class T, class A>
	std::ostream& operator << (std::ostream& os, LCV<std::vector<T,A>>) {
		return os << "vector<" << LCV<T>() << '>';
	}

	// [LCV<std::tuple<>> = array]
	template <class... Ts>
	struct LCV<std::tuple<Ts...>> {
		using Tuple = std::tuple<Ts...>;
		template <std::size_t N>
		using IConst = std::integral_constant<std::size_t, N>;

		// std::tuple<> -> Args...
		int _pushElem(lua_State* /*ls*/, const Tuple& /*t*/, IConst<sizeof...(Ts)>) const {
			return 0;
		}
		template <std::size_t N>
		int _pushElem(lua_State* ls, const Tuple& t, IConst<N>) const {
			using T = typename std::decay<typename std::tuple_element<N, Tuple>::type>::type;
			int count = GetLCVType<T>()(ls, std::get<N>(t));
			return count + _pushElem(ls, t, IConst<N+1>());
		}
		int operator()(lua_State* ls, const Tuple& t) const {
			return _pushElem(ls, t, IConst<0>());
		}

		// Table -> std::tuple<>
		void _getElem(Tuple& /*dst*/, int /*idx*/, lua_State* /*ls*/, IConst<sizeof...(Ts)>) const {}
		template <std::size_t N>
		void _getElem(Tuple& dst, const int idx, lua_State* ls, IConst<N>) const {
			lua_pushinteger(ls, N+1);
			lua_gettable(ls, idx);
			std::get<N>(dst) = GetLCVType<typename std::tuple_element<N, Tuple>::type>()(-1, ls);
			lua_pop(ls, 1);
			_getElem(dst, idx, ls, IConst<N+1>());
		}
		Tuple operator()(const int idx, lua_State* ls) const {
			Tuple ret;
			_getElem(ret, idx, ls, IConst<0>());
			return ret;
		}

		std::ostream& operator()(std::ostream& os, const Tuple& /*v*/) const {
			return os << "(tuple size=" << sizeof...(Ts) << ")";
		}
		LuaType operator()() const {
			return LuaType::Table;
		}
	};

	// [LCV<std::pair<>> = array]
	template <class T0, class T1>
	struct LCV<std::pair<T0,T1>> : LCV<std::tuple<T0,T1>> {
		using base_t = LCV<std::tuple<T0,T1>>;
		using Pair = std::pair<T0,T1>;
		using Tuple = std::tuple<T0,T1>;
		Pair operator()(const int idx, lua_State* ls) const {
			auto ret = base_t::operator()(idx, ls);
			return {std::get<0>(ret), std::get<1>(ret)};
		}
		using base_t::operator();
	};
	template <class T0, class T1>
	std::ostream& operator << (std::ostream& os, LCV<std::pair<T0,T1>>) {
		return os << "pair<" << LCV<T0>() << ", " << LCV<T1>() << '>';
	}

	// [LCV<lubee::Range<T>> = LCV<std::vector<T>>]
	template <class T>
	struct LCV<lubee::Range<T>> {
		using Vec_t = std::vector<T>;
		using LCV_t = GetLCVType<Vec_t>;
		using range_t = lubee::Range<T>;
		int operator()(lua_State* ls, const range_t& r) const {
			return LCV_t()(ls, {r.from, r.to});
		}
		range_t operator()(const int idx, lua_State* ls) const {
			auto p = LCV_t()(idx, ls);
			return {p[0], p[1]};
		}
		std::ostream& operator()(std::ostream& os, const range_t& r) const {
			return LCV_t()(os, {r.from, r.to});
		}
		LuaType operator()() const {
			return LCV_t()();
		}
	};
	template <class T>
	std::ostream& operator << (std::ostream& os, LCV<lubee::Range<T>>) {
		return os << "Range<" << LCV<T>() << '>';
	}

	// [LCV<shared_ptr<T>>]
	template <class T>
	struct LCV<std::shared_ptr<T>> : LCV<HRes> {
		using sp_t = std::shared_ptr<T>;
		int operator()(lua_State* ls, const sp_t& t) const {
			return LCV<HRes>()(ls, t);
		}
		sp_t operator()(const int idx, lua_State* ls) const {
			auto vp = LCV<HRes>()(idx, ls);
			return std::static_pointer_cast<T>(vp);
		}
	};
	template <class T>
	std::ostream& operator << (std::ostream& os, LCV<std::shared_ptr<T>>) {
		return os << "shared_ptr";
	}

	// [LCV<weak_ptr<T>>]
	template <class T>
	struct LCV<std::weak_ptr<T>> : LCV<WRes> {
		using wp_t = std::weak_ptr<T>;
		int operator()(lua_State* ls, const wp_t& t) const {
			return LCV<WRes>()(ls, t);
		}
		wp_t operator()(const int idx, lua_State* ls) const {
			return std::static_pointer_cast<T>(LCV<WRes>()(idx, ls));
		}
	};
	template <class T>
	std::ostream& operator << (std::ostream& os, LCV<std::weak_ptr<T>>) {
		return os << "weak_ptr";
	}
}
#include "frea/vector.hpp"
#include "frea/quaternion.hpp"
namespace std {
	template <>
	struct hash<boost::blank> {
		std::size_t operator()(boost::blank) const {
			return 0;
		}
	};
}
#include "lubee/meta/typelist.hpp"
namespace rev {
	using Vec2 = frea::Vec_t<float,2,false>;
	using Vec3 = frea::Vec_t<float,3,false>;
	using Vec4 = frea::Vec_t<float,4,false>;
	#define SEQ_LCVAR \
		(boost::blank)(LuaNil) \
		(bool)(const char*)(lua_Integer)(lua_Number) \
		(Vec2)(Vec3)(Vec4)(frea::Quat) \
		(HRes)(WRes)(Lua_SP)(LCTable_SP)(void*)(lua_CFunction)(std::string)
	using LCVar = boost::variant<BOOST_PP_SEQ_ENUM(SEQ_LCVAR)>;
	using LCVar_Types = lubee::Types<BOOST_PP_SEQ_ENUM(SEQ_LCVAR)>;
	#undef SEQ_LCVAR

	class LCValue : public LCVar {
		private:
			template <int N>
			using IConst = std::integral_constant<int, N>;
			template <class... Args, int N, ENABLE_IF((N==sizeof...(Args)))>
			static void _TupleAsTable(LCTable_SP&, const std::tuple<Args...>&, IConst<N>) {}
			template <class... Args, int N, ENABLE_IF((N != sizeof...(Args)))>
			static void _TupleAsTable(LCTable_SP& tbl, const std::tuple<Args...>& t, IConst<N>);
			template <class... Args>
			static LCTable_SP _TupleAsTable(const std::tuple<Args...>& t);
			HRes _toSPtr() const;
		public:
			struct HashVisitor : boost::static_visitor<std::size_t> {
				template <class T>
				std::size_t operator()(const T& t) const {
					return std::hash<T>()(t);
				}
			};
			LCValue();
			LCValue(const LCValue& lc);
			LCValue(LCValue&& lcv);
			LCValue(lua_OtherNumber num);
			LCValue(lua_IntegerU num);
			LCValue(lua_OtherIntegerU num);
			LCValue(lua_OtherInteger num);
			template <
				class T,
				ENABLE_IF((
					LCVar_Types::Has<std::decay_t<T>>{}
				))
			>
			LCValue(T&& t): LCVar(std::forward<T>(t)) {}
			template <class T, ENABLE_IF((!LCVar_Types::Has<T*>{}))>
			LCValue(T* ptr):
				LCValue(reinterpret_cast<void*>(ptr))
			{}

			// Tupleは配列に変換
			LCValue(std::tuple<>& t);
			LCValue(std::tuple<>&& t);
			LCValue(const std::tuple<>& t);
			template <class... Args>
			LCValue(std::tuple<Args...>& t);
			template <class... Args>
			LCValue(std::tuple<Args...>&& t);
			template <class... Args>
			LCValue(const std::tuple<Args...>& t);
			template <
				class V,
				ENABLE_IF((
					!LCVar_Types::Has<std::decay_t<V>>{} &&
					frea::is_vector<V>{}
				))
			>
			LCValue(const V& v):
				LCVar(frea::Vec_t<float,V::size, false>(v))
			{}
			LCValue& operator = (const LCValue& lcv);
			LCValue& operator = (LCValue&& lcv);
			// リソースの固有ハンドルは汎用へ読み替え
			template <
				class T,
				ENABLE_IF((
					!LCVar_Types::Has<std::shared_ptr<T>>{} &&
					std::is_base_of<Resource, T>{}
				))
			>
			LCValue(const std::shared_ptr<T>& sp):
				LCVar(std::static_pointer_cast<Resource>(sp))
			{}
			//! 中身を配列とみなしてアクセス
			/*! \param[in] n 0オリジンのインデックス */
			const LCValue& operator[] (int n) const;

			//! 任意のリソースハンドルへ変換して取り出す
			template <class H>
			auto toSPtr() const {
				return std::static_pointer_cast<H>(_toSPtr());
			}
			bool preciseCompare(const LCValue& lcv) const;
			bool operator == (const LCValue& lcv) const noexcept;
			bool operator != (const LCValue& lcv) const noexcept;
			//! blank, Nilかfalseの場合にのみfalse, それ以外はtrueを返す
			explicit operator bool () const noexcept;
			void push(lua_State* ls) const;
			const char* toCStr() const;
			std::string toString() const;
			LuaType type() const;
	};
	std::ostream& operator << (std::ostream& os, const LCValue& lcv);
	using LCValue_SP = std::shared_ptr<LCValue>;
}
namespace std {
	template <>
	struct hash<rev::LCValue> {
		std::size_t operator()(const rev::LCValue& v) const {
			return boost::apply_visitor(rev::LCValue::HashVisitor(), static_cast<const rev::LCVar&>(v));
		}
	};
}
namespace rev {
	class LCTable : public std::unordered_map<LCValue, LCValue> {
		public:
			using base = std::unordered_map<LCValue, LCValue>;
			using base::base;

			// 文字列(const char*)とstd::stringの同一視
			// テーブルはポインタではなく中身を全て比較
			bool preciseCompare(const LCTable& tbl) const;
	};

	template <class... Args, int N, ENABLE_IF_I((N != sizeof...(Args)))>
	void LCValue::_TupleAsTable(LCTable_SP& tbl, const std::tuple<Args...>& t, IConst<N>) {
		tbl->emplace(N+1, std::get<N>(t));
		_TupleAsTable(tbl, t, IConst<N+1>());
	}
	template <class... Args>
	LCTable_SP LCValue::_TupleAsTable(const std::tuple<Args...>& t) {
		auto ret = std::make_shared<LCTable>();
		LCValue::_TupleAsTable(ret, t, IConst<0>());
		return ret;
	}
	template <class... Args>
	LCValue::LCValue(std::tuple<Args...>& t):
		LCValue(static_cast<const std::tuple<Args...>&>(t))
	{}
	template <class... Args>
	LCValue::LCValue(std::tuple<Args...>&& t):
		LCValue(static_cast<const std::tuple<Args...>&>(t))
	{}
	template <class... Args>
	LCValue::LCValue(const std::tuple<Args...>& t):
		LCVar(_TupleAsTable(t))
	{}
}
namespace rev {
	using ByteBuff = std::vector<uint8_t>;
	//! lua_Stateの単純なラッパークラス
	class LuaState : public std::enable_shared_from_this<LuaState> {
		private:
			template <class T>
			friend struct LCV;
			template <int N>
			using IConst = std::integral_constant<int,N>;
		public:
			DefineEnumPair(CMP,
				((Equal)(LUA_OPEQ))
				((LessThan)(LUA_OPLT))
				((LessEqual)(LUA_OPLE))
			);
			DefineEnumPair(OP,
				((Add)(LUA_OPADD))
				((Sub)(LUA_OPSUB))
				((Mul)(LUA_OPMUL))
				((Div)(LUA_OPDIV))
				((Mod)(LUA_OPMOD))
				((Pow)(LUA_OPPOW))
				((Umm)(LUA_OPUNM))
			);
			DefineEnum(GC,
				(Stop)
				(Restart)
				(Collect)
				(Count)
				(CountB)
				(Step)
				(SetPause)
				(SetStepMul)
				(IsRunning)
				(Gen)
				(Inc)
			);

		private:
			const static std::string cs_fromId,
									cs_fromThread,
									cs_mainThread;
			const static int ENT_ID,
							ENT_THREAD,
							ENT_NREF,
							ENT_SPILUA,
							ENT_POINTER;
			Lua_SP		_base;		//!< メインスレッド (自身がそれな場合はnull)
			ILua_SP		_lua;		//!< 自身が保有するスレッド
			using CheckTop_OP = spi::Optional<CheckTop>;
			CheckTop_OP	_opCt;

			static lubee::Freelist<int> s_index;
			static void Nothing(lua_State* ls);
			static void Delete(lua_State* ls);

		public:
			// -------------- Exceptions --------------
			struct EBase : std::runtime_error {
				EBase(const std::string& typ_msg, const std::string& msg);
			};
			//! 実行時エラー
			struct ERun : EBase {
				ERun(const std::string& s);
			};
			//! コンパイルエラー
			struct ESyntax : EBase {
				ESyntax(const std::string& s);
			};
			//! メモリ割り当てエラー
			struct EMem : EBase {
				EMem(const std::string& s);
			};
			//! メッセージハンドラ実行中のエラー
			struct EError : EBase {
				EError(const std::string& s);
			};
			//! メタメソッド実行中のエラー
			struct EGC : EBase {
				EGC(const std::string& s);
			};
			//! 型エラー
			struct EType : EBase {
				LuaType expect,
						actual;
				EType(lua_State* ls, LuaType expect, LuaType actual);
			};
			//! Luaのエラーコードに応じて例外を投げる
			static void CheckError(lua_State* ls, int code);
			void checkError(int code) const;
			//! スタック位置idxの値がtypであるかチェック
			static void CheckType(lua_State* ls, int idx, LuaType typ);
			void checkType(int idx, LuaType typ) const;
		private:
			using Deleter = std::function<void (lua_State*)>;
			static Deleter _MakeDeleter(int id);
			static Deleter _MakeCoDeleter(int id);

			static struct _TagThread {} TagThread;
			using Int_OP = spi::Optional<int>;
			void _registerNewThread(LuaState& lsc, Int_OP id);
			/* Thread変数の参照カウント機構の為の関数群
				(Luaのthread変数には__gcフィールドを設定できないため、このような面倒くさい事になっている) */
			//! global[cs_fromId], global[cs_fromThread]を無ければ作成しスタックに積む
			/*!
				FromIdはLuaStateのシリアルIdから詳細情報を
				FromThreadはLuaのスレッド変数から参照する物
			*/
			static void _PrepareThreadTable(LuaState& lsc);
			using CBGetAuxTable = std::function<void (LuaState&)>;
			/*!
				\param[in] cb	対象のLuaStateを取得してスタックのトップに積む関数
			*/
			static ILua_SP _Increment(LuaState& lsc, const CBGetAuxTable& cb);
			//! Idをキーとして参照カウンタをインクリメント
			static ILua_SP _Increment_Id(LuaState& lsc, int id);
			//! スレッド変数をキーとして参照カウンタをインクリメント
			/*! スタックトップにThread変数を積んでから呼ぶ */
			static ILua_SP _Increment_Th(LuaState& lsc);
			/*!
				\param[in] cb	対象のLuaStateを取得してスタックのトップに積む関数
			*/
			static void _Decrement(LuaState& lsc, const CBGetAuxTable& cb);
			static void _Decrement_Id(LuaState& lsc, int id);
			static void _Decrement_Th(LuaState& lsc);
			//! NewThread初期化 (コルーチン作成)
			LuaState(const Lua_SP& spLua);
			//! スレッド共有初期化
			/*! lua_Stateから元のLuaStateクラスを辿り、そのshared_ptrをコピーして使う */
			LuaState(lua_State* ls, _TagThread);

			//! RWopsに対するLua用のリーダークラス
			struct Reader {
				const HRW&		ops;
				int64_t			size;
				ByteBuff		buff;

				Reader(const HRW& hRW);
				static const char* Proc(lua_State* ls, void* data, std::size_t* size);
				static void Read(lua_State* ls, const HRW& hRW, const char* chunkName, const char* mode);
			};
			template <class TUP, int N, ENABLE_IF((N==std::tuple_size<TUP>::value))>
			void _popValues(TUP&, int, IConst<N>) {}
			template <class TUP, int N, ENABLE_IF((N!=std::tuple_size<TUP>::value))>
			void _popValues(TUP& dst, const int ofs, IConst<N>) {
				std::get<N>(dst) = toValue<typename std::tuple_element<N, TUP>::type>(ofs + N);
				_popValues(dst, ofs, IConst<N+1>());
			}
			// Luaステートの新規作成はNewState()で行う
			LuaState(lua_Alloc f, void* ud);

		public:
			LuaState(const LuaState&) = delete;
			LuaState(LuaState&& ls) = default;
			// 借り物初期化 (破棄の処理はしない) from ILua_SP -> (lua_State*)で初期化した場合と同じ
			LuaState(const ILua_SP& ls, bool bCheckTop);
			// 借り物初期化 (破棄の処理はしない) from lua_State
			LuaState(lua_State* ls, bool bCheckTop);

			//! リソースパスからファイル名指定でスクリプトを読み込み、標準ライブラリもロードする
			static Lua_SP FromResource(const std::string& name);
			//! 新しくLuaステートを作成(コルーチンではない)
			static Lua_SP NewState(lua_Alloc f=nullptr, void* ud=nullptr);
			const static char* cs_defaultmode;
			//! Text/Binary形式でLuaソースを読み取り、チャンクをスタックトップに積む
			int load(const HRW& hRW, const char* chunkName=nullptr, const char* mode=cs_defaultmode, bool bExec=true);
			//! ソースコードを読み取り、チャンクをスタックトップに積む
			/*! \param[in] bExec チャンクを実行するか */
			int loadFromSource(const HRW& hRW, const char* chunkName=nullptr, bool bExec=true);
			//! コンパイル済みバイナリを読み取り、チャンクをスタックトップに積む
			int loadFromBinary(const HRW& hRW, const char* chunkName=nullptr, bool bExec=true);
			int loadModule(const std::string& name);
			//! 任意の値をスタックに積む
			void push(const LCValue& v);
			template <class T>
			void push(const LValue<T>& v) {
				v.prepareValue(getLS());
			}
			//! C関数をスタックに積む
			/*! \param[in] nvalue 関連付けるUpValueの数 */
			void pushCClosure(lua_CFunction func, int nvalue);
			//! 任意の複数値をスタックに積む
			template <class A, class... Args>
			void pushArgs(A&& a, Args&&... args) {
				push(std::forward<A>(a));
				pushArgs(std::forward<Args>(args)...);
			}
			void pushArgs() {}
			//! スタックの値をポップすると同時にstd::tupleでそれを受け取る
			template <class... Ret>
			void popValues(std::tuple<Ret...>& dst) {
				int ofs = getTop() - sizeof...(Ret);
				D_Assert(ofs>=0, "not enough values on stack (needed: %d, actual: %d)", sizeof...(Ret), getTop());
				_popValues(dst, ofs+1, IConst<0>());
				pop(sizeof...(Ret));
			}
			void pushValue(int idx);
			void pop(int n=1);
			void pushSelf();
			void pushGlobal();
			//! 標準ライブラリを読み込む
			void loadLibraries();

			int absIndex(int idx) const;
			void arith(OP op);
			lua_CFunction atPanic(lua_CFunction panicf);
			// 内部ではpcallに置き換え、エラーを検出したら例外を投げる
			int call(int nargs, int nresults=LUA_MULTRET);
			int callk(int nargs, int nresults, lua_KContext ctx, lua_KFunction k);
			bool checkStack(int extra);
			bool compare(int idx1, int idx2, CMP cmp) const;
			void concat(int n);
			void copy(int from, int to);
			void dump(lua_Writer writer, void* data);
			void error();
			int gc(GC what, int data);
			lua_Alloc getAllocf(void** ud) const;
			void getField(int idx, const LCValue& key);
			void getGlobal(const LCValue& key);
			void getTable(int idx);
			const char* getUpvalue(int idx, int n);
			int getTop() const;
			void getUserValue(int idx);
			void getMetatable(int idx);
			void insert(int idx);
			bool isBoolean(int idx) const;
			bool isCFunction(int idx) const;
			bool isLightUserdata(int idx) const;
			bool isNil(int idx) const;
			bool isNone(int idx) const;
			bool isNoneOrNil(int idx) const;
			bool isNumber(int idx) const;
			bool isString(int idx) const;
			bool isTable(int idx) const;
			bool isThread(int idx) const;
			bool isUserdata(int idx) const;
			void length(int idx);
			int getLength(int idx);
			void newTable(int narr=0, int nrec=0);
			Lua_SP newThread();
			void* newUserData(std::size_t sz);
			int next(int idx);
			bool rawEqual(int idx0, int idx1);
			void rawGet(int idx);
			void rawGetField(int idx, const LCValue& key);
			std::size_t rawLen(int idx) const;
			void rawSet(int idx);
			void rawSetField(int idx, const LCValue& key, const LCValue& val);
			void remove(int idx);
			void replace(int idx);
			bool resume(const Lua_SP& from, int narg=0);
			void setAllocf(lua_Alloc f, void* ud);
			void setField(int idx, const LCValue& key, const LCValue& val);
			void setGlobal(const LCValue& key);
			void setMetatable(int idx);
			void setTable(int idx);
			void setTop(int idx);
			void setUservalue(int idx);
			const char* setUpvalue(int funcidx, int n);
			void* upvalueId(int funcidx, int n);
			void upvalueJoin(int funcidx0, int n0, int funcidx1, int n1);
			bool status() const;

			// --- convert function ---
			bool toBoolean(int idx) const;
			lua_CFunction toCFunction(int idx) const;
			lua_Integer toInteger(int idx) const;
			std::string toString(int idx) const;
			std::string cnvString(int idx);
			lua_Number toNumber(int idx) const;
			const void* toPointer(int idx) const;
			void* toUserData(int idx) const;
			Lua_SP toThread(int idx) const;
			LCTable_SP toTable(int idx, LPointerSP* spm=nullptr) const;
			LCValue toLCValue(int idx, LPointerSP* spm=nullptr) const;

			template <class R>
			decltype(auto) toValue(const int idx) const {
				return GetLCVType<R>()(idx, getLS());
			}

			LuaType type(int idx) const;
			static LuaType SType(lua_State* ls, int idx);
			const char* typeName(LuaType typ) const;
			static const char* STypeName(lua_State* ls, LuaType typ);
			const lua_Number* version() const;
			void xmove(const Lua_SP& to, int n);
			int yield(int nresults);
			int yieldk(int nresults, lua_KContext ctx, lua_KFunction k);

			/*! スタックトップのテーブルに"name"というテーブルが無ければ作成
				既にあれば単にそれを積む */
			void prepareTable(const std::string& name);
			void prepareTableGlobal(const std::string& name);

			lua_State* getLS() const;
			Lua_SP getLS_SP();
			Lua_SP getMainLS_SP();
			static Lua_SP GetLS_SP(lua_State* ls);
			static Lua_SP GetMainLS_SP(lua_State* ls);
			friend std::ostream& operator << (std::ostream& os, const LuaState&);
	};
	std::ostream& operator << (std::ostream& os, const LuaState& ls);
}
#include "luaimport.hpp"
DEF_LUAIMPORT_BASE
#include <map>
namespace rev {
	//! Luaから引数を変換取得して関数を呼ぶ
	template <class... Ts0>
	struct FuncCall {
		template <class CB, class... Ts1>
		static decltype(auto) callCB(const CB& cb, lua_State* /*ls*/, int /*idx*/, Ts1&&... ts1) {
			return cb(std::forward<Ts1>(ts1)...);
		}
		template <class T, class RT, class FT, class... Args, class... Ts1>
		static RT procMethod(lua_State* /*ls*/, T* ptr, int /*idx*/, RT (FT::*func)(Args...), Ts1&&... ts1) {
			return (ptr->*func)(std::forward<Ts1>(ts1)...);
		}
		template <class RT, class... Args, class... Ts1>
		static RT proc(lua_State* /*ls*/, int /*idx*/, RT (*func)(Args...), Ts1&&... ts1) {
			return func(std::forward<Ts1>(ts1)...);
		}
	};
	template <class Ts0A, class... Ts0>
	struct FuncCall<Ts0A, Ts0...> {
		template <class CB, class... Ts1>
		static decltype(auto) callCB(const CB& cb, lua_State* ls, const int idx, Ts1&&... ts1) {
			decltype(auto) value = GetLCVType<std::decay_t<Ts0A>>()(idx, ls);
			return FuncCall<Ts0...>::callCB(
						cb,
						ls,
						idx+1,
						std::forward<Ts1>(ts1)...,
						(Ts0A)value
					);
		}
		template <class T, class RT, class FT, class... Args, class... Ts1>
		static RT procMethod(lua_State* ls, T* ptr, const int idx, RT (FT::*func)(Args...), Ts1&&... ts1) {
			decltype(auto) value = GetLCVType<std::decay_t<Ts0A>>()(idx, ls);
			return FuncCall<Ts0...>::procMethod(
						ls,
						ptr,
						idx+1,
						func,
						std::forward<Ts1>(ts1)...,
						(Ts0A)value
					);
		}
		template <class RT, class... Args, class... Ts1>
		static RT proc(lua_State* ls, const int idx, RT (*func)(Args...), Ts1&&... ts1) {
			decltype(auto) value = GetLCVType<std::decay_t<Ts0A>>()(idx, ls);
			return FuncCall<Ts0...>::proc(
						ls,
						idx+1,
						func,
						std::forward<Ts1>(ts1)...,
						(Ts0A)value
					);
		}
	};
	//! Luaに返す値の数を型から特定する
	template <class T>
	struct CallAndRetSize {
		template <class CB>
		static int proc(lua_State* ls, const CB& cb) {
			return GetLCVType<T>()(ls, static_cast<detail::GetLCVType<T>>(cb()));
		}
	};
	template <>
	struct CallAndRetSize<void> {
		constexpr static int size = 0;
		template <class CB>
		static int proc(lua_State* /*ls*/, const CB& cb) {
			cb();
			return size;
		}
	};
	// --- Lua->C++グルーコードにおけるクラスポインタの取得方法 ---
	//! "pointer"に生ポインタが記録されている (void*)
	struct LI_GetPtrBase {
		void* operator()(lua_State* ls, int idx, const char* name) const;
	};
	// void*で一旦取得してからT*に変換
	template <class T>
	struct LI_GetPtr : LI_GetPtrBase {
		T* operator()(lua_State* ls, int idx) const {
			return reinterpret_cast<T*>(LI_GetPtrBase()(ls, idx, lua::LuaName((T*)nullptr)));
		}
	};
	struct LI_GetHandleBase {
		HRes operator()(lua_State* ls, int idx, const char* name) const;
	};
	//! "udata"にハンドルが記録されている -> void*からそのままポインタ変換
	template <class T>
	struct LI_GetHandle : LI_GetHandleBase {
		std::shared_ptr<T> operator()(lua_State* ls, const int idx) const {
			return std::static_pointer_cast<T>(
						LI_GetHandleBase::operator()(ls, idx, lua::LuaName((T*)nullptr))
					);
		}
	};

	//! LuaへC++のクラスをインポート、管理する
	class LuaImport {
		private:
			//! インポートしたクラス、関数や変数をテキスト出力(デバッグ用)
			static std::stringstream	s_importLog;
			static std::string			s_firstBlock;
			static int					s_indent;
			// アルファベット順に出力したいのでunordered_mapではなくmap
			using LogMap = std::map<std::string, std::string>;
			static LogMap				s_logMap;

			//! ハンドルオブジェクトの基本メソッド
			static const char* HandleName(const HRes& sh);
			static lua_Integer NumRef(const HRes& sh);

			template <class PTR>
			static PTR _GetUD(lua_State* ls, const int idx) {
				void** ud = reinterpret_cast<void**>(LCV<void*>()(idx, ls));
				return *reinterpret_cast<PTR*>(ud);
			}
			template <class V, class T>
			using PMember = V T::*;
			template <class RT, class T, class... Args>
			using PMethod = RT (T::*)(Args...);
			template <class V, class T>
			static PMember<V,T> GetMember(lua_State* ls, const int idx) {
				return _GetUD<PMember<V,T>>(ls, idx);
			}
			template <class RT, class T, class... Args>
			static PMethod<RT,T,Args...> GetMethod(lua_State* ls, const int idx) {
				return _GetUD<PMethod<RT,T,Args...>>(ls, idx);
			}
			// クラスの変数、関数ポインタは4byteでない可能性があるのでuserdataとして格納する
			template <class PTR>
			static void _SetUD(lua_State* ls, PTR ptr) {
				void* ud = lua_newuserdata(ls, sizeof(PTR));
				std::memcpy(ud, reinterpret_cast<void**>(&ptr), sizeof(PTR));
			}
			template <class T, class V>
			static void SetMember(lua_State* ls, V T::*member) {
				_SetUD(ls, member);
			}
			template <class T, class RT, class... Args>
			static void SetMethod(lua_State* ls, RT (T::*method)(Args...)) {
				_SetUD(ls, method);
			}

			static std::ostream& _PushIndent(std::ostream& s);
			template <class... Ts, ENABLE_IF(sizeof...(Ts)==0)>
			static std::ostream& _OutputArgs(std::ostream& s, bool=true) { return s; }
			template <class T, class... Ts>
			static std::ostream& _OutputArgs(std::ostream& s, bool bFirst=true) {
				if(!bFirst)
					s << ", ";
				s << LCV<T>();
				return _OutputArgs<Ts...>(s, false);
			}
		public:
			static void BeginImportBlock(const std::string& name);
			static void EndImportBlock();
			static void SaveImportLog(const HRW& hRW);

			//! メンバ関数のエクスポート
			/*! lscにFuncTableを積んだ状態で呼ぶ */
			template <class GET, class T, class RT, class FT, class... Ts>
			static void RegisterMember(LuaState& lsc, const char* name, RT (FT::*func)(Ts...)) {
				lsc.push(name);
				SetMethod(lsc.getLS(), func);
				lsc.pushCClosure(&CallMethod<GET,T,RT,FT,Ts...>, 1);
				lsc.rawSet(-3);

				// ---- ログ出力 ----
				_PushIndent(s_importLog) << LCV<RT>() << ' ' << name << '(';
				_OutputArgs<Ts...>(s_importLog) << ')' << std::endl;
			}
			//! constメンバ関数のエクスポート
			template <class GET, class T, class RT, class FT, class... Ts>
			static void RegisterMember(LuaState& lsc, const char* name, RT (FT::*func)(Ts...) const) {
				RegisterMember<GET,T>(lsc, name, (RT (FT::*)(Ts...))func);
			}
			//! メンバ変数のエクスポート
			/*! lscにReadTable, WriteTableを積んだ状態で呼ぶ */
			template <class GET, class T, class V, class VT>
			static void RegisterMember(LuaState& lsc, const char* name, V VT::*member) {
				// ReadValue関数の登録
				lsc.push(name);
				SetMember(lsc.getLS(), member);
				lsc.pushCClosure(&ReadValue<GET,T,V,VT>, 1);
				lsc.rawSet(-4);
				// WriteValue関数の登録
				lsc.push(name);
				SetMember(lsc.getLS(), member);
				lsc.pushCClosure(&WriteValue<GET,T,V,VT>, 1);
				lsc.rawSet(-3);

				// ---- ログ出力 ----
				_PushIndent(s_importLog) << LCV<V>() << ' ' << name << std::endl;
			}
			//! static関数のエクスポート
			/*! static なメンバ関数はCFunctionとして登録 */
			template <class GET, class T, class RT, class... Ts>
			static void RegisterMember(LuaState& lsc, const char* name, RT (*func)(Ts...)) {
				lsc.prepareTableGlobal(lua::LuaName((T*)nullptr));
				lsc.push(name);
				PushFunction(lsc, func);
				lsc.rawSet(-3);
				lsc.pop(1);

				// ---- ログ出力 ----
				_PushIndent(s_importLog) << "static " << LCV<RT>() <<  ' ' << name << '(';
				_OutputArgs<Ts...>(s_importLog) << ')' << std::endl;
			}
			static int ReturnException(lua_State* ls, const char* func, const std::exception& e, int nNeed);
			//! luaスタックから変数ポインタとクラスを取り出しメンバ変数を読み込む
			template <class GET, class T, class V, class VT>
			static int ReadValue(lua_State* ls) {
				try {
					// up[1]	変数ポインタ
					// [1]		クラスポインタ(userdata)
					const T* src = reinterpret_cast<const T*>(GET()(ls, 1));
					auto vp = GetMember<V,VT>(ls, lua_upvalueindex(1));
					GetLCVType<V>()(ls, src->*vp);
					return 1;
				} catch(const std::exception& e) {
					return ReturnException(ls, __PRETTY_FUNCTION__, e, 1);
				}
			}
			//! luaスタックから変数ポインタとクラスと値を取り出しメンバ変数に書き込む
			template <class GET, class T, class V, class VT>
			static int WriteValue(lua_State* ls) {
				try {
					// up[1]	変数ポインタ
					// [1]		クラスポインタ(userdata)
					// [2]		セットする値
					T* dst = reinterpret_cast<T*>(GET()(ls, 1));
					auto ptr = GetMember<V,VT>(ls, lua_upvalueindex(1));
					(dst->*ptr) = GetLCVType<V>()(2, ls);
					return 0;
				} catch(const std::exception& e) {
					return ReturnException(ls, __PRETTY_FUNCTION__, e, 2);
				}
			}
			//! luaスタックから関数ポインタとクラス、引数を取り出しクラスのメンバ関数を呼ぶ
			template <class GET, class T, class RT, class FT, class... Args>
			static int CallMethod(lua_State* ls) {
				try {
					// up[1]	関数ポインタ
					// [1]		クラスポインタ(userdata)
					// [2以降]	引数
					using F = RT (FT::*)(Args...);
					void* tmp = lua_touserdata(ls, lua_upvalueindex(1));
					F f = *reinterpret_cast<F*>(tmp);
					auto* ptr = dynamic_cast<T*>(GET()(ls, 1));
					return CallAndRetSize<RT>::proc(
							ls,
							[ls,ptr,f]() -> decltype(auto) {
								return FuncCall<Args...>::procMethod(ls, ptr, 2, f);
							}
					);
				} catch(const std::exception& e) {
					return ReturnException(ls, __PRETTY_FUNCTION__, e, sizeof...(Args)+1);
				}
			}
			//! luaスタックから関数ポインタと引数を取り出しcall
			template <class RT, class... Args>
			static int CallFunction(lua_State* ls) {
				try {
					// up[1]	関数ポインタ
					// [1以降]	引数
					using F = RT (*)(Args...);
					F f = reinterpret_cast<F>(lua_touserdata(ls, lua_upvalueindex(1)));
					// 引数を変換しつつ関数を呼んで、戻り値を変換しつつ個数を返す
					return CallAndRetSize<RT>::proc(
							ls,
							[ls,f]() -> decltype(auto) {
								return FuncCall<Args...>::proc(ls, 1, f);
							}
					);
				} catch(const std::exception& e) {
					return ReturnException(ls, __PRETTY_FUNCTION__, e, sizeof...(Args));
				}
			}
			//! staticな関数をスタックへpush
			template <class RT, class... Ts>
			static void PushFunction(LuaState& lsc, RT (*func)(Ts...)) {
				lsc.push(reinterpret_cast<void*>(func));
				lsc.pushCClosure(&CallFunction<RT, Ts...>, 1);
			}
			//! グローバル関数の登録
			template <class RT, class... Ts>
			static void RegisterFunction(LuaState& lsc, const char* name, RT (*func)(Ts...)) {
				PushFunction(lsc, func);
				lsc.setGlobal(name);

				BeginImportBlock("--Global--");
				// ---- ログ出力 ----
				_PushIndent(s_importLog) << LCV<RT>() <<  ' ' << name << '(';
				_OutputArgs<Ts...>(s_importLog) << ')' << std::endl;
				EndImportBlock();
			}
			//! C++クラスの登録(登録名はクラスから取得)
			template <class T>
			static void RegisterClass(LuaState& lsc) {
				RegisterObjectBase(lsc);
				auto* ptr = static_cast<T*>(nullptr);
				lua::LuaExport(lsc, ptr);
			}
			static bool IsObjectBaseRegistered(LuaState& lsc);
			static bool IsUpdaterObjectRegistered(LuaState& lsc);
			//! C++クラス登録基盤を初期化
			static void RegisterObjectBase(LuaState& lsc);
			static void RegisterUpdaterObject(LuaState& lsc);
			//! ベースオブジェクトを使った派生クラスの読み込み
			/*! 1クラス1ファイルの対応
				ベースクラスの名前はファイルに記載 */
			static void LoadClass(LuaState& lsc, const std::string& name, const HRW& hRW);
			//! 固有オブジェクトのインポート
			/*! ポインタ指定でLuaにクラスを取り込む
				リソースマネージャやシステムクラス用 */
			template <class T>
			static void ImportClass(LuaState& lsc, const std::string& tableName, const std::string& name, T* ptr) {
				RegisterObjectBase(lsc);

				const int stk = lsc.getTop();
				const auto* dummy = static_cast<T*>(nullptr);
				lua::LuaExport(lsc, dummy);
				if(ptr) {
					MakePointerInstance(lsc, lua::LuaName(dummy), ptr);
					// [Instance]
					if(!tableName.empty())
						lsc.prepareTableGlobal(tableName);
					else
						lsc.pushGlobal();
					lsc.push(name);
					// [Instance][Target][name]
					lsc.pushValue(-3);
					// [Instance][Target][name][Instance]
					lsc.rawSet(-3);
				}
				lsc.setTop(stk);
			}
			static void MakePointerInstance(LuaState& lsc, const std::string& luaName, void* ptr);
			template <class T>
			static void MakePointerInstance(LuaState& lsc, const T* ptr) {
				MakePointerInstance(lsc, lua::LuaName((T*)nullptr), (void*)ptr);
			}
			static void RegisterRSClass(LuaState& lsc);
			//! GObjectメッセージを受信
			/*! Obj(UData), MessageStr, {Args} */
			static int RecvMsgCpp(lua_State* ls);
	};
}
namespace rev {
	template <class T>
	struct LCV_In {
		template <class T2>
		int operator()(lua_State* ls, T2&& t) const {
			LuaState lsc(ls, true);
			lsc.getGlobal(lua::LuaName((T*)nullptr));
			lsc.getField(-1, luaNS::ConstructPtr);
			lsc.newUserData(sizeof(T));
			new(lsc.toUserData(-1)) T(std::forward<T2>(t));
			lsc.call(1, 1);
			lsc.remove(-2);
			return 1;
		}
		// (int, lua_State*)の順なのは、pushの時と引数が被ってしまう為
		T operator()(const int idx, lua_State* ls, LPointerSP* /*spm*/=nullptr) const {
			return *LI_GetPtr<T>()(ls, idx);
		}
		std::ostream& operator()(std::ostream& os, const T& t) const {
			return os << "(userdata) 0x" << std::hex << &t;
		}
		LuaType operator()() const {
			return LuaType::Userdata;
		}
	};
	template <class T>
	struct LCV : LCV_In<T> {};
	// 参照またはポインターの場合はUserdataに格納
	template <class T>
	struct LCV<const T&> : LCV<T> {};
	template <class T>
	struct LCV<const T*> : LCV<T> {
		using base_t = LCV<T>;
		using base_t::operator();
		int operator()(lua_State* ls, const T* t) const {
			return base_t()(ls, *t);
		}
		std::ostream& operator()(std::ostream& os, const T* t) const {
			return base_t()(os, *t);
		}
	};
	// 非const参照またはポインターの場合はLightUserdataに格納
	template <class T>
	struct LCV<T*> {
		int operator()(lua_State* ls, const T* t) const {
			LuaState lsc(ls, true);
			LuaImport::MakePointerInstance(lsc, t);
			return 1;
		}
		T* operator()(const int idx, lua_State* ls, LPointerSP* /*spm*/=nullptr) const {
			return LI_GetPtr<T>()(ls, idx);
		}
		std::ostream& operator()(std::ostream& os, const T* t) const {
			return LCV<T>()(os, *t);
		}
		LuaType operator()() const {
			return LuaType::LightUserdata;
		}
	};
	template <class T>
	struct LCV<T&> : LCV<T*> {
		using base_t = LCV<T*>;
		using base_t::operator();
		int operator()(lua_State* ls, const T& t) const {
			return base_t()(ls, &t);
		}
		T& operator()(const int idx, lua_State* ls, LPointerSP* spm=nullptr) const {
			return *base_t()(idx, ls, spm);
		}
		std::ostream& operator()(std::ostream& os, const T& t) const {
			return base_t()(os, &t);
		}
	};
	template <
		class T,
		ENABLE_IF((
			!std::is_pointer<T>{} &&
			!std::is_reference<T>{} &&
			!std::is_unsigned<T>{}
		))
	>
	std::ostream& operator << (std::ostream& os, const LCV<T>&) {
		return os << typeid(T).name();
	}
	template <
		class T,
		ENABLE_IF(std::is_unsigned<T>{})
	>
	std::ostream& operator << (std::ostream& os, const LCV<T>&) {
		return os << "unsigned " << LCV<typename std::make_signed<T>::type>();
	}
	template <class T>
	std::ostream& operator << (std::ostream& os, const LCV<const T>&) {
		return os << "const " << LCV<T>();
	}
	template <class T>
	std::ostream& operator << (std::ostream& os, const LCV<T*>&) {
		return os << LCV<T>() << '*';
	}
	template <class T>
	std::ostream& operator << (std::ostream& os, const LCV<T&>&) {
		return os << LCV<T>() << '&';
	}
	template <class T>
	std::ostream& operator << (std::ostream& os, const LCV<T&&>&) {
		return os << LCV<T>() << "&&";
	}
}
