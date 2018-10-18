#pragma once
#include "resmgr_app.hpp"
#include "glx_parse.hpp"
#include "handle/opengl.hpp"
#include "tech_pass.hpp"

namespace rev::parse {
	class BlockSet;
	#define mgr_tech (::rev::parse::TechMgr::ref())
	class TechMgr : public ResMgrApp<GLXStruct>, public spi::Singleton<TechMgr> {
		private:
			using RName = std::string;
			// [TechName | PassName]
			using TechName = std::string;
			using Path = std::string;
			using Path_V = std::vector<Path>;

			// フルパス -> 依存するリソース名
			using DepName = std::unordered_multimap<Path, RName>;
			class Reload {
				private:
					using TechM = std::unordered_map<TechName, WTech>;

					Path_V	_depend;
					WTP		_tp;
					TechM	_tech;

					void _removeFromDependency(const RName& rName, DepName& dep) const;
					void _registerDependency(const RName& rName, DepName& dep) const;
					void _cleanRemovedTech();
				public:
					HTP load(const RName& rName, const BlockSet& bs, DepName& dep);
			};
			using ReloadM = std::unordered_map<RName, Reload>;

			DepName		_depName;
			ReloadM		_reload;

			BlockSet _loadBlockSet(const RName& rName);
		public:
			TechMgr();
			HTP loadTechPass(const RName& rName);
			// pathはフルパス
			void onFileModified(const Path_V& path);
	};
}
