#include "main.hpp"
#include "../../uri.hpp"
#include "../../glx.hpp"
#include "../../gl_resource.hpp"

namespace rev {
	namespace test {
		lubee::SizeI Param::getScreenSize() const {
			return {640, 480};
		}
		//! アプリケション名(一時ファイル用)
		const std::string& Param::getAppName() const {
			static std::string name("App_Name");
			return name;
		}
		//! 組織名(一時ファイル用)
		const std::string& Param::getOrgName() const {
			static std::string name("Org_Name");
			return name;
		}
		//! パス記述ファイル名
		const URI& Param::getPathfile() const {
			static URI uri("file", "./pathlist");
			return uri;
		}
		HFx Param::loadEffect(const std::string& name) const {
			return mgr_gl.template loadEffect<GLEffect>(name);
		}
		IMainProc* Param::makeMainProc(const Window_SP&) const {
			return new MainProc();
		}
		IDrawProc* Param::makeDrawProc() const {
			return new DrawProc();
		}
		bool Param::getMultiContext() const noexcept {
			return true;
		}
	}
}
