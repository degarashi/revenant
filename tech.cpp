#include "vertex.hpp"
#include "gl_state.hpp"
#include "gl_program.hpp"
#include "tech.hpp"

namespace rev {
	void Tech::Runtime::clear() {
		noDefValue.clear();
		defaultValue.clear();
		vattr.clear();
	}
	void Tech::ts_onDeviceLost() {
		D_Assert0(_bInit);
		_bInit = false;
		// OpenGLのリソースが絡んでる変数を消去
		_runtime.clear();
	}
	void Tech::ts_onDeviceReset(const IEffect& e) {
		D_Assert0(!_bInit);
		_bInit = true;
		_program->onDeviceReset();

		_onDeviceReset(e, _runtime);
	}
	const HProg& Tech::getProgram() const noexcept {
		return _program;
	}
	const Tech::Runtime& Tech::getRuntime() const noexcept {
		return _runtime;
	}
	void Tech::applySetting() const {
		for(auto& st : _setting)
			st->apply();
	}
}
