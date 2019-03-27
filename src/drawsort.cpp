#include "drawsort.hpp"
#include "lubee/src/sort.hpp"
#include "glx_if.hpp"
#include "primitive.hpp"
#include "tech_if.hpp"
#include "gl/program.hpp"
#include "uniform_ent.hpp"

namespace rev {
	const DSort_SP cs_dsort_z_asc = std::make_shared<DSort_Z_Asc>(),
					cs_dsort_z_desc = std::make_shared<DSort_Z_Desc>(),
					cs_dsort_priority_asc = std::make_shared<DSort_Priority_Asc>(),
					cs_dsort_priority_desc = std::make_shared<DSort_Priority_Desc>(),
					cs_dsort_techpass = std::make_shared<DSort_TechPass>(),
					cs_dsort_texture = std::make_shared<DSort_Texture>(),
					cs_dsort_primitive = std::make_shared<DSort_Primitive>();

	void DSort::apply(const DrawTag& /*d*/, IEffect& /*e*/) {}
	void DSort::DoSort(const DSort_V& alg, int cursor, typename DLObj_V::iterator itr0, typename DLObj_V::iterator itr1) {
		if(itr0 == itr1)
			return;
		if(cursor == int(alg.size()))
			return;
		auto* pAlg = alg[cursor].get();
		const auto fnCmp = [pAlg](const DLObj_P& p0, const DLObj_P& p1){
			return pAlg->compare(*p0.first, *p1.first);
		};
		lubee::insertion_sort(itr0, itr1, fnCmp);
		if(++cursor < int(alg.size())) {
			// 対象となる値が等しい範囲で別の基準にてソート
			auto itr = std::next(itr0);
			while(itr != itr1) {
				if(fnCmp(*itr0, *itr)) {
					DoSort(alg, cursor, itr0, itr);
					itr0 = itr;
				}
				++itr;
			}
			DoSort(alg, cursor, itr0, itr);
		}
	}
	// ------------------- DSort_Z_Asc -------------------
	const float DSort_Z_Asc::cs_border(-1e8f);
	bool DSort_Z_Asc::hasInfo(const DrawTag& d) const {
		return d.zOffset > cs_border;
	}
	bool DSort_Z_Asc::compare(const DrawTag& d0, const DrawTag& d1) const {
		return d0.zOffset < d1.zOffset;
	}
	// ------------------- DSort_Z_Desc -------------------
	bool DSort_Z_Desc::compare(const DrawTag& d0, const DrawTag& d1) const {
		return d0.zOffset > d1.zOffset;
	}
	// ------------------- DSort_Priority_Asc -------------------
	bool DSort_Priority_Asc::hasInfo(const DrawTag& /*d*/) const {
		return true;
	}
	bool DSort_Priority_Asc::compare(const DrawTag& d0, const DrawTag& d1) const {
		return d0.priority < d1.priority;
	}
	// ------------------- DSort_Priority_Desc -------------------
	bool DSort_Priority_Desc::compare(const DrawTag& d0, const DrawTag& d1) const {
		return d0.priority > d1.priority;
	}
	// ------------------- DSort_TechPass -------------------
	bool DSort_TechPass::hasInfo(const DrawTag& d) const {
		return static_cast<bool>(d.technique);
	}
	bool DSort_TechPass::compare(const DrawTag& d0, const DrawTag& d1) const {
		return d0.technique.get() < d1.technique.get();
	}
	void DSort_TechPass::apply(const DrawTag& d, IEffect& e) {
		if(hasInfo(d)) {
			e.setTechnique(d.technique);
		}
	}

	namespace detail {
		void DSort_UniformPairBase::_refreshUniformId(IEffect& e, const std::string* name, GLint* id, size_t length) {
			if(_pFx != &e) {
				_pFx = &e;
				auto& p = *e.getTechnique()->getProgram();
				for(int i=0 ; i<static_cast<int>(length) ; i++) {
					auto& s = name[i];
					id[i] = (!s.empty()) ? *p.getUniformId(s) : -1;
				}
			}
		}
	}
	// ------------------- DSort_Texture -------------------
	bool DSort_Texture::hasInfo(const DrawTag& d) const {
		return static_cast<bool>(d.idTex[0]);
	}
	bool DSort_Texture::compare(const DrawTag& d0, const DrawTag& d1) const {
		return std::lexicographical_compare(
					d0.idTex.begin(), d0.idTex.end(),
					d1.idTex.begin(), d1.idTex.end(),
					[](auto& t0, auto& t1){
						return reinterpret_cast<uintptr_t>(t0.get()) < reinterpret_cast<uintptr_t>(t1.get());
					}
				);
	}
	void DSort_Texture::apply(const DrawTag& d, IEffect& e) {
		auto& id = _getUniformId(e);
		auto& u = e.refUniformEnt();
		for(int i=0 ; i<length ; i++) {
			if(id[i] >= 0)
				u.setUniformById(id[i], d.idTex[i]);
		}
	}

	// ------------------- DSort_Primitive -------------------
	bool DSort_Primitive::hasInfo(const DrawTag& d) const {
		return static_cast<bool>(d.primitive);
	}
	bool DSort_Primitive::compare(const DrawTag& d0, const DrawTag& d1) const {
		return *d0.primitive < *d1.primitive;
	}
	void DSort_Primitive::apply(const DrawTag& d, IEffect& e) {
		e.setPrimitive(d.primitive);
	}
}
