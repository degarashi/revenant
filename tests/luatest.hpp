#pragma once
#include "test.hpp"
#include "lubee/random/string.hpp"
#include "../lcv.hpp"

namespace rev {
	namespace test {
		extern const LuaType c_luaTypes[8];
		extern const LuaType c_luaTypes_nonil[7];
		extern const LuaType c_luaTypes_key[6];
		#define NUM_CFUNC 16
		struct CFunction {
			constexpr static int N_Func = NUM_CFUNC;
			#define PP_CFUNC(z,n,data) \
				static int BOOST_PP_CAT(data,n)(lua_State* ls) { \
					lua_pushinteger(ls, n); \
					return 1; \
				}
			static lua_CFunction cs_func[N_Func];
			BOOST_PP_REPEAT(NUM_CFUNC, PP_CFUNC, Func)
			#undef PP_CFUNC
		};

		template <class T>
		struct GenIntegralValue;
		template <class T>
		struct GenValue;
		using LTypes = lubee::Types<bool, void_sp, void_wp>;
		template <
			class T,
			ENABLE_IF(
				!LTypes::Has<T>{} &&
				(std::is_integral<T>{} || std::is_floating_point<T>{})
			)
		>
		GenIntegralValue<T> GenValueTF(T*);
		template <
			class T,
			ENABLE_IF(
				LTypes::Has<T>{} ||
				!(std::is_integral<T>{} || std::is_floating_point<T>{})
			)
		>
		GenValue<T> GenValueTF(T*);
		template <class T>
		using GenValue_t = decltype(GenValueTF((T*)nullptr));

		class LuaTest : public Random {
			protected:
				Lua_SP			_lsp;
				using RdI = decltype(std::declval<Random>().mt().template getUniformF<int>());
				RdI				_rdi;
				int				_maxTableEnt=16;
				//! 生成するconst char*文字列ストア
				using StrV = std::vector<std::string>;
				StrV			_stringVec;
				using LsV = std::vector<Lua_SP>;
				LsV				_lsVec;
				using SpV = std::vector<void_sp>;
				SpV				_spVec;

				LuaTest();
			public:
				lua_State* getLS();
				const Lua_SP& getLSP();
				template <class T>
				friend struct GenValue;
				template <class T>
				friend struct GenIntegralValue;
				// ----------- GenValue -----------
				template <class V>
				V genValue();
				template <class CB>
				void genLuaValue(const LuaType typ, const CB& cb) {
					switch(typ) {
						case LuaType::Nil:
							cb(LuaNil());
							break;
						case LuaType::Number:
							cb(genValue<lua_Number>());
							break;
						case LuaType::Boolean:
							cb(genValue<bool>());
							break;
						case LuaType::String:
							cb(genValue<std::string>());
							break;
						case LuaType::Table:
							cb(genValue<LCTable_SP>());
							break;
						case LuaType::Function:
							cb(genValue<lua_CFunction>());
							break;
						case LuaType::Thread:
							cb(genValue<Lua_SP>());
							break;
						case LuaType::LightUserdata:
							cb(genValue<void*>());
							break;
						default:
							AssertF("not supported");
					}
				}

				template <int N, class CB>
				void genLuaValue(const LuaType (&cand)[N], const CB& cb) {
					genLuaValue(cand[_rdi({0, N-1})], cb);
				}
				template <class T>
				LCValue genLCValue(const T& t) {
					LCValue lcv;
					genLuaValue(t, [&lcv](auto&& val){
						lcv = val;
					});
					return lcv;
				}
				template <int N>
				LCValue genLCValue(const LuaType (&cand)[N]) {
					return genLCValue(cand[_rdi({0, N-1})]);
				}
				//! スタックへランダムに値をPush
				template <int N>
				int makeRandomValues(const Lua_SP& ls, const LuaType (&cand)[N], const int n) {
					for(int i=0 ; i<n ; i++)
						ls->push(genLCValue(cand));
					return n;
				}
				template <class T>
				static auto GetValue(T& p, const int idx, bool*) {
					return p->toBoolean(idx);
				}
				template <class T, class V, ENABLE_IF(std::is_integral<V>{})>
				static auto GetValue(T& p, const int idx, V*) {
					return p->toInteger(idx);
				}
				template <class T, class V, ENABLE_IF(std::is_floating_point<V>{})>
				static auto GetValue(T& p, const int idx, V*) {
					return p->toNumber(idx);
				}
				template <class T>
				static auto GetValue(T& p, const int idx, lua_CFunction*) {
					return p->toCFunction(idx);
				}
				template <class T>
				static auto GetValue(T& p, const int idx, std::string*) {
					return p->toString(idx);
				}
				template <class T>
				static auto GetValue(T& p, const int idx, void**) {
					return p->toUserData(idx);
				}
				template <class T>
				static auto GetValue(T& p, const int idx, Lua_SP*) {
					return p->toThread(idx);
				}
				template <class T>
				static auto GetValue(T& p, const int idx, LCTable_SP*) {
					return p->toTable(idx);
				}
		};
		template <class T>
		struct GenIntegralValue {
			T operator()(LuaTest& self) const {
				using L = std::numeric_limits<T>;
				return self.mt().template getUniform<T>({L::lowest()/4, L::max()/4});
			}
		};
		template <class T>
		struct GenValue<std::shared_ptr<T>> {
			std::shared_ptr<T> operator()(LuaTest& self) const {
				return std::make_shared<T>(GenValue_t<T>()(self));
			}
		};
		template <class T>
		struct GenValue<std::weak_ptr<T>> {
			std::weak_ptr<T> operator()(LuaTest& self) const {
				auto sp = GenValue_t<std::shared_ptr<T>>()(self);
				self._spVec.emplace_back(sp);
				return sp;
			}
		};
		template <>
		struct GenValue<LuaNil> {
			LuaNil operator()(LuaTest&) const;
		};
		template <>
		struct GenValue<bool> {
			bool operator()(LuaTest& self) const;
		};
		template <>
		struct GenValue<void_sp> {
			void_sp operator()(LuaTest& self) const;
		};
		template <>
		struct GenValue<void_wp> {
			void_wp operator()(LuaTest& self) const;
		};
		template <>
		struct GenValue<lua_CFunction> {
			lua_CFunction operator()(LuaTest& self) const;
		};
		template <>
		struct GenValue<std::string> {
			std::string operator()(LuaTest& self) const;
		};
		template <>
		struct GenValue<void*> {
			void* operator()(LuaTest& self) const;
		};
		template <>
		struct GenValue<Lua_SP> {
			Lua_SP operator()(LuaTest& self) const;
		};
		template <>
		struct GenValue<const char*> {
			const char* operator()(LuaTest& self) const;
		};
		template <>
		struct GenValue<lua_State*> {
			lua_State* operator()(LuaTest& self) const;
		};
		template <>
		struct GenValue<LCValue> {
			LCValue operator()(LuaTest& self) const;
		};
		template <>
		struct GenValue<LCTable_SP> {
			LCTable_SP operator()(LuaTest& self) const;
		};
		template <class V>
		V LuaTest::genValue() {
			return GenValue_t<V>()(*this);
		}

		template <class T>
		class LuaTestT : public LuaTest {
			public:
				using lua_t = T;
				template <class T2=lua_t, ENABLE_IF((std::is_same<T2,LValueS>{}))>
				lua_State* getLSZ(const Lua_SP& lsp = nullptr) const {
					if(lsp)
						return lsp->getLS();
					return _lsp->getLS();
				}
				template <class T2=lua_t, ENABLE_IF((std::is_same<T2,LValueG>{}))>
				Lua_SP getLSZ(const Lua_SP& lsp = nullptr) const {
					if(lsp)
						return lsp;
					return _lsp;
				}
		};

		// テーブルの比較はポインタではなく参照先にする
		// std::stringで入力した値はconst char*で取得される為、専用関数を使う
		bool operator == (const LCTable_SP& s0, const LCTable_SP& s1) noexcept;
	}
}
