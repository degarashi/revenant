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
			void SetUp() override {
				MyClass::s_called = 0;
			}
		};
		using TypesC = ::testing::Types<
			MyClass
		>;

		TYPED_TEST_CASE(LCV_ClassTest, TypesC);
		TYPED_TEST(LCV_ClassTest, PushPop) {
			auto& lsp = this->_lsp;
			this->loadSharedPtrModule();

			// クラスの登録と、LuaExport関数が呼ばれたかの確認
			ASSERT_EQ(0, MyClass::s_called);
			LuaImport::RegisterClass<MyClass>(*lsp);
			ASSERT_EQ(1, MyClass::s_called);

			using T0 = typename ::rev::test::MyClass::T0;
			using T1 = typename ::rev::test::MyClass::T1;
			using T2 = typename ::rev::test::MyClass::T2;
			const auto v0 = this->template genValue<T0>();
			const auto v1 = this->template genValue<T1>();
			const auto v2 = this->template genValue<T2>();
			MyClass myc(v0, v1, v2);

			lua_State *const ls = lsp->getLS();
			// 3種類のPush
			// [1]値渡し
			LCV<MyClass>()(ls, myc);
			// [2]ポインタ渡し
			LCV<MyClass*>()(ls, &myc);
			// [3]参照渡し
			LCV<MyClass&>()(ls, myc);

			// 3種類のPop
			auto ret0 = LCV<MyClass>()(1, ls, nullptr);
			auto ret1 = LCV<MyClass*>()(2, ls, nullptr);
			decltype(auto) ret2 = LCV<MyClass&>()(3, ls, nullptr);

			// 値を比較
			ASSERT_EQ(myc, ret0);
			ASSERT_EQ(myc, *ret1);
			ASSERT_EQ(myc, ret2);

			// [2], [3]のケースでは元の値を改変したらret1, ret2も追従する
			MyClass myc_prev(myc);
			myc.value = this->template genValue<T0>();
			myc.value1 = this->template genValue<T1>();
			myc.value2 = this->template genValue<T2>();
			ASSERT_EQ(myc, *ret1);
			ASSERT_EQ(myc, ret2);

			// [1]のケースは値渡しなので元の値のまま
			ASSERT_EQ(myc_prev, ret0);
		}
	}
}
