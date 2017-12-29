#include "sys_uniform.hpp"
#include "sys_uniform_value.hpp"
#include "glx_if.hpp"

namespace rev {
	namespace unif {
		const Name	Alpha("u_alpha"),
					Color("u_color");
		namespace texture {
			const Name	Diffuse("u_texDiffuse");
		}
	}
	namespace sysunif {
		namespace screen {
			const Name	Size("sys_vScreenSize");
		}
	}

	// --------------------- SystemUniform ---------------------
	namespace {
		using SetF = std::function<void (const SystemUniform&, IEffect&)>;
		const SetF c_systagF[] = {
			[](const SystemUniform& s, IEffect& e) {
				auto& ss = s.getScreenSize();
				e.setUniform_if(
					sysunif::screen::Size,
					frea::Vec4(
						ss.width,
						ss.height,
						1.f/ss.width,
						1.f/ss.height
					)
				);
			}
		};
	}
	void SystemUniform::moveFrom(SystemUniform& prev) {
		_screenSize = prev._screenSize;
	}
	const lubee::SizeI& SystemUniform::getScreenSize() const {
		return _screenSize;
	}
	void SystemUniform::setScreenSize(const lubee::SizeI& s) {
		_screenSize = s;
	}
	void SystemUniform::outputUniforms(IEffect& e) const {
		for(auto& f : c_systagF)
			f(*this, e);
	}
}
