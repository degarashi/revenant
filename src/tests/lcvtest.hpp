#pragma once
#include "luatest.hpp"
#include "../sdl_rw.hpp"
#include "../dir.hpp"
#include "lubee/src/logical.hpp"

namespace std {
	template <class T>
	bool operator == (const weak_ptr<T>& p0, const weak_ptr<T>& p1) noexcept {
		return p0.lock() == p1.lock();
	}
}
namespace rev {
	namespace test {
		template <class T>
		bool PreciseCompare(const T& v0, const T& v1) {
			return v0 == v1;
		}
		inline bool PreciseCompare(const LCValue& v0, const LCValue& v1) noexcept {
			return v0.preciseCompare(v1);
		}
		inline bool PreciseCompare(const char* c0, const char* c1) {
			return std::string(c0) == std::string(c1);
		}
		template <class... Ts>
		bool PreciseCompare(const std::tuple<Ts...>& v0, const std::tuple<Ts...>& v1);
		template <class... Ts, std::size_t... Idx>
		bool _PreciseCompare(const std::tuple<Ts...>& v0, const std::tuple<Ts...>& v1, std::index_sequence<Idx...>) {
			return lubee::And_L(PreciseCompare(std::get<Idx>(v0), std::get<Idx>(v1))...);
		}
		template <class... Ts>
		bool PreciseCompare(const std::tuple<Ts...>& v0, const std::tuple<Ts...>& v1) {
			return _PreciseCompare(v0, v1, std::make_index_sequence<sizeof...(Ts)>());
		}
		template <class T0, class T1>
		bool PreciseCompare(const std::pair<T0,T1>& v0, const std::pair<T0,T1>& v1) {
			return PreciseCompare(v0.first, v1.first) &&
					PreciseCompare(v0.second, v1.second);
		}
		struct LCV_Test : LuaTest {
			template <class T>
			void pushTest() {
				LCV<T> lcv;
				auto& lsp = this->_lsp;
				lua_State *const ls = lsp->getLS();
				lsp->checkStack(32);
				constexpr int Max_Other = 8;
				LCValue prev[Max_Other],
						post[Max_Other];
				const int nprev = _rdi({0,Max_Other-1}),
						npost = _rdi({0,Max_Other-1});
				// 前に関係ない値を挟む
				for(int i=0 ; i<nprev ; i++) {
					prev[i] = genLCValue(c_luaTypes_key);
					lsp->push(prev[i]);
				}
				// LCV経由でPush
				const auto v0 = genValue<T>();
				lcv(ls, v0);
				ASSERT_EQ(nprev+1, lsp->getTop());
				// 後に関係ない値を加える
				for(int i=0 ; i<npost ; i++) {
					post[i] = genLCValue(c_luaTypes_key);
					lsp->push(post[i]);
				}
				// LCV経由で値を取得(正のインデックス & 負のインデックス)
				const auto v1 = lcv(nprev+1, ls, nullptr),
							v2 = lcv(lsp->getTop()-npost, ls, nullptr);
				ASSERT_TRUE(PreciseCompare(v0, decltype(v0)(v1)) || PreciseCompare(decltype(v1)(v0), v1));
				ASSERT_TRUE(PreciseCompare(v1, v2));

				// スタックの他の値は変化なし
				for(int i=0 ; i<nprev ; i++)
					ASSERT_TRUE(prev[i].preciseCompare(lsp->toLCValue(i+1)));
				for(int i=0 ; i<npost ; i++)
					ASSERT_TRUE(post[i].preciseCompare(lsp->toLCValue(nprev+1+1+i)));
			}
			template <class T>
			void typeTest() {
				LCV<T> lcv;
				// LCV()が返す型タイプが実際Pushした時と一致しているか確認
				auto& lsp = this->_lsp;
				const auto val = genValue<T>();
				lcv(lsp->getLS(), val);
				ASSERT_EQ(lcv(val), lsp->type(-1));
			}
		};

		class LCV_TestRW : public LCV_Test {
			private:
				static std::unique_ptr<RWMgr> s_rw;
			public:
				static void SetUpTestCase() {
					s_rw = std::make_unique<RWMgr>("APP", "ORG");
				}
				static void TearDownTestCase() {
					s_rw.reset();
				}
				void loadSharedPtrModule() {
					auto& lsp = this->_lsp;
					lsp->addResourcePath(Dir::GetProgramDir() + u8"/resource/sys_script/?.lua");
					lsp->loadModule("shared_ptr");
				}
		};
	}
}
