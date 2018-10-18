#pragma once

namespace rev {
	template <class... Der>
	struct OVR_Functor : Der... {
		using Der::operator()...;
	};
	template <class... Der>
	OVR_Functor(Der...) -> OVR_Functor<Der...>;
}
