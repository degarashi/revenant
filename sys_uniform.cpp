#include "sys_uniform.hpp"
#include "sys_uniform_value.hpp"
#include "glx_if.hpp"
#include "gl_program.hpp"
#include "drawtoken/make_uniform.hpp"
#include "uniform_ent.hpp"

namespace rev {
	namespace unif {
		const UniformName
					Alpha("u_alpha"),
					Color("u_color");
		namespace texture {
			const UniformName
					Diffuse("u_texDiffuse");
		}
	}
	namespace {
		namespace su {
			const UniformName
					Size("sys_vScreenSize");
		}
	}

	// --------------------- SystemUniform ---------------------
	void SystemUniform::moveFrom(ISystemUniform& prev) {
		auto& p = dynamic_cast<SystemUniform&>(prev);
		_screenSize = p._screenSize;
	}
	const lubee::SizeI& SystemUniform::getScreenSize() const {
		return _screenSize;
	}
	void SystemUniform::setScreenSize(const lubee::SizeI& s) {
		_screenSize = s;
	}
	void SystemUniform::extractUniform(UniformSetF_V& dst, const GLProgram& prog) const {
		if(const auto id = prog.getUniformId(su::Size)) {
			dst.emplace_back([id=*id](const void* p, UniformEnt& u){
				auto& ss = static_cast<const SystemUniform*>(p)->getScreenSize();
				u.setUniform(
					id,
					frea::Vec4(
						ss.width,
						ss.height,
						1.f/ss.width,
						1.f/ss.height
					)
				);
			});
		}
	}
}
