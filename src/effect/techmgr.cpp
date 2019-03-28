#include "techmgr.hpp"
#include "tech_pass.hpp"
#include "blockset.hpp"
#include "tech.hpp"
#include "../sdl/rw.hpp"

namespace rev::parse {
	// -------------------- TechMgr::Reload --------------------
	void TechMgr::Reload::_removeFromDependency(const RName& rName, DepName& dep) const {
		for(auto& d : _depend) {
			auto [from, to] = dep.equal_range(d);
			bool found = false;
			while(from != to) {
				if(from->second == rName) {
					dep.erase(from);
					found = true;
					break;
				}
				++from;
			}
			D_Assert0(found);
		}
	}
	void TechMgr::Reload::_registerDependency(const RName& rName, DepName& dep) const {
		for(auto& d : _depend)
			dep.emplace(d, rName);
	}
	void TechMgr::Reload::_cleanRemovedTech() {
		auto itr = _tech.begin();
		const auto itrE = _tech.end();
		while(itr != itrE) {
			if(itr->second.expired())
				itr = _tech.erase(itr);
			else
				++itr;
		}
	}
	HTP TechMgr::Reload::load(const RName& rName, const BlockSet& bs, DepName& dep) {
		// シェーダーのコンパイルでエラーが出た場合、例外を送出するので最初にTechPassを作る
		const HTP ntp = std::make_shared<TechPass>(bs);

		_cleanRemovedTech();
		_removeFromDependency(rName, dep);
		_depend = bs.dependency;
		_registerDependency(rName, dep);

		// 生き残っているTechがあったらそのポインタを使う
		for(auto& t : ntp->refTech()) {
			if(const auto itr = _tech.find(t.first);
				itr != _tech.end())
			{
				const auto tech_sp = itr->second.lock();
				D_Assert0(tech_sp);
				dynamic_cast<Tech&>(*tech_sp) = std::move(dynamic_cast<Tech&>(*t.second));
				t.second = tech_sp;
			} else {
				_tech.emplace(t.first, t.second);
			}
		}

		// 前のTechPassが生き残っていたら上書き
		if(const auto tpsp = _tp.lock()) {
			*tpsp = std::move(*ntp);
			return tpsp;
		} else {
			_tp = ntp;
			return ntp;
		}
	}

	BlockSet TechMgr::_loadBlockSet(const RName& rName) {
		BlockSet bs;
		std::unordered_set<std::string>		loaded,
											toLoad{rName};
		// 現状はファイルパスのみ対応
		// toLoad = まだ読み込んでないファイル名
		while(!toLoad.empty()) {
			// toLoadから1つ取り出して読み込み
			const auto itr = toLoad.begin();
			auto hdl =
				loadResourceApp<GLXStruct>(
					UserURI(*itr),
					[&bs](auto& uri, auto&& mk){
						{
							auto& file = dynamic_cast<const FileURI&>(*uri.uri);
							bs.dependency.emplace_back(file.path());
						}
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

	HTP TechMgr::loadTechPass(const RName& rName) {
		return _reload[rName].load(rName, _loadBlockSet(rName), _depName);
	}
	void TechMgr::onFileModified(const Path_V& path) {
		if(path.empty())
			return;

		// ロードすべきTechPassリソース名を収集
		std::unordered_set<RName> toLoad;
		for(auto& p : path) {
			// 更新されたファイルを無効化
			this->setAnonymous(FileURI(p));
			auto [from, to] = _depName.equal_range(p);
			while(from != to) {
				toLoad.emplace(from->second);
				++from;
			}
		}
		for(auto& rn : toLoad) {
			_reload[rn].load(rn, _loadBlockSet(rn), _depName);
		}
	}
	namespace {
		const std::string cs_rtname[] = {
			"effect"
		};
	}
	TechMgr::TechMgr():
		ResMgrApp(cs_rtname)
	{}
}
