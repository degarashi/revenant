#pragma once
#include "../handle/model.hpp"
#include "channel_if.hpp"
#include "spine/optional.hpp"
#include <vector>

namespace rev::dc {
	class Animation {
		private:
			using Ch = std::vector<HChannel>;
			using Float_OP = spi::Optional<float>;
			Ch					_channel;
			float				_time;
			mutable Float_OP	_length;
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
