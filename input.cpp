#include "input.hpp"
#include "lubee/compare.hpp"
#include "frea/constant.hpp"

namespace rev {
	// ------------------ DZone ------------------
	DZone::DZone(const float r, const float dz) noexcept:
		ratio(r),
		deadzone(dz)
	{}
	int DZone::filter(int val) const noexcept {
		float fval = (val * ratio) / InputRange;
		if(val > 0)
			fval = std::max(0.f, fval-deadzone*ratio);
		else
			fval = std::min(0.f, fval+deadzone*ratio);
		return lubee::Saturate(static_cast<int>(fval * InputRange), InputRange);
	}

	void RecvPtrGroup::newPointer(WPtr wptr) {
		for(auto& p : listener)
			p->newPointer(wptr);
	}

	const int InputRange = 1024,
				InputRangeHalf = InputRange/2;
	const TPos2D::tagClean TPos2D::Clean;
	// ----------------- TPos2D -----------------
	TPos2D::TPos2D(tagClean) noexcept:
		bTouch(false),
		absPos{0},
		relPos{0},
		pressure(0)
	{}
	void TPos2D::setNewAbs(const frea::Vec2& p) noexcept {
		relPos.x = p.x - absPos.x;
		relPos.y = p.y - absPos.y;
		absPos = p;
	}
	void TPos2D::setNewRel(const frea::Vec2& p) noexcept {
		absPos += p;
		relPos = p;
	}
	const char* TPos2D::getResourceName() const noexcept {
		return ::rev::resource_name::TPos2D;
	}
	// ----------------- Keyboard -----------------
	InputType Keyboard::getType() const {
		return InputType::Keyboard;
	}
	int Keyboard::getButton(const int num) const {
		return dep_getButton(num);
	}
	int Keyboard::numButtons() const {
		return VKey::_Num;
	}
	HInput Keyboard::OpenKeyboard() {
		return KeyboardDep::OpenKeyboard<InputMaker<Keyboard>>();
	}
	bool Keyboard::scan() {
		return dep_scan();
	}
	const std::string& Keyboard::name() const {
		return KeyboardDep::name();
	}

	// ----------------- Mouse -----------------
	InputType Mouse::getType() const {
		return InputType::Mouse;
	}
	int Mouse::getButton(const int num) const {
		return dep_getButton(num);
	}
	int Mouse::getAxis(const int num) const {
		if(num < 2) {
			auto& p = *_hPtr;
			return _axisDZ[num].filter((num == 0) ? p.relPos.x : p.relPos.y);
		}
		return _axisDZ[num].filter(dep_getAxis(num));
	}
	int Mouse::numButtons() const {
		return dep_numButtons();
	}
	int Mouse::numAxes() const {
		return dep_numAxes();
	}
	bool Mouse::scan() {
		return dep_scan(*_hPtr);
	}
	HInput Mouse::OpenMouse(const int num) {
		return MouseDep::OpenMouse<InputMaker<Mouse>>(num);
	}
	const std::string& Mouse::name() const {
		return MouseDep::name();
	}
	void Mouse::setMouseMode(MouseMode mode) {
		dep_setMode(mode, *_hPtr);
	}
	MouseMode Mouse::getMouseMode() const {
		return dep_getMode();
	}
	WPtr Mouse::getPointer() const {
		// 常にマウスの座標を返す
		return WPtr(_hPtr);
	}
	void Mouse::setDeadZone(const int num, const float r, const float dz) {
		auto& d = _axisDZ[num];
		d.ratio = r;
		d.deadzone = dz;
	}

	// ----------------- Joypad -----------------
	InputType Joypad::getType() const {
		return InputType::Joypad;
	}
	int Joypad::getButton(const int num) const {
		return dep_getButton(num);
	}
	int Joypad::getAxis(const int num) const {
		return _axisDZ[num].filter(dep_getAxis(num));
	}
	int Joypad::getHat(const int num) const {
		return dep_getHat(num);
	}
	int Joypad::numButtons() const {
		return dep_numButtons();
	}
	int Joypad::numAxes() const {
		return dep_numAxes();
	}
	int Joypad::numHats() const {
		return dep_numHats();
	}
	bool Joypad::scan() {
		return dep_scan();
	}
	HInput Joypad::OpenJoypad(const int num) {
		return JoypadDep::OpenJoypad<InputMaker<Joypad>>(num);
	}
	const std::string& Joypad::name() const {
		return JoypadDep::name();
	}

	// ----------------- Touchpad -----------------
	InputType Touchpad::getType() const {
		return InputType::Touchpad;
	}
	bool Touchpad::scan() {
		return dep_scan(&_group);
	}
	void Touchpad::addListener(const RecvPointer_SP& r) {
		_group.listener.insert(r);
	}
	void Touchpad::remListener(const RecvPointer_SP& r) {
		_group.listener.erase(r);
	}
	HInput Touchpad::OpenTouchpad(const int num) {
		return TouchDep::OpenTouchpad<InputMaker<Touchpad>>(num);
	}
	const std::string& Touchpad::name() const {
		return TouchDep::name();
	}
	WPtr Touchpad::getPointer() const {
		return dep_getPointer();
	}

	// ----------------- Action::Funcs -----------------
	namespace {
		int FM_Direct(int val) {
			return val;
		}
		int FM_Flip(int val) {
			return -val;
		}
		int FM_Positive(int val) {
			return std::max(0, val);
		}
		int FM_Negative(int val) {
			return std::max(0, -val);
		}
		template <class F>
		int16_t HatAngToValue(int val, F f) {
			if(val == -1)
				return int16_t(0);
			auto ang = static_cast<float>(val) / static_cast<float>(InputRange);
			ang *= 2 * frea::Pi<float>;
			return static_cast<int16_t>(f(ang) * InputRange);
		}
		int FM_AngToX(int val) {
			using SF = float (*)(float);
			return HatAngToValue<SF>(val, std::sin);
		}
		int FM_AngToY(int val) {
			using SF = float (*)(float);
			return HatAngToValue<SF>(val, std::cos);
		}
	}
	const Action::Funcs Action::cs_funcs[InputFlag::_Num] = {
		{&Input::getButton, FM_Direct},
		{&Input::getButton, FM_Flip},
		{&Input::getAxis, FM_Direct},
		{&Input::getAxis, FM_Positive},
		{&Input::getAxis, FM_Negative},
		{&Input::getHat, FM_Direct},
		{&Input::getHat, FM_AngToX},
		{&Input::getHat, FM_AngToY}
	};

	// ----------------- Action::Link -----------------
	int Action::Link::getValue() const {
		const auto f = cs_funcs[inF];
		Input* iip = hInput.get();
		const int val = (iip->*f.getter)(num);
		return f.manipulator(val);
	}
	bool Action::Link::operator == (const Link& l) const {
		return hInput == l.hInput &&
				num == l.num &&
				inF == l.inF;
	}
	// ----------------- Action -----------------
	Action::Action():
		_state(0),
		_value(0),
		_bOnce(false)
	{}
	void Action::update() {
		int sum = 0;
		// linkの値を加算合成
		for(auto& l : _link)
			sum += l.getValue();
		sum = lubee::Saturate(sum, InputRange);
		if(!_advanceState(sum))
			_bOnce = false;
	}
	bool Action::_advanceState(const int val) {
		if(val >= InputRangeHalf) {
			if(_state <= 0)
				_state = 1;
			else {
				// オーバーフロー対策
				_state = std::max(_state+1, _state);
			}
		} else {
			if(_state > 0)
				_state = 0;
			else {
				// アンダーフロー対策
				_state = std::min(_state-1, _state);
			}
		}
		_value = val;
		return !lubee::IsInRange(_value, -InputRangeHalf, InputRangeHalf);
	}
	bool Action::isKeyPressed() const {
		return getState() == 1;
	}
	bool Action::isKeyReleased() const {
		return getState() == 0;
	}
	bool Action::isKeyPressing() const {
		return getState() > 0;
	}
	void Action::addLink(const HInput hI, const InputFlag::e inF, const int num) {
		const Link link{hI, inF, num};
		const auto itr = std::find(_link.begin(), _link.end(), link);
		if(itr == _link.end())
			_link.emplace_back(link);
	}
	void Action::remLink(const HInput hI, const InputFlag::e inF, const int num) {
		const Link link{hI, inF, num};
		const auto itr = std::find(_link.begin(), _link.end(), link);
		if(itr != _link.end())
			_link.erase(itr);
	}
	int Action::getState() const {
		return _state;
	}
	int Action::getValue() const {
		return _value;
	}
	int Action::getKeyValueSimplified() const {
		const auto v = getValue();
		if(v >= InputRangeHalf)
			return 1;
		if(v <= -InputRangeHalf)
			return -1;
		return 0;
	}
	int Action::getKeyValueSimplifiedOnce() const {
		if(!_bOnce) {
			const auto ret = getKeyValueSimplified();
			_bOnce = (ret != 0);
			return ret;
		}
		return 0;
	}
	void Action::linkButtonAsAxis(HInput hI, const int num_negative, const int num_positive) {
		addLink(hI, InputFlag::ButtonFlip, num_negative);
		addLink(hI, InputFlag::Button, num_positive);
	}
	const char* Action::getResourceName() const noexcept {
		return ::rev::resource_name::Action;
	}

	// ----------------- InputMgr -----------------
	HAct InputMgr::makeAction(const std::string& name) {
		HAct ret = _act.emplace(name).first;
		_aset.insert(ret);
		return ret;
	}
	void InputMgr::LinkButtonAsAxis(HInput hI, HAct hAct, const int num_negative, const int num_positive) {
		hAct->addLink(hI, InputFlag::ButtonFlip, num_negative);
		hAct->addLink(hI, InputFlag::Button, num_positive);
	}
	void InputMgr::addAction(const HAct& hAct) {
		_aset.insert(hAct);
	}
	void InputMgr::remAction(const HAct& hAct) {
		_aset.erase(hAct);
	}
	namespace {
		// Terminateメソッドを持っていたら呼び出す
		#define DEF_CALLER(func) \
			struct Call##func { \
				template <class T> \
				void operator()(...) const {} \
				template <class T> \
				void operator()(decltype(&T::func)=nullptr) const { \
					T::func(); \
				} \
		};

		DEF_CALLER(Initialize)
		DEF_CALLER(Terminate)
		DEF_CALLER(Update)
		#undef DEF_CALLER

		template <class Caller>
		void _CallFunction(Caller) {}
		template <class Caller, class T, class... Ts>
		void _CallFunction(Caller c, T*, Ts*...) {
			c.template operator()<T>(nullptr);
			_CallFunction(c, (Ts*)(nullptr)...);
		}
		template <class... Ts, class Caller>
		void CallFunction(Caller c) {
			_CallFunction(c, (Ts*)(nullptr)...);
		}
	}
	void InputMgr::update() {
		CallFunction<Keyboard, Mouse, Joypad, Touchpad, TextInputDep, KeyLogDep>(CallUpdate());
		for(auto h : *this)
			h->scan();
		for(auto& h : _aset) {
			h->update();
		}
		_text = TextInputDep::GetText();
	}
	const std::string& InputMgr::getTextInput() const noexcept {
		return _text;
	}
	const KeyLogV& InputMgr::getKeyLog() const noexcept {
		return KeyLogDep::GetLog();
	}
	const KeyAux& InputMgr::getKeyAux() const noexcept {
		return KeyLogDep::GetAux();
	}
	InputMgr::InputMgr() {
		CallFunction<Keyboard, Mouse, Joypad, Touchpad>(CallInitialize());
	}
	InputMgr::~InputMgr() {
		CallFunction<Keyboard, Mouse, Joypad, Touchpad>(CallTerminate());
	}
}
