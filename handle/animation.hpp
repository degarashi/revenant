#pragma once
#include "common.hpp"

namespace rev {
	namespace dc {
		struct IChannel;
		struct IJointAt;
		struct IPoseFrame;
		struct ISeekFrame;
	}
	DEF_HANDLE(dc::IChannel, Channel)
	DEF_HANDLE(dc::IJointAt, Jat)
	DEF_HANDLE(dc::IPoseFrame, PoseFrame)
	DEF_HANDLE(dc::ISeekFrame, SeekFrame)
}
