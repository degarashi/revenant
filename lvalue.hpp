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
				LV_Inter(LV& src, const IDX& index): _src(src), _index(index) {}
				LV_Inter(const LV_Inter&) = delete;
				LV_Inter(LV_Inter&&) = default;

				void prepareValue(lua_State* ls) const {
					_src.prepareAt(ls, _index);
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
					LV_Inter<LV,IDX>(const_cast<LV&>(src), index) {}
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
			void _prepareValue(lua_State* ls) const;
			void _cleanValue(int pos) const;
		public:
			LV_Global(lua_State* ls);
			// スタックトップの値を管理対象とする
			LV_Global(const Lua_SP& sp);
			// 引数の値を管理対象とする
			LV_Global(const Lua_SP& sp, const LCValue& lcv);
			template <class LV>
			LV_Global(const Lua_SP& sp, const LValue<LV>& lv) {
				lv.prepareValue(sp->getLS());
				_init(sp);
			}
			LV_Global(const LV_Global& lv);
			LV_Global(LV_Global&& lv);
			~LV_Global();

			template <class LV>
			LV_Global& operator = (const LValue<LV>& lcv) {
				lua_State* ls = _lua->getLS();
				lcv.prepareValue(ls);
				return *this;
			}
			template <class T2>
			LV_Global& operator = (T2&& t) {
				_lua->getGlobal(cs_entry);
				_lua->push(_id);
				_lua->push(std::forward<T2>(t));
				_lua->setTable(-3);
				_lua->pop(1);
				return *this;
			}
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
		public:
			using VPop = detail::VPop<LV_Stack>;
			friend VPop;
		protected:
			void _setValue();
			void _init(lua_State* ls);
			int _prepareValue(bool bTop) const;
			void _prepareValue(lua_State* ls) const;
			void _cleanValue(int pos) const;
		public:
			LV_Stack(lua_State* ls);
			LV_Stack(lua_State* ls, const LCValue& lcv);
			template <class LV, class IDX>
			LV_Stack(detail::LV_Inter<LValue<LV>,IDX>&& lv) {
				lua_State* ls = lv.getLS();
				lv.prepareValue(ls);
				_init(ls);
			}
			template <class LV>
			LV_Stack(lua_State* ls, const LValue<LV>& lv) {
				lv.prepareValue(ls);
				_init(ls);
			}
			LV_Stack(const LV_Stack& lv);
			~LV_Stack();

			template <class LV>
			LV_Stack& operator = (const LValue<LV>& lcv) {
				lcv.prepareValue(_ls);
				_setValue();
				return *this;
			}
			LV_Stack& operator = (const LCValue& lcv);
			LV_Stack& operator = (lua_State* ls);

			lua_State* getLS() const;
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
		public:
			template <class Callback>
			void iterateTable(Callback&& cb) const {
				LuaState lsc(T::getLS());
				typename T::VPop vp(*this, true);
				// LValueの値がテーブル以外の時は処理しない
				if(lsc.type(-1) == LuaType::Table) {
					lsc.push(LuaNil());
					while(lsc.next(vp.getIndex()) != 0) {
						cb(lsc);
						lsc.pop(1);
					}
				}
			}
			using T::T;
			LValue(const LValue& lv):
				T(lv)
			{}
			LValue(LValue&& lv):
				T(std::move(lv))
			{}
			LValue& operator = (const LCValue& lcv) {
				lcv.push(T::getLS());
				T::_setValue();
				return *this;
			}
			template <class LV>
			LValue& operator = (const LValue<LV>& lv) {
				return reinterpret_cast<LValue&>(T::operator =(lv));
			}
			LValue& operator = (LValue&& lv) noexcept {
				static_cast<T&>(*this).swap(static_cast<T&>(lv));
				return *this;
			}
			template <class T2>
			LValue& operator = (T2&& t) {
				static_cast<T&>(*this) = std::forward<T2>(t);
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
				return detail::LV_Inter<LValue<T>, LCValue>(*this, lcv);
			}
			auto operator [](const LCValue& lcv) const {
				return detail::LV_Inter<const LValue<T>, LCValue>(*this, lcv);
			}
			template <class... Ret, class... Args>
			void operator()(std::tuple<Ret...>& dst, Args&&... args) {
				LuaState lsc(T::getLS());
				T::_prepareValue(true);
				// 引数をスタックに積む
				lsc.pushArgs(std::forward<Args>(args)...);
				lsc.call(sizeof...(Args), sizeof...(Ret));
				// 戻り値をtupleにセットする
				lsc.popValues(dst);
			}
			template <class... Ret, class... Args>
			std::tuple<Ret...> call(Args&&... args) {
				std::tuple<Ret...> ret;
				this->operator()(ret, std::forward<Args>(args)...);
				return ret;
			}
			template <class... Args>
			LCTable_SP callNRet(Args&&... args) {
				LuaState lsc(T::getLS());
				const int top = lsc.getTop();
				T::_prepareValue(true);
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
				lsc.setTop(top);
				return spTbl;
			}
			template <class... Ret, class... Args>
			std::tuple<Ret...> callMethod(const std::string& method, Args&&... args) {
				LValue lv = (*this)[method];
				return lv.call<Ret...>(*this, std::forward<Args>(args)...);
			}
			template <class... Args>
			LCValue callMethodNRet(const std::string& method, Args&&... args) {
				LValue lv = (*this)[method];
				return lv.callNRet(*this, std::forward<Args>(args)...);
			}

			// --- convert function ---
			#define DEF_FUNC(typ, name) typ name() const { \
				return LCV<typ>()(typename T::VPop(*this, true), T::getLS()); }
			DEF_FUNC(bool, toBoolean)
			DEF_FUNC(lua_Integer, toInteger)
			DEF_FUNC(lua_Number, toNumber)
			DEF_FUNC(void*, toUserData)
			DEF_FUNC(const char*, toString)
			DEF_FUNC(LCValue, toLCValue)
			DEF_FUNC(Lua_SP, toThread)
			#undef DEF_FUNC

			void prepareValue(lua_State* ls) const {
				T::_prepareValue(ls);
			}
			template <class IDX>
			void prepareAt(lua_State* ls, const IDX& idx) const {
				T::_prepareValue(ls);
				idx.push(ls);
				lua_gettable(ls, -2);
				lua_remove(ls, -2);
			}
			template <class IDX, class VAL>
			void setField(const IDX& idx, const VAL& val) {
				lua_State* ls = T::getLS();
				typename T::VPop vp(*this, true);
				GetLCVType<typename detail::LValue_CharN<IDX>::type>()(ls, idx);
				GetLCVType<typename detail::LValue_CharN<VAL>::type>()(ls, val);
				lua_settable(ls, vp.getIndex());
			}

			const void* toPointer() const {
				return lua_topointer(T::getLS(), typename T::VPop(*this, false));
			}
			template <class R>
			decltype(auto) toValue() const {
				return detail::GetLCVType<R>()(typename T::VPop(*this, false), T::getLS());
			}
			int length() const {
				typename T::VPop vp(*this, true);
				auto* ls = T::getLS();
				lua_len(ls, vp.getIndex());
				int ret = lua_tointeger(ls, -1);
				lua_pop(ls, 1);
				return ret;
			}
			LuaType type() const {
				return LuaState::SType(T::getLS(), typename T::VPop(*this, false));
			}
	};
	using LValueS = LValue<LV_Stack>;
	using LValueG = LValue<LV_Global>;
	std::ostream& operator << (std::ostream& os, const LV_Stack& t);
	std::ostream& operator << (std::ostream& os, const LV_Global& t);
}
