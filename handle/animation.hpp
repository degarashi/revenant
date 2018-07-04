#pragma once
#include "common.hpp"

namespace beat::g3 {
	class Pose;
}
namespace rev {
	namespace dc {
		struct IJointQuery;
		template <class Query>
		struct IChannelT;

		struct IJointAt;
		template <class Dst>
		struct IFrameOut;
		struct ISeekFrame;
	}
	DEF_HANDLE(dc::IChannelT<dc::IJointQuery>, JChannel)
	DEF_HANDLE(dc::IJointAt, Jat)
	DEF_HANDLE(dc::IFrameOut<beat::g3::Pose>, PoseFrame)
	DEF_HANDLE(dc::ISeekFrame, SeekFrame)
}
