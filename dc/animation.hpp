#pragma once
#include "../handle/model.hpp"
#include "channel_if.hpp"
#include <vector>

namespace rev::dc {
	class Animation {
		private:
			using Ch = std::vector<HChannel>;
			Ch		_channel;
			float	_time;
		public:
			Animation();
			void addChannel(const HChannel& c);
			float length() const;

			void setTime(float t);
			float time() const noexcept;
			void reset();

			bool update(const IJointQuery& q, float dt);
	};
}
