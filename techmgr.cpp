#include "techmgr.hpp"
#include "sdl_rw.hpp"
#include "tech_pass.hpp"
#include "blockset.hpp"

namespace rev {
	namespace parse {
		BlockSet TechMgr::_loadBlockSet(const Path& path) {
			BlockSet bs;
			std::unordered_set<std::string>		loaded,
												toLoad{path};
			// 現状はファイルパスのみ対応
			// toLoad = まだ読み込んでないファイル名
			while(!toLoad.empty()) {
				// toLoadから1つ取り出して読み込み
				const auto itr = toLoad.begin();
				auto hdl = loadResourceApp<GLXStruct>(
								UserURI(*itr),
								[](auto& uri, auto&& mk){
									auto s = mgr_rw.fromURI(*uri.uri, Access::Read);
									mk();
									ParseGlx(*mk.pointer, s->readAllAsString());
								}
							).first;
				loaded.emplace(*itr);
				toLoad.erase(itr);
				// まだ読み込んでないファイルをtoLoadに加える
				for(auto& inc : hdl->incl) {
					if(loaded.count(inc) == 0)
						toLoad.emplace(inc);
				}
				bs.emplace(std::move(hdl));
			}
			return bs;
		}
		namespace {
			const std::string cs_rtname[] = {
				"effect"
			};
		}
		TechMgr::TechMgr():
			ResMgrApp(cs_rtname)
		{}
		HTP TechMgr::loadTechPass(const Path& path) {
			return std::make_shared<TechPass>(_loadBlockSet(path));
		}
	}
}
