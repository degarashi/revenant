#include "sys_uniform.hpp"
#include "sys_uniform_value.hpp"
#include "glx_if.hpp"
#include "gl_program.hpp"
#include "drawtoken/make_uniform.hpp"

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
		using SetF = std::function<void (const SystemUniform&, UniformIdMap_t&, const GLProgram&)>;
		const SetF c_systagF[] = {
			[](const SystemUniform& s, UniformIdMap_t& u, const GLProgram& p) {
				auto& ss = s.getScreenSize();
				if(const auto idv = p.getUniformId(sysunif::screen::Size)) {
					u[*idv] = draw::MakeUniform(
						frea::Vec4(
							ss.width,
							ss.height,
							1.f/ss.width,
							1.f/ss.height
						)
					);
				}
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
	void SystemUniform::outputUniforms(UniformIdMap_t& u, const GLProgram& p) const {
		for(auto& f : c_systagF)
			f(*this, u, p);
	}
}
