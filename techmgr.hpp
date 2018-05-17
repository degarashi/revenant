#pragma once
#include "tech_pass.hpp"
#include "spine/singleton.hpp"

namespace rev {
	#define mgr_tech	(::rev::TechMgr::ref())
	class TechMgr :
		public spi::Singleton<TechMgr>
	{
		public:
			//! ファイルからエフェクトの読み込み
			HTP loadTechPass(const std::string& path);
	};
}
