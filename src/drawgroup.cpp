#include "drawgroup.hpp"
#include "effect/drawsort.hpp"

namespace rev {
	namespace {
		const DSort_SP cs_dsort[SortAlg::_Num] = {
			cs_dsort_z_asc,
			cs_dsort_z_desc,
			cs_dsort_priority_asc,
			cs_dsort_priority_desc,
			cs_dsort_techpass,
			cs_dsort_texture,
			cs_dsort_primitive
		};
		const std::string cs_drawgroupname("DrawGroup");
	}
	DSort_V DrawGroup::_MakeDSort(const SortAlgList& al) {
		const auto sz = al.size();
		DSort_V ret(sz);
		for(std::size_t i=0 ; i<sz ; i++) {
			ret[i] = cs_dsort[static_cast<int>(al[i])];
		}
		return ret;
	}
	DrawGroup::DrawGroup(const Priority prio):
		DrawGroup(DSort_V{}, false, prio)
	{}
	DrawGroup::DrawGroup(const DSort_V& ds, const bool bDynamic, const Priority prio):
		DrawableObj(prio),
		_dsort(ds),
		_bDynamic(bDynamic)
	{}
	DrawGroup::DrawGroup(const SortAlgList& al, const bool bDynamic, const Priority prio):
		DrawableObj(prio),
		_dsort(_MakeDSort(al)),
		_bDynamic(bDynamic)
	{}
	bool DrawGroup::isNode() const noexcept {
		return true;
	}
	ObjTypeId DrawGroup::getTypeId() const {
		return ObjectIdT<DrawGroup, idtag::Object>::Id;
	}
	void DrawGroup::setSortAlgorithmId(const SortAlgList& al, const bool bDynamic) {
		setSortAlgorithm(_MakeDSort(al), bDynamic);
	}
	void DrawGroup::setSortAlgorithm(const DSort_V& ds, const bool bDynamic) {
		_dsort = ds;
		_bDynamic = bDynamic;
		if(!bDynamic) {
			// リスト中のオブジェクトをソートし直す
			_doDrawSort();
		}
	}
	const DSort_V& DrawGroup::getSortAlgorithm() const noexcept {
		return _dsort;
	}
	const DLObj_V& DrawGroup::getMember() const noexcept {
		return _dobj;
	}
	DrawTag& DrawGroup::refDTag() noexcept {
		return _dtag;
	}
	void DrawGroup::addObj(const HDObj& hObj) {
		auto* dtag = &hObj->getDTag();
		_dobj.emplace_back(dtag, hObj);
		// 毎フレームソートする設定でない時はここでソートする
		if(!_bDynamic)
			_doDrawSort();
	}
	void DrawGroup::remObj(const HDObj& hObj) {
		auto itr = std::find_if(
					_dobj.begin(),
					_dobj.end(),
					[hObj](const auto& p){
						return p.second == hObj;
					}
		);
		if(itr == _dobj.end()) {
			ExpectF("object not found");
			return;
		}
		_dobj.erase(itr);
	}
	void DrawGroup::clear() {
		_dobj.clear();
	}
	void DrawGroup::_doDrawSort() {
		DSort::DoSort(_dsort, 0, _dobj.begin(), _dobj.end());
	}
	void DrawGroup::onUpdate(bool /*execLua*/) {
		ExpectF("called deleted function: DrawGroup::onUpdate()");
	}
	void DrawGroup::onDraw(IEffect& e) const {
		if(_bDynamic) {
			// 微妙な実装
			const_cast<DrawGroup*>(this)->_doDrawSort();
		}
		// ソート済みの描画オブジェクトを1つずつ処理していく
		for(auto& d : _dobj) {
			d.second->onDraw(e);
		}
	}
	int DrawGroup::getNMember() const noexcept {
		return getMember().size();
	}
	const char* DrawGroup::getResourceName() const noexcept {
		return cs_drawgroupname.c_str();
	}
}
