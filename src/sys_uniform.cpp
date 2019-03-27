#include "sys_uniform.hpp"
#include "glx_if.hpp"
#include "gl/program.hpp"
#include "uniform_ent.hpp"
#include "spine/src/flyweight_item.hpp"

namespace rev {
	using UniformName = spi::FlyweightItem<std::string>;
	namespace {
		namespace su {
			const UniformName
					Size("sys_vScreenSize");
		}
	}

	// --------------------- SystemUniform ---------------------
	const lubee::SizeI& SystemUniform::getScreenSize() const {
		return _screenSize;
	}
	void SystemUniform::setScreenSize(const lubee::SizeI& s) {
		_screenSize = s;
	}
	UniformSetF SystemUniform::getUniformF(const GLProgram& prog) const {
		if(const auto id = prog.getUniformId(su::Size)) {
			return [id=*id](const void* p, UniformEnt& u){
				auto& ss = static_cast<const SystemUniform*>(p)->getScreenSize();
				u.setUniformById(
					id,
					frea::Vec4(
						ss.width,
						ss.height,
						1.f/ss.width,
						1.f/ss.height
					)
				);
			};
		}
		return nullptr;
	}
}
