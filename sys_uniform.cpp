#include "sys_uniform.hpp"
#include "sys_uniform_value.hpp"
#include "glx_if.hpp"
#include "gl_program.hpp"
#include "drawtoken/make_uniform.hpp"
#include "uniform_ent.hpp"

namespace rev {
	namespace unif {
		const LiteralStr
					Alpha("u_alpha"),
					Color("u_color");
		namespace texture {
			const LiteralStr
					Diffuse("u_texDiffuse");
		}
	}
	namespace sysunif {
		namespace screen {
			const LiteralStr
					Size("sys_vScreenSize");
		}
	}

	// --------------------- SystemUniform ---------------------
	namespace {
		using SetF = std::function<void (const SystemUniform&, UniformEnt&)>;
		const SetF c_systagF[] = {
			[](const SystemUniform& s, UniformEnt& u) {
				auto& ss = s.getScreenSize();
				u.setUniform(sysunif::screen::Size, [&ss](){
					return draw::MakeUniform(
								frea::Vec4(
									ss.width,
									ss.height,
									1.f/ss.width,
									1.f/ss.height
								)
							);
				});
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
	void SystemUniform::outputUniforms(UniformEnt& u) const {
		for(auto& f : c_systagF)
			f(*this, u);
	}
}
