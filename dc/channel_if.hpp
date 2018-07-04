#pragma once

namespace rev::dc {
	struct IChannel {
		HSeekFrame		_seek;
		virtual ~IChannel () {}
		float length() const;
	};
	template <class Query>
	struct IChannelT : IChannel {
		virtual void apply(const Query& q, float t) const = 0;
	};
}
