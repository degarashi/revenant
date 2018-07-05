#pragma once
#include "common.hpp"

namespace rev {
	namespace dc {
		struct Channel;
		struct IJointAt;
		struct IFrameOut;
		struct ISeekFrame;
	}
	DEF_HANDLE(dc::Channel, Channel)
	DEF_HANDLE(dc::IJointAt, Jat)
	DEF_HANDLE(dc::IFrameOut, FrameOut)
	DEF_HANDLE(dc::ISeekFrame, SeekFrame)
}
