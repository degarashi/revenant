#include "vertex.hpp"
#include "gl_state.hpp"
#include "gl_program.hpp"
#include "material.hpp"

namespace rev {
	void Material::Runtime::clear() {
		noDefValue.clear();
		defaultValue.clear();
		vattr.clear();
	}
	void Material::ts_onDeviceLost() {
		D_Assert0(_bInit);
		_bInit = false;
		// OpenGLのリソースが絡んでる変数を消去
		_runtime.clear();
	}
	void Material::ts_onDeviceReset(const IEffect& e) {
		D_Assert0(!_bInit);
		_bInit = true;
		_program->onDeviceReset();

		_onDeviceReset(e, _runtime);
	}
	const HProg& Material::getProgram() const noexcept {
		return _program;
	}
	const Material::Runtime& Material::getRuntime() const noexcept {
		return _runtime;
	}
	void Material::applySetting() const {
		for(auto& st : _setting)
			st->apply();
	}
}
