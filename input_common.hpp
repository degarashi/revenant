#pragma once
#include "spine/resmgr.hpp"
#include "frea/vector.hpp"
#include "spine/singleton.hpp"
#include "handle.hpp"
#include "spine/enum.hpp"

namespace rev {
	extern const int InputRange,
					InputRangeHalf;
	struct TPos2D : Resource {
		bool		bTouch;
		// 画面に対する0〜1の比率で記録する
		frea::Vec2	absPos,
					relPos;
		float		pressure;

		const static struct tagClean {} Clean;
		TPos2D() = default;
		TPos2D(tagClean) noexcept;
		void setNewAbs(const frea::Vec2& p) noexcept;
		void setNewRel(const frea::Vec2& p) noexcept;
		const char* getResourceName() const noexcept override;
	};
	#define mgr_pointer (::rev::PointerMgr::ref())
	class PointerMgr : public spi::ResMgr<TPos2D>, public spi::Singleton<PointerMgr> {};

	struct RecvPointer {
		virtual void newPointer(WPtr wptr) = 0;
		virtual ~RecvPointer() {}
	};
	using RecvPointer_SP = std::shared_ptr<RecvPointer>;

	struct RecvPtrGroup : RecvPointer {
		using LSet = std::unordered_set<RecvPointer_SP>;
		LSet	listener;

		void newPointer(WPtr wptr) override;
	};

	DefineEnum(InputType,
		(Keyboard)
		(Mouse)
		(Joypad)
		(Touchpad)
	);
	DefineEnum(MouseMode,
		(Absolute)
		(Clipping)
		(Relative)
	);
	DefineEnum(VKey,
		(A)(B)(C)(D)(E)(F)(G)(H)(I)(J)(K)(L)(M)(N)(O)(P)(Q)(R)(S)(T)(U)(V)(W)(X)(Y)(Z)
		(_1)(_2)(_3)(_4)(_5)(_6)(_7)(_8)(_9)(_0)
		(Return)(Escape)(Backspace)(Tab)(Space)
		(Minus)(Equals)(Leftbracket)(Rightbracket)(Backslash)(Nonushash)(Semicolon)(Apostrophe)(Grave)(Comma)(Period)(Slash)
		(Capslock)
		(F1)(F2)(F3)(F4)(F5)(F6)(F7)(F8)(F9)(F10)(F11)(F12)
		(Printscreen)(Scrolllock)(Pause)(Insert)
		(Home)(Pageup)(Delete)(End)(Pagedown)(Right)(Left)(Down)(Up)
		(Numlockclear)
		(Kp_divide)(Kp_multiply)(Kp_minus)(Kp_plus)(Kp_enter)(Kp_1)(Kp_2)(Kp_3)(Kp_4)(Kp_5)(Kp_6)(Kp_7)(Kp_8)(Kp_9)(Kp_0)(Kp_period)
		(Nonusbackslash)(Application)(Power)(Kp_equals)(F13)(F14)(F15)(F16)(F17)(F18)(F19)(F20)(F21)(F22)(F23)(F24)(Execute)(Help)(Menu)(Select)(Stop)(Again)(Undo)(Cut)(Copy)(Paste)(Find)(Mute)(Volumeup)(Volumedown)
		(Kp_comma)(Kp_equalsas400)
		(International1)(International2)(International3)(International4)(International5)(International6)(International7)(International8)(International9)
		(Lang1)(Lang2)(Lang3)(Lang4)(Lang5)(Lang6)(Lang7)(Lang8)(Lang9)
		(Alterase)(Sysreq)(Cancel)(Clear)(Prior)(Return2)(Separator)(Out)(Oper)(Clearagain)(Crsel)(Exsel)
		(Kp_00)(Kp_000)(Thousandsseparator)(Decimalseparator)(Currencyunit)(Currencysubunit)(Kp_leftparen)(Kp_rightparen)(Kp_leftbrace)(Kp_rightbrace)(Kp_tab)(Kp_backspace)(Kp_a)(Kp_b)(Kp_c)(Kp_d)(Kp_e)(Kp_f)(Kp_xor)(Kp_power)(Kp_percent)(Kp_less)(Kp_greater)(Kp_ampersand)(Kp_dblampersand)(Kp_verticalbar)(Kp_dblverticalbar)(Kp_colon)(Kp_hash)(Kp_space)(Kp_at)(Kp_exclam)(Kp_memstore)(Kp_memrecall)(Kp_memclear)(Kp_memadd)(Kp_memsubtract)(Kp_memmultiply)(Kp_memdivide)(Kp_plusminus)(Kp_clear)(Kp_clearentry)(Kp_binary)(Kp_octal)(Kp_decimal)(Kp_hexadecimal)
		(Lctrl)(Lshift)(Lalt)(Lgui)(Rctrl)(Rshift)(Ralt)(Rgui)
		(Mode)
		(Audionext)(Audioprev)(Audiostop)(Audioplay)(Audiomute)(Mediaselect)(Www)(Mail)(Calculator)(Computer)(Ac_search)(Ac_home)(Ac_back)(Ac_forward)(Ac_stop)(Ac_refresh)(Ac_bookmarks)
		(Brightnessdown)(Brightnessup)(Displayswitch)
		(Kbdillumtoggle)(Kbdillumdown)(Kbdillumup)(Eject)(Sleep)
		(App1)(App2)
	);

	struct KeyLog {
		int		scancode,
				ch;
		bool	down;

		bool operator == (const KeyLog& k) const noexcept {
			return scancode == k.scancode &&
					ch == k.ch &&
					down == k.down;
		}
		bool operator != (const KeyLog& k) const noexcept {
			return !(this->operator == (k));
		}
	};
	struct KeyAux {
		bool	shift,
				ctrl,
				alt,
				super;
	};
}
