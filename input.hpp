#pragma once
#include "spine/resmgr_named.hpp"
#include "input_dep_sdl.hpp"
#include <unordered_set>

namespace rev {
	//! 入力共通インタフェース
	struct Input : Resource {
		virtual ~Input() {}

		virtual InputType getType() const = 0;
		// エラー時はfalseを返す
		virtual bool scan() = 0;
		virtual const std::string& name() const = 0;
		// ---- relative input querying ----
		virtual int getButton(int /*num*/) const { return 0;}
		virtual int getAxis(int /*num*/) const { return 0; }
		virtual int getHat(int /*num*/) const { return -1; }
		virtual int numButtons() const { return 0; }
		virtual int numAxes() const { return 0; }
		virtual int numHats() const { return 0; }
		virtual void setDeadZone(int /*num*/, float /*r*/, float /*dz*/) {}

		virtual void setMouseMode(MouseMode /*mode*/) {}
		virtual MouseMode getMouseMode() const { return MouseMode::Absolute; }
		// ---- absolute input querying ----
		virtual void addListener(const RecvPointer_SP&) {}
		virtual void remListener(const RecvPointer_SP&) {}
		// 1つ以上の座標が検出されたら何れかを返す
		virtual WPtr getPointer() const { return WPtr(); }
		const char* getResourceName() const noexcept {
			return ::rev::resource_name::Input;
		}
	};

	//! アナログ入力値のデッドゾーンと倍率調整
	struct DZone {
		float	ratio,
				deadzone;

		DZone(const DZone& dz) = default;
		DZone(float r=1.f, float dz=0) noexcept;
		int filter(int val) const noexcept;
	};
	using DZoneL = std::vector<DZone>;
	enum VKey {
		VKEY_0, VKEY_1, // ...
		NUM_VKEY
	};
	// Class <-> Dep の間は数値を正規化 (-InputRange +InputRange)
	/*! Depend要件:
		protected:
			(キーボードに関しては最大キー数は関係なく、キーコードで問い合わせる)
			int dep_getButton(int) const;	<= 引数はキーコード
			bool dep_scan();
			template <class> static HInput OpenKeyboard();
		public:
			static int NumKeyboards(); */
	class Keyboard : public KeyboardDep, public Input {
		private:
			const static std::string		s_name;
		public:
			using KeyboardDep::KeyboardDep;
			InputType getType() const override;
			int getButton(int num) const override;
			int numButtons() const override;
			bool scan() override;
			const std::string& name() const override;

			static HInput OpenKeyboard();
	};

	/*! Depend要件:
		protected:
			template <T> static T* OpenMouse(int num);
			int dep_numButtons() const;
			int dep_numAxes() const;
			int dep_getButton(int) const;
			int dep_getAxis(int) const;
			bool dep_scan(TPos2D&);
		public:
			static void Update();
			static int NumMouse();
			static std::string GetMouseName(int);
			void setWindowClip(bool); */
	class Mouse : public MouseDep, public Input {
		DZoneL		_axisDZ = DZoneL(MouseDep::dep_numAxes());
		HPtr		_hPtr = mgr_pointer.emplace(TPos2D::Clean);
		const static std::string		s_name;

		public:
			using MouseDep::MouseDep;
			InputType getType() const override;
			bool scan() override;
			const std::string& name() const override;
			int getButton(int num) const override;
			int getAxis(int num) const override;
			int numButtons() const override;
			int numAxes() const override;
			void setMouseMode(MouseMode mode) override;
			MouseMode getMouseMode() const override;
			WPtr getPointer() const override;
			void setDeadZone(int num, float r, float dz) override;

			static HInput OpenMouse(int num);
	};

	/*! Depend要件:
		protected:
			static void Update();
			template <class> static HInput OpenJoypad(int);
			int dep_numButtons() const;
			int dep_numAxes() const;
			int dep_numHats() const;
			int dep_getButton(int) const;
			int dep_getAxis(int) const;
			int dep_getHat(int) const;
			bool dep_scan();
		public:
			static int NumJoypad();
			static std::string GetJoypadName(int);
			const std::string& name() const; */
	class Joypad : public JoypadDep, public Input {
		DZoneL		_axisDZ = DZoneL(JoypadDep::dep_numAxes());

		public:
			using JoypadDep::JoypadDep;

			InputType getType() const override;
			bool scan() override;
			int getButton(int num) const override;
			int getAxis(int num) const override;
			int getHat(int num) const override;
			int numButtons() const override;
			int numAxes() const override;
			int numHats() const override;

			static HInput OpenJoypad(int num);
			const std::string& name() const override;
	};
	// タッチパネル関係はイベントでしか取得できない？
	/*! Depend要件:
		protected:
			template <class> static HInput OpenTouchpad(int);
			bool dep_scan(RecvPointer*);
		public:
			static void Update();
			static int NumTouchpad();
			static std::string GetTouchpadName(int);
			const std::string& name() const; */
	class Touchpad : public TouchDep, public Input {
		private:
			RecvPtrGroup	_group;
		public:
			using TouchDep::TouchDep;
			InputType getType() const override;
			bool scan() override;
			WPtr getPointer() const override;

			void addListener(const RecvPointer_SP& r) override;
			void remListener(const RecvPointer_SP& r) override;

			static HInput OpenTouchpad(int num);
			const std::string& name() const override;
	};

	DefineEnum(InputFlag,
		(Button)
		(ButtonFlip)
		(Axis)
		(AxisNegative)
		(AxisPositive)
		(Hat)
		(HatX)
		(HatY)
	);
	class Action : public Resource {
		public:
			// 値の取得
			using FGet = int (Input::*)(int) const;
			// 値の改変
			using FManip = int (*)(int);
			struct Funcs {
				FGet	getter;
				FManip	manipulator;
			};
		private:
			const static Funcs cs_funcs[InputFlag::_Num];

			//! 入力機器からの値取得方法を定義
			struct Link {
				HInput	hInput;
				InputFlag::e	inF;
				int		num;

				bool operator == (const Link& l) const;
				int getValue() const;
			};
			using LinkV = std::vector<Link>;
			LinkV		_link;

			// ボタンを押している間は
			// Pressed=1, Neutral=0, Released=-1, Pressing= n>=1
			int			_state,
						_value;
			//! getKeyValueSimplifiedOnce()にて一度入力があった時にtrue, 無入力の時にfalseとされる
			mutable bool	_bOnce;
			bool _advanceState(int val);
		public:
			Action();
			Action(Action&& a) = default;
			void update();
			bool isKeyPressed() const;
			bool isKeyReleased() const;
			bool isKeyPressing() const;
			void addLink(HInput hI, InputFlag::e inF, int num);
			void remLink(HInput hI, InputFlag::e inF, int num);
			int getState() const;
			int getValue() const;
			//! getValueの結果を使いやすいように加工(-1〜1)して返す
			/*! \retval 1	getValueの値がInputRangeHalf以上
						-1	getValueの値が-InputRangeHalf以下
						0	どちらでもない時 */
			int getKeyValueSimplified() const;
			//! getKeyValueSimplified()をキーの押しっぱなしに対応
			//! 一度値を0に戻してからでないと反応しない
			/*!
				\return getKeyValueSimplified()と同様
				@sa getKeyValueSimplified()
			*/
			int getKeyValueSimplifiedOnce() const;
			void linkButtonAsAxis(HInput hI, int num_negative, int num_positive);
			const char* getResourceName() const noexcept override;
	};
	class ActMgr : public spi::ResMgrName<Action>, public spi::Singleton<ActMgr> {};

	#define mgr_input (::rev::InputMgr::ref())
	//! Actionとキーの割付とステート管理用
	/*! ボタンステートはAction毎に用意 */
	class InputMgr : public spi::ResMgr<Input>, public spi::Singleton<InputMgr> {
		private:
			using ActSet = std::unordered_set<HAct>;
			ActMgr		_act;
			ActSet		_aset;
			std::string	_text;
			template <class CHK>
			bool _checkKeyValue(CHK chk, HAct hAct) const;
			template <std::size_t... N, class Tuple>
			static auto _GetKeyValueSimplifiedMulti(std::index_sequence<N...>, const Tuple& t) {
				return std::make_tuple((std::get<N>(t)->getKeyValueSimplified())...);
			}
			template <std::size_t... N>
			static void _LinkButtonAsAxisMulti(std::index_sequence<N...>, HInput) {}
			template <std::size_t... N, class T, class... Tuple>
			static void _LinkButtonAsAxisMulti(std::index_sequence<N...> seq, HInput hI, const T& t, const Tuple&... ts) {
				LinkButtonAsAxis(hI, std::get<N>(t)...);
				_LinkButtonAsAxisMulti(seq, hI, ts...);
			}
		public:
			InputMgr();
			~InputMgr();

			HAct makeAction(const std::string& name);
			void addAction(const HAct& hAct);
			// 更新リストから除くだけで削除はしない
			void remAction(const HAct& hAct);
			static void LinkButtonAsAxis(HInput hI, HAct hAct, int num_negative, int num_positive);
			template <class T, class... Tuple>
			static void LinkButtonAsAxisMulti(HInput hI, const T& t, const Tuple&... ts) {
				using IntS = std::make_index_sequence<std::tuple_size<T>::value>;
				_LinkButtonAsAxisMulti(IntS(), hI, t, ts...);
			}
			//! 複数のハンドルを受け取りsimplifiedの結果をstd::tupleで返す
			template <class... H>
			static auto GetKeyValueSimplifiedMulti(H... h) {
				using IntS = std::make_index_sequence<sizeof...(H)>;
				return _GetKeyValueSimplifiedMulti(IntS(), std::forward_as_tuple(h...));
			}
			// 次回の更新時に処理
			void update();
			// 文字入力処理用
			const std::string& getTextInput() const noexcept;
			const KeyLogV& getKeyLog() const noexcept;
			const KeyAux& getKeyAux() const noexcept;
	};
	template <class T>
	struct InputMaker {
		template <class... Ts>
		auto operator ()(Ts&&... ts) const {
			return mgr_input.emplace<T>(std::forward<Ts>(ts)...);
		}
	};
}
#include "luaimport.hpp"
DEF_LUAIMPORT(rev::Input)
DEF_LUAIMPORT(rev::Action)
DEF_LUAIMPORT(rev::Keyboard)
DEF_LUAIMPORT(rev::Mouse)
DEF_LUAIMPORT(rev::Joypad)
DEF_LUAIMPORT(rev::TPos2D)
DEF_LUAIMPORT(rev::InputMgr)
