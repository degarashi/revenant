#include "parambase.hpp"
#include "../mainproc.hpp"
#include "../drawproc.hpp"
#include "../uri.hpp"

namespace rev::test {
	lubee::SizeI ParamBase::getScreenSize() const {
		return {1440, 900};
	}
	//! アプリケション名(一時ファイル用)
	const std::string& ParamBase::getAppName() const {
		static std::string name("App_Name");
		return name;
	}
	//! 組織名(一時ファイル用)
	const std::string& ParamBase::getOrgName() const {
		static std::string name("Org_Name");
		return name;
	}
	//! パス記述ファイル名
	const HURI& ParamBase::getPathfile() const {
		static HURI uri = std::make_shared<FileURI>("./resource/pathlist");
		return uri;
	}
	::rev::MainProc* ParamBase::makeMainProc() const {
		return new MainProc();
	}
	::rev::DrawProc* ParamBase::makeDrawProc() const {
		return new DrawProc();
	}
	bool ParamBase::getMultiContext() const noexcept {
		return true;
	}
}
