#include "urihandler.hpp"
#include "lubee/error.hpp"

namespace rev {
	auto UriHandlerV::_findHandler(const UriHandler_SP& h) const {
		return std::find_if(_handler.begin(), _handler.end(),
				[&h](const HandlerPair& p){
					return p.second == h;
				});
	}
	void UriHandlerV::addHandler(const Priority prio, const UriHandler_SP& h) {
		Assert0(_findHandler(h) == _handler.end());
		_handler.emplace_back(prio, h);
		std::sort(
			_handler.begin(), _handler.end(),
			[](const HandlerPair& p0, const HandlerPair& p1) {
				return p0.first < p1.first; }
		);
	}
	void UriHandlerV::remHandler(const UriHandler_SP& h) {
		auto itr = _findHandler(h);
		Assert0(itr != _handler.end());
		_handler.erase(itr);
	}
	HRW UriHandlerV::procHandler(const URI& uri, const int access) const {
		// 優先度の高い順に開けるか試していく
		for(auto& p : _handler) {
			if(HRW ret = p.second->openURI(uri, access))
				return ret;
		}
		return HRW();
	}
	void UriHandlerV::clearHandler() {
		_handler.clear();
	}
}
