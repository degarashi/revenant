#pragma once
#include "../handle/animation.hpp"
#include "channel_if.hpp"
#include "spine/optional.hpp"

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
			void append(const Animation& a);
			void clear();
			bool empty() const noexcept;
			float length() const;

			void setTime(float t);
			float time() const noexcept;
			void loop();

			bool update(const IJointQuery& q, float dt);
	};
}
