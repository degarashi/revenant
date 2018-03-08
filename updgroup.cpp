#include "updgroup.hpp"
#include "lubee/sort.hpp"
#include "object_id.hpp"

namespace rev {
	TLS<bool> UpdGroup::tls_bUpdateRoot(false);
	UpdGroup::UGVec UpdGroup::s_ug;
	void UpdGroup::_registerUGVec() {
		if(_addObj.empty() && _remObj.empty())
			s_ug.emplace_back(shared_from_this());
	}
	void UpdGroup::_doAddRemove() {
		HGroup hThis = shared_from_this();
		for(;;) {
			// -- add --
			// オブジェクト追加中に更に追加オブジェクトが出るかも知れないので一旦退避
			auto addTmp = std::move(_addObj);
			for(auto& h : addTmp) {
				auto* p = h.second.get();
				if(p->isNode()) {
					// Groupリスト自体はソートしない
					_groupV.emplace_back(std::static_pointer_cast<UpdGroup>(h.second));
				}
				// 末尾に追加
				_objV.emplace_back(h);
				p->onConnected(hThis);
			}
			// -- remove --
			// オブジェクト削除中に更に削除オブジェクトが出るかも知れないので一旦退避
			auto remTmp = std::move(_remObj);
			for(auto& h : remTmp) {
				auto* p = h.get();
				if(p->isNode()) {
					auto itr = std::find_if(_groupV.begin(), _groupV.end(), [&h](const HGroup& hl){
												return hl == h;
											});
					if(itr != _groupV.end())
						_groupV.erase(itr);
					else
						continue;
				}
				auto itr = std::find_if(
					_objV.begin(),
					_objV.end(),
					[&h](const auto& obj){
						return obj.second == h;
					}
				);
				if(itr != _objV.end()) {
					p->onDisconnected(hThis);
					_objV.erase(itr);
				}
			}
			if(_addObj.empty() && _remObj.empty()) {
				// 優先度値は変化しないので多分、単純挿入ソートが最適
				lubee::insertion_sort(
							_objV.begin(),
							_objV.end(),
							[](const auto& obj0, const auto& obj1){
								return obj0.first < obj1.first;
							}
				);
				break;
			}
		}
	}
	void UpdGroup::_AllCheck() {
		#ifdef DEBUG
			for(auto u : s_ug) {
				if(auto hdl = u.lock()) {
					hdl->_checkInternalData();
				}
			}
		#endif
	}
	void UpdGroup::_checkInternalData() const {
		#ifdef DEBUG
			// 削除予定のオブジェクトがメンバリスト内に存在しているか
			for(auto& rt : _remObj) {
				auto itr = std::find_if(
					_objV.begin(),
					_objV.end(),
					[&rt](const auto& obj){
						return obj.second == rt;
					}
				);
				Assert0(itr != _objV.end());
			}
		#endif
	}
	void UpdGroup::ProcAddRemove() {
		_AllCheck();
		while(!s_ug.empty()) {
			// 削除中、他に追加削除されるオブジェクトが出るかも知れないので一旦リストを退避
			decltype(s_ug) tmp;
			tmp.swap(s_ug);
			for(auto ent : tmp) {
				if(auto hdl = ent.lock()) {
					hdl->_checkInternalData();
					hdl->_doAddRemove();
				}
			}
		}
	}
	void UpdGroup::SetAsUpdateRoot() {
		ProcAddRemove();
		tls_bUpdateRoot = true;
	}
	UpdGroup::UpdGroup(const Priority p):
		_priority(p),
		_nParent(0),
		_interval(interval::Wait(0), interval::EveryN(0))
	{}
	UpdGroup::~UpdGroup() {
		if(!std::uncaught_exception()) {
			D_Assert0(_nParent==0);
		}
	}
	Priority UpdGroup::getPriority() const {
		return _priority;
	}
	bool UpdGroup::isNode() const noexcept {
		return true;
	}
	ObjTypeId UpdGroup::getTypeId() const {
		return ObjectIdT<UpdGroup, idtag::Object>::Id;
	}
	void UpdGroup::onConnected(const HGroup& /*hGroup*/) {
		++_nParent;
	}
	void UpdGroup::onDisconnected(const HGroup& /*hGroup*/) {
		// 親グループから切り離された数のチェック
		--_nParent;
		D_Assert0(_nParent >= 0);
		if(_nParent == 0)
			clear();
	}
	interval::Wait& UpdGroup::refWait() {
		return std::get<interval::Wait>(_interval);
	}
	interval::EveryN& UpdGroup::refEveryN() {
		return std::get<interval::EveryN>(_interval);
	}
	void UpdGroup::onUpdate(bool /*execLua*/) {
		if(_interval.advance()) {
			class FlagSet {
				private:
					bool _bRootPrev = *tls_bUpdateRoot;
				public:
					FlagSet():
						_bRootPrev(tls_bUpdateRoot)
					{
						tls_bUpdateRoot = false;
					}
					~FlagSet() {
						tls_bUpdateRoot = _bRootPrev;
					}
			} flagset;

			for(auto& obj : _objV) {
				auto* ent = obj.second.get();
				const auto b = ent->_onUpdate();
				if(b) {
					// 次のフレーム直前で消す
					remObj(obj.second);
				}
			}
			_AllCheck();
		}
		// ルートノードで一括してオブジェクトの追加、削除
		if(tls_bUpdateRoot)
			ProcAddRemove();
	}
	void UpdGroup::onDraw(IEffect& e) const {
		// DrawUpdate中のオブジェクト追加削除はナシ
		for(auto& obj : _objV)
			obj.second->onDraw(e);
	}
	void UpdGroup::enumGroup(const CBFindGroup& cb, const GroupTypeId id, int depth) const {
		for(auto& h : _groupV) {
			if(h->getTypeId() == id)
				cb(h);
		}
		if(depth > 0) {
			--depth;
			for(auto& h : _groupV) {
				h->enumGroup(cb, id, depth);
			}
		}
	}
	bool UpdGroup::recvMsg(LCValue& dst, const GMessageStr& msg, const LCValue& arg) {
		bool received = false;
		for(auto& obj : _objV)
			received |= obj.second->recvMsg(dst, msg, arg);
		return received;
	}
	void UpdGroup::proc(const CBUpdProc& p, const bool bRecursive, const Priority prioBegin, const Priority prioEnd) {
		if(_objV.empty())
			return;

		auto itr = _objV.begin(),
			itrE = _objV.end();
		// prBeginの優先度までスキップ
		for(;;) {
			if(itr == itrE)
				return;
			if(itr->first >= prioBegin)
				break;
			++itr;
		}
		// prEndに達したらそれ以上処理しない
		do {
			if(itr == itrE || itr->first > prioEnd)
				return;
			itr->second->proc(p, bRecursive);
			++itr;
		} while(itr != itrE);
	}
	void UpdGroup::addObj(const HObj& hObj) {
		addObjPriority(hObj, hObj->getPriority());
	}
	void UpdGroup::addObjPriority(const HObj& hObj, const Priority p) {
		_registerUGVec();
		D_Assert(std::count_if(_addObj.begin(), _addObj.end(),
					[&hObj](const auto& ao){ return ao.second == hObj; }) == 0, "オブジェクトの重複登録");
		// すぐ追加するとリスト巡回が不具合起こすので後で一括処理
		_addObj.emplace_back(p, hObj);
	}
	void UpdGroup::remObj(const HObj& hObj) {
		_registerUGVec();
		// すぐ削除するとリスト巡回が不具合起こすので後で一括削除
		// onUpdateの最後で削除する
		D_Assert(std::find(_remObj.begin(), _remObj.end(), hObj) == _remObj.end(), "同一オブジェクトの複数回削除");
		D_Assert(
			std::find_if(
				_objV.begin(),
				_objV.end(),
				[hObj](auto& p){
					return p.second == hObj;
				}
			) != _objV.end(),
			"存在しないオブジェクトの削除"
		);
		_remObj.emplace_back(hObj);
	}
	const UpdGroup::ObjVP& UpdGroup::getList() const noexcept {
		return _objV;
	}
	UpdGroup::ObjVP& UpdGroup::getList() noexcept {
		return _objV;
	}
	int UpdGroup::getNMember() const noexcept {
		return getList().size();
	}
	void UpdGroup::clear() {
		_addObj.clear();
		_remObj.clear();
		// Remove時の処理をする為、一旦RemoveListに追加
		for(auto& obj : _objV)
			_remObj.push_back(obj.second);
		std::copy(_groupV.begin(), _groupV.end(), std::back_inserter(_remObj));
		_doAddRemove();

		D_Assert0(_objV.empty() && _groupV.empty() && _addObj.empty() && _remObj.empty());
	}
	HGroup UpdGroup::getHandle() {
		return shared_from_this();
	}
	const char* UpdGroup::getResourceName() const noexcept {
		return "UpdGroup";
	}

}
