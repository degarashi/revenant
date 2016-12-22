#include "lcvtest.hpp"
#include "../luaimpl.hpp"

namespace rev {
	namespace test {
		class MyClass : public Resource {
			public:
				static int s_called;
				using T0 = int;
				using T1 = float;
				using T2 = double;
				T0 value;
				T1 value1;
				T2 value2;

				// メンバ変数をそれぞれ引数倍する
				T0 mFunc(const T0& r) {
					value *= r;
					value1 *= r;
					value2 *= r;
					return r;
				}
				// 引数をそのまま返す
				static T1 sFunc(const T1& t) {
					return t;
				}
				// Luaに登録するほうのコンストラクタ
				MyClass(const T0 v0, const T1 v1, const T2 v2):
					value(v0),
					value1(v1),
					value2(v2)
				{}
				// 関係ない(Luaには登録しない)コンストラクタ
				MyClass(const char*):
					value(100),
					value1(200),
					value2(300)
				{}
				bool operator == (const MyClass& m) const noexcept {
					return value == m.value &&
							value1 == m.value1 &&
							value2 == m.value2;
				}
				const char* getResourceName() const noexcept {
					return "MyClass";
				}
				static void LuaExport(LuaState&) {
					++s_called;
				}
		};
		int MyClass::s_called;
	}
	class MyResMgr : public spi::ResMgr<Resource>, public spi::Singleton<MyResMgr> {};
}
DEF_LUAIMPORT(rev::test::MyClass)
DEF_LUAIMPLEMENT_SPTR(
	::rev::MyResMgr,
	::rev::test::MyClass, MyClass,
	LUAIMPLEMENT_BASE,
	(value)(value1),
	(mFunc)(sFunc),
	(typename ::rev::test::MyClass::T0)
	(typename ::rev::test::MyClass::T1)
	(typename ::rev::test::MyClass::T2)
)

namespace rev {
	namespace test {
		template <class T>
		struct LCV_ClassTest : LCV_TestRW<T> {
			using value_t = T;
			using T0 = typename value_t::T0;
			using T1 = typename value_t::T1;
			using T2 = typename value_t::T2;
			void SetUp() override {
				value_t::s_called = 0;
				this->loadSharedPtrModule();
			}
		};
		using TypesC = ::testing::Types<
			MyClass
		>;

		TYPED_TEST_CASE(LCV_ClassTest, TypesC);
		TYPED_TEST(LCV_ClassTest, Member) {
			USING(value_t);
			USING(T0);
			USING(T1);
			USING(T2);
			auto& lsp = this->_lsp;
			LuaImport::RegisterClass<value_t>(*lsp);

			const auto v0 = this->template genValue<T0>();
			const auto v1 = this->template genValue<T1>();
			const auto v2 = this->template genValue<T2>();
			value_t myc(v0, v1, v2);
			LCV<value_t>()(lsp->getLS(), myc);
			// メンバ変数 "value" の読み込みテスト
			lsp->getField(-1, "value");
			ASSERT_EQ(LCV<T0>()(v0), lsp->type(-1));
			ASSERT_EQ(LCValue(typename LCV<T0>::value_t(v0)), lsp->toLCValue(-1));
			lsp->pop(1);

			// メンバ変数 "value1" の読み込みテスト
			lsp->getField(-1, "value1");
			ASSERT_EQ(LCV<T1>()(v1), lsp->type(-1));
			ASSERT_EQ(LCValue(typename LCV<T1>::value_t(v1)), lsp->toLCValue(-1));
			lsp->pop(1);

			// value2は登録してないのでnilが返る
			lsp->getField(-1, "value2");
			ASSERT_EQ(LuaType::Nil, lsp->type(-1));
			lsp->pop(1);

			// メンバ変数の書き込みテスト
			const auto v01 = this->template genValue<T0>();
			const auto v11 = this->template genValue<T1>();
			lsp->setField(-1, "value", v01);
			lsp->setField(-1, "value1", v11);

			const value_t myc2 = LCV<value_t>()(-1, lsp->getLS());
			ASSERT_EQ(myc2.value, v01);
			ASSERT_EQ(myc2.value1, v11);
		}
		TYPED_TEST(LCV_ClassTest, PushPop) {
			USING(value_t);
			USING(T0);
			USING(T1);
			USING(T2);
			auto& lsp = this->_lsp;
			// クラスの登録と、LuaExport関数が呼ばれたかの確認
			ASSERT_EQ(0, value_t::s_called);
			LuaImport::RegisterClass<value_t>(*lsp);
			ASSERT_EQ(1, value_t::s_called);

			const auto v0 = this->template genValue<T0>();
			const auto v1 = this->template genValue<T1>();
			const auto v2 = this->template genValue<T2>();
			value_t myc(v0, v1, v2);

			lua_State *const ls = lsp->getLS();
			// 3種類のPush
			// [1]値渡し
			LCV<value_t>()(ls, myc);
			// [2]ポインタ渡し
			LCV<value_t*>()(ls, &myc);
			// [3]参照渡し
			LCV<value_t&>()(ls, myc);

			// 3種類のPop
			auto ret0 = LCV<value_t>()(1, ls, nullptr);
			auto ret1 = LCV<value_t*>()(2, ls, nullptr);
			decltype(auto) ret2 = LCV<value_t&>()(3, ls, nullptr);

			// 値を比較
			ASSERT_EQ(myc, ret0);
			ASSERT_EQ(myc, *ret1);
			ASSERT_EQ(myc, ret2);

			// [2], [3]のケースでは元の値を改変したらret1, ret2も追従する
			value_t myc_prev(myc);
			myc.value = this->template genValue<T0>();
			myc.value1 = this->template genValue<T1>();
			myc.value2 = this->template genValue<T2>();
			ASSERT_EQ(myc, *ret1);
			ASSERT_EQ(myc, ret2);

			// [1]のケースは値渡しなので元の値のまま
			ASSERT_EQ(myc_prev, ret0);
		}
		TYPED_TEST(LCV_ClassTest, MemberFunction) {
			USING(value_t);
			USING(T0);
			USING(T1);
			USING(T2);
			auto& lsp = this->_lsp;
			LuaImport::RegisterClass<value_t>(*lsp);

			auto v0 = this->template genValue<T0>();
			auto v1 = this->template genValue<T1>();
			auto v2 = this->template genValue<T2>();
			const value_t myc(v0, v1, v2);
			LCV<value_t>()(lsp->getLS(), myc);

			// Luaを通してmember functionを呼ぶ
			lsp->getField(-1, "mFunc");
			const auto r = this->template genValue<T0>();
			lsp->pushValue(-2);
			lsp->push(r);
			lsp->call(2,1);
			// 引数と同じ値が返って来ている筈
			ASSERT_EQ(LCV<T0>()(r), lsp->type(-1));
			const auto myc2 = LCV<value_t>()(1, lsp->getLS());
			// クラス内部の変数はそれぞれr倍になっている
			ASSERT_EQ(v0*=r, myc2.value);
			ASSERT_EQ(v1*=r, myc2.value1);
			ASSERT_EQ(v2*=r, myc2.value2);
		}
		TYPED_TEST(LCV_ClassTest, StaticFunction) {
			USING(value_t);
			USING(T0);
			USING(T1);
			USING(T2);
			auto& lsp = this->_lsp;
			LuaImport::RegisterClass<value_t>(*lsp);

			const auto v0 = this->template genValue<T0>();
			const auto v1 = this->template genValue<T1>();
			const auto v2 = this->template genValue<T2>();
			const value_t myc(v0, v1, v2);
			LCV<value_t>()(lsp->getLS(), myc);

			// Luaを通してstatic functionを呼ぶ
			lsp->getField(-1, "sFunc");
			const auto r = this->template genValue<T0>();
			lsp->push(r);
			lsp->call(1,1);
			// 引数と同じ値が返って来ている筈
			ASSERT_EQ(LCV<T0>()(r), lsp->type(-1));
			// クラス内部の変数は変化なし
			const value_t myc2(v0, v1, v2);
			const auto* ret = LCV<value_t*>()(1, lsp->getLS());
			ASSERT_EQ(myc2, *ret);
		}
	}
}
