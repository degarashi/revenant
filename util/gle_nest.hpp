#pragma once
#include "../glx.hpp"

namespace rev {
	namespace util {
		template <class... Base>
		class GLE_Nest :
			public GLEffect,
			public Base...
		{
			protected:
				void _prepareUniforms() override {
					const auto dummy = [](auto...){};
					auto& u = refUniformEnt();
					const auto fn = [&u](auto& self){
						self.outputUniforms(u);
					};
					dummy((fn(static_cast<Base&>(*this)), 0)...);
					GLEffect::_prepareUniforms();
				}
				void moveFrom(IEffect& prev) override {
					const auto dummy = [](auto...){};
					const auto fn = [&prev](auto& self){
						using Type = std::decay_t<decltype(self)>;
						self.moveFrom(dynamic_cast<Type&>(prev));
					};
					dummy((fn(static_cast<Base&>(*this)), 0)...);
				}
		};
	}
}
