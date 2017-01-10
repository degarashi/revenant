#pragma once
#include "lcv.hpp"

namespace rev {
	namespace detail {
		// LValue[], *LValueの時だけ生成される中間クラス
		template <class LV, class IDX>
		class LV_Inter {
			private:
				LV&				_src;
				const IDX&		_index;
			public:
				LV_Inter(LV& src, const IDX& index):
					_src(src),
					_index(index)
				{}
				LV_Inter(const LV_Inter&) = delete;
				LV_Inter(LV_Inter&&) = default;

				void prepareValue(lua_State* ls) const {
					_src._prepareAtIndex(ls, _index);
				}
				template <class VAL>
				LV_Inter& operator = (VAL&& v) {
					_src.setField(_index, std::forward<VAL>(v));
					return *this;
				}
				lua_State* getLS() const {
					return _src.getLS();
				}
		};
		// LV_Inter (const版)
		template <class LV, class IDX>
		class LV_Inter<const LV, IDX> : public LV_Inter<LV,IDX> {
			public:
				LV_Inter(const LV& src, const IDX& index):
					LV_Inter<LV,IDX>(const_cast<LV&>(src), index)
				{}
				LV_Inter(const LV_Inter&) = delete;
				LV_Inter(LV_Inter&&) = default;

				template <class VAL>
				LV_Inter& operator = (VAL&& v) = delete;
		};
		//! LValue内部の値をスタックに積み、デストラクタで元に戻す
		template <class T>
		struct VPop {
			const T&	self;
			int			index;
			VPop(const T& s, const bool bTop):
				self(s)
			{
				index = s._prepareValue(bTop);
			}
			operator int() const noexcept {
				return index;
			}
			int getIndex() const noexcept {
				return index;
			}
			~VPop() {
				self._cleanValue(index);
			}
		};
	}
	class LV_Global {
		private:
			const static std::string	cs_entry;
			static lubee::Freelist<int>	s_index;
			Lua_SP		_lua;
			int			_id;

			void _init(const Lua_SP& sp);
		public:
			using VPop = detail::VPop<LV_Global>;
			friend VPop;
		protected:
			// 代入する値をスタックの先頭に置いた状態で呼ぶ
			void _setValue();
			// 当該値をスタックに置く
			int _prepareValue(bool bTop) const;
			void _cleanValue(int pos) const;
			lua_State* _prepareValue(lua_State* ls) const;
		public:
			explicit LV_Global(lua_State* ls);
			// スタックトップの値を管理対象とする
			LV_Global(const Lua_SP& sp);
			// 引数の値を管理対象とする
			LV_Global(const Lua_SP& sp, const LCValue& lcv);
			template <class LV>
			LV_Global(const Lua_SP& sp, const LValue<LV>& lv) {
				lv.prepareValue(sp->getLS());
				_init(sp);
			}
			template <class LV, class IDX>
			LV_Global(detail::LV_Inter<LValue<LV>,IDX>&& lv) {
				lua_State* ls = lv.getLS();
				lv.prepareValue(ls);
				_init(LuaState::GetLS_SP(ls));
			}
			LV_Global(const LV_Global& lv);
			LV_Global(LV_Global&& lv) noexcept;
			~LV_Global();
			LV_Global& operator = (const LV_Global& lv);
			LV_Global& operator = (LV_Global&& lv) noexcept;

			// lua_State*をゲットする関数
			lua_State* getLS() const;
			void swap(LV_Global& lv) noexcept;
			friend std::ostream& operator << (std::ostream& os, const LV_Global& t);
	};
	class LV_Stack {
		private:
			lua_State*	_ls;
			int			_pos;
			#ifdef DEBUG
				LuaType	_type;
			#endif

			void _invalidate();
		public:
			using VPop = detail::VPop<LV_Stack>;
			friend VPop;
		protected:
			void _setValue();
			void _init(lua_State* ls);
			int _prepareValue(bool bTop) const;
			void _cleanValue(int pos) const;
			lua_State* _prepareValue(lua_State* ls) const;
		public:
			explicit LV_Stack(lua_State* ls);
			LV_Stack(lua_State* ls, const LCValue& lcv);
			template <class LV, class IDX>
			LV_Stack(detail::LV_Inter<LValue<LV>,IDX>&& lv) {
				lua_State* ls = lv.getLS();
				lv.prepareValue(ls);
				_init(ls);
			}
			LV_Stack(LV_Stack&& lv) noexcept;
			LV_Stack(const LV_Stack&);
			~LV_Stack();

			LV_Stack& operator = (const LV_Stack& lv);
			LV_Stack& operator = (LV_Stack&& lv);

			lua_State* getLS() const;
			void swap(LV_Stack& lv) noexcept;
			friend std::ostream& operator << (std::ostream& os, const LV_Stack& t);
	};
	namespace detail {
		template <class T>
		struct LValue_CharN {
			using type = T;
		};
		template <int N>
		struct LValue_CharN<char [N]> {
			using type = const char*;
		};
	}
	//! LuaState内部に値を保持する
	template <class T>
	class LValue : public T {
		private:
			template <class LV, class IDX>
			friend class detail::LV_Inter;
			using base_t = T;
			template <class LV>
			bool _compare(const LValue<LV>& lv, const int flag) const {
				lua_State* ls = base_t::getLS();
				const RewindTop rt(ls);
				prepareValue(ls);
				lv.prepareValue(ls);
				return lua_compare(ls, -2, -1, flag);
			}
			//! この値が指すテーブルからidxが指す要素をスタックに積む
			template <class IDX>
			void _prepareAtIndex(lua_State* ls, const IDX& idx) const {
				D_Assert0(type() == LuaType::Table);
				prepareValue(ls);
				idx.push(ls);
				// [This][Idx]
				lua_gettable(ls, -2);
				lua_remove(ls, -2);
			}
		public:
			template <class Callback>
			void iterateTable(Callback&& cb) const {
				LuaState lsc(base_t::getLS(), true);
				typename base_t::VPop vp(*this, true);
				// LValueの値がテーブル以外の時は処理しない
				if(lsc.type(-1) == LuaType::Table) {
					lsc.push(LuaNil());
					while(lsc.next(vp.getIndex()) != 0) {
						cb(lsc);
						lsc.pop(1);
					}
				}
			}
			using base_t::base_t;
			template <class LV>
			LValue(const LValue<LV>& lv):
				base_t(lv.prepareValue(lv.getLS()))
			{}
			template <class LV>
			LValue(LValue<LV>&& lv):
				base_t(lv.prepareValue(lv.getLS()))
			{}
			LValue(const LValue& lv):
				base_t(lv.prepareValue(lv.getLS()))
			{}
			LValue(LValue&& lv) noexcept:
				base_t(std::move(static_cast<base_t&>(lv)))
			{}

			LValue& operator = (lua_State *const ls) {
				return *this = LCValue(ls);
			}
			LValue& operator = (const LCValue& lcv) {
				lcv.push(base_t::getLS());
				base_t::_setValue();
				return *this;
			}
			template <class LV>
			LValue& operator = (const LValue<LV>& t) {
				if(auto* ls = base_t::getLS()) {
					t.prepareValue(ls);
					base_t::_setValue();
				} else {
					this->~LValue();
					new(this) LValue(t);
				}
			}
			LValue& operator = (const LValue& lv) {
				static_cast<base_t&>(*this) = static_cast<const base_t&>(lv);
				return *this;
			}
			LValue& operator = (LValue&& lv) {
				static_cast<base_t&>(*this) = std::move(static_cast<base_t&>(lv));
				return *this;
			}
			template <class LV>
			auto operator [](const LValue<LV>& lv) {
				return detail::LV_Inter<LValue<LV>, LValue<LV>>(*this, lv);
			}
			template <class LV>
			auto operator [](const LValue<LV>& lv) const {
				return detail::LV_Inter<const LValue<LV>, LValue<LV>>(*this, lv);
			}
			auto operator [](const LCValue& lcv) {
				return detail::LV_Inter<LValue<base_t>, LCValue>(*this, lcv);
			}
			auto operator [](const LCValue& lcv) const {
				return detail::LV_Inter<const LValue<base_t>, LCValue>(*this, lcv);
			}
			//! LValueが指す関数を任意の引数で呼ぶ
			/*! 戻り値を引数として与えたstd::tupleで返す */
			template <class... Ret, class... Args>
			void operator()(std::tuple<Ret...>& dst, Args&&... args) const {
				LuaState lsc(base_t::getLS(), true);
				base_t::_prepareValue(true);
				// 引数をスタックに積む
				lsc.pushArgs(std::forward<Args>(args)...);
				lsc.call(sizeof...(Args), sizeof...(Ret));
				// 戻り値をtupleにセットする
				lsc.popValues(dst);
			}
			//! LValueが指す関数を任意の引数で呼ぶ
			/*! 戻り値をstd::tupleとして返す */
			template <class... Ret, class... Args>
			std::tuple<Ret...> call(Args&&... args) const {
				std::tuple<Ret...> ret;
				this->operator()(ret, std::forward<Args>(args)...);
				return ret;
			}
			//! LValueが指す関数を任意の引数で呼ぶ
			/*! 戻り値をLCValueとして返す */
			template <class... Args>
			LCTable_SP callNRet(Args&&... args) const {
				const RewindTop rt(base_t::getLS());
				LuaState lsc(base_t::getLS(), false);
				const int top = lsc.getTop();
				base_t::_prepareValue(true);
				// 引数をスタックに積んで関数コール
				lsc.pushArgs(std::forward<Args>(args)...);
				lsc.call(sizeof...(Args), LUA_MULTRET);
				const int nRet = lsc.getTop() - top;
				if(nRet == 0)
					return nullptr;
				// 戻り値はテーブルで返す
				auto spTbl = std::make_shared<LCTable>();
				for(int i=0 ; i<nRet ; i++)
					(*spTbl)[i+1] = lsc.toLCValue(top + i + 1);
				return spTbl;
			}
			//! LValueが指すテーブルからmethodにある関数を自身を第一引数にして呼ぶ
			/*! 戻り値をstd::tupleとして返す */
			template <class... Ret, class... Args>
			std::tuple<Ret...> callMethod(const std::string& method, Args&&... args) const {
				LValue lv = (*this)[method];
				return lv.call<Ret...>(*this, std::forward<Args>(args)...);
			}
			//! LValueが指すテーブルからmethodにある関数を自身を第一引数にして呼ぶ
			/*! 戻り値をLCValueとして返す */
			template <class... Args>
			LCValue callMethodNRet(const std::string& method, Args&&... args) const {
				LValue lv = (*this)[method];
				return lv.callNRet(*this, std::forward<Args>(args)...);
			}

			// --- convert function ---
			#define DEF_FUNC(typ, name) typ name() const { \
				return LCV<typ>()(typename base_t::VPop(*this, true), base_t::getLS(), nullptr); }
			DEF_FUNC(bool, toBoolean)
			DEF_FUNC(lua_Integer, toInteger)
			DEF_FUNC(lua_Number, toNumber)
			DEF_FUNC(void*, toUserData)
			DEF_FUNC(const char*, toString)
			DEF_FUNC(LCValue, toLCValue)
			DEF_FUNC(LCTable_SP, toTable)
			DEF_FUNC(Lua_SP, toThread)
			#undef DEF_FUNC

			void push(lua_State* ls) const {
				prepareValue(ls);
			}
			lua_State* prepareValue(lua_State* ls) const {
				return base_t::_prepareValue(ls);
			}
			template <class IDX, class VAL>
			void setField(const IDX& idx, const VAL& val) {
				lua_State* ls = base_t::getLS();
				typename base_t::VPop vp(*this, true);
				GetLCVType<typename detail::LValue_CharN<IDX>::type>()(ls, idx);
				GetLCVType<typename detail::LValue_CharN<VAL>::type>()(ls, val);
				lua_settable(ls, vp.getIndex());
			}

			const void* toPointer() const {
				return lua_topointer(base_t::getLS(), typename base_t::VPop(*this, false));
			}
			template <class R>
			decltype(auto) toValue() const {
				return GetLCVType<R>()(typename base_t::VPop(*this, false), base_t::getLS(), nullptr);
			}
			int length() const {
				typename base_t::VPop vp(*this, true);
				auto* ls = base_t::getLS();
				lua_len(ls, vp.getIndex());
				const int ret = lua_tointeger(ls, -1);
				lua_pop(ls, 1);
				return ret;
			}
			LuaType type() const {
				return LuaState::SType(base_t::getLS(), typename base_t::VPop(*this, false));
			}
			template <class LV>
			bool operator == (const LValue<LV>& lv) const {
				return _compare(lv, LUA_OPEQ);
			}
			template <class LV>
			bool operator < (const LValue<LV>& lv) const {
				return _compare(lv, LUA_OPLT);
			}
			template <class LV>
			bool operator <= (const LValue<LV>& lv) const {
				return _compare(lv, LUA_OPLE);
			}
			template <class LV>
			bool operator != (const LValue<LV>& lv) const {
				return !(this->operator == (lv));
			}
	};
	using LValueS = LValue<LV_Stack>;
	using LValueG = LValue<LV_Global>;
	std::ostream& operator << (std::ostream& os, const LV_Stack& t);
	std::ostream& operator << (std::ostream& os, const LV_Global& t);
}
