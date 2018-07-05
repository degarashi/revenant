#pragma once
#include "../handle/animation.hpp"
#include "channel.hpp"
#include "spine/optional.hpp"

namespace rev::dc {
	class Animation {
		private:
			using ChV = std::vector<HChannel>;
			using Float_OP = spi::Optional<float>;
			ChV					_channel;
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

			void update(const IJointQuery& q);
			bool advance(float dt);
	};
}
