#include "lcvtest.hpp"
#include "../luaimpl.hpp"

namespace rev {
	class MyResMgr;
	namespace test {
		class MyClass : public Resource {
			public:
				static int s_called;
				using mgr_t = MyResMgr;
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
				MyClass(LuaTest& t):
					MyClass(
						t.genValue<T0>(),
						t.genValue<T1>(),
						t.genValue<T2>()
					)
				{}
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
			using Mgr_t = typename value_t::mgr_t;
			static std::unique_ptr<Mgr_t>	s_mgr;

			static void SetUpTestCase() {
				LCV_TestRW<T>::SetUpTestCase();
				s_mgr = std::make_unique<Mgr_t>();
			}
			static void TearDownTestCase() {
				s_mgr.reset();
				LCV_TestRW<T>::TearDownTestCase();
			}
			void SetUp() override {
				value_t::s_called = 0;
				this->loadSharedPtrModule();
			}
			void registerClass() {
				LuaImport::RegisterClass<value_t>(*this->_lsp);
			}
		};
		template <class T>
		std::unique_ptr<typename LCV_ClassTest<T>::Mgr_t> LCV_ClassTest<T>::s_mgr;

		using TypesC = ::testing::Types<
			MyClass
		>;

		TYPED_TEST_CASE(LCV_ClassTest, TypesC);
		// Luaからのオブジェクト作成 = New(...)テスト
		TYPED_TEST(LCV_ClassTest, New) {
			USING(value_t);
			this->registerClass();
			auto& lsp = this->_lsp;

			const value_t myc(*this);
			const std::string code =
				std::string("return ") + myc.getResourceName() + ".New("
				+ std::to_string(myc.value) + ", " + std::to_string(myc.value1) + ", " + std::to_string(myc.value2)
				+ ")";
			HRW hRW = mgr_rw.fromConstTemporal(code.c_str(), code.length());
			lsp->loadFromSource(hRW);

			ASSERT_EQ(LCV<value_t>()(myc), lsp->type(-1));
			const value_t myc2 = LCV<value_t>()(-1, lsp->getLS());
			ASSERT_EQ(myc, myc2);
		}
		TYPED_TEST(LCV_ClassTest, Member) {
			USING(value_t);
			USING(T0);
			USING(T1);
			this->registerClass();
			auto& lsp = this->_lsp;

			const value_t myc(*this);
			LCV<value_t>()(lsp->getLS(), myc);
			// メンバ変数 "value" の読み込みテスト
			lsp->getField(-1, "value");
			ASSERT_EQ(LCV<T0>()(myc.value), lsp->type(-1));
			ASSERT_EQ(LCValue(typename LCV<T0>::value_t(myc.value)), lsp->toLCValue(-1));
			lsp->pop(1);

			// メンバ変数 "value1" の読み込みテスト
			lsp->getField(-1, "value1");
			ASSERT_EQ(LCV<T1>()(myc.value1), lsp->type(-1));
			ASSERT_EQ(LCValue(typename LCV<T1>::value_t(myc.value1)), lsp->toLCValue(-1));
			lsp->pop(1);

			// value2は登録してないのでnilが返る
			lsp->getField(-1, "value2");
			ASSERT_EQ(LuaType::Nil, lsp->type(-1));
			lsp->pop(1);

			// メンバ変数の書き込みテスト
			const value_t myc2(*this);
			lsp->setField(-1, "value", myc2.value);
			lsp->setField(-1, "value1", myc2.value1);

			const value_t myc3 = LCV<value_t>()(-1, lsp->getLS());
			ASSERT_EQ(myc3.value, myc2.value);
			ASSERT_EQ(myc3.value1, myc2.value1);
		}
		TYPED_TEST(LCV_ClassTest, PushPop) {
			USING(value_t);
			auto& lsp = this->_lsp;
			// クラスの登録と、LuaExport関数が呼ばれたかの確認
			ASSERT_EQ(0, value_t::s_called);
			this->registerClass();
			ASSERT_EQ(1, value_t::s_called);

			value_t myc(*this);
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
			const value_t myc_prev(myc);
			myc = value_t(*this);
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
			this->registerClass();
			auto& lsp = this->_lsp;

			const value_t myc(*this);
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
			ASSERT_EQ(T0(myc.value * r), myc2.value);
			ASSERT_EQ(T1(myc.value1 * r), myc2.value1);
			ASSERT_EQ(T2(myc.value2 * r), myc2.value2);
		}
		TYPED_TEST(LCV_ClassTest, StaticFunction) {
			USING(value_t);
			USING(T0);
			this->registerClass();
			auto& lsp = this->_lsp;

			const value_t myc(*this);
			LCV<value_t>()(lsp->getLS(), myc);

			// Luaを通してstatic functionを呼ぶ
			lsp->getField(-1, "sFunc");
			const auto r = this->template genValue<T0>();
			lsp->push(r);
			lsp->call(1,1);
			// 引数と同じ値が返って来ている筈
			ASSERT_EQ(LCV<T0>()(r), lsp->type(-1));
			// クラス内部の変数は変化なし
			const auto* ret = LCV<value_t*>()(1, lsp->getLS());
			ASSERT_EQ(myc, *ret);
		}
	}
}
