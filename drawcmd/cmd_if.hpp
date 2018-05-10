#pragma once

namespace rev::draw {
	class IQueue;
	struct ICmd {
		virtual void dcmd_export(IQueue& q, int id, int texId) const = 0;
		virtual ~ICmd() {}
	};
}
