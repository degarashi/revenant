#pragma once

namespace rev {
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
