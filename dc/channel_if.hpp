#pragma once

namespace rev::dc {
	struct IJointQuery;
	struct IChannel {
		virtual ~IChannel () {}
		virtual float length() const = 0;
		virtual void apply(const IJointQuery& q, float t) const = 0;
	};
}
