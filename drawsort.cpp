#include "drawsort.hpp"
#include "lubee/sort.hpp"
#include "glx_if.hpp"

namespace rev {
	const DSort_SP cs_dsort_z_asc = std::make_shared<DSort_Z_Asc>(),
					cs_dsort_z_desc = std::make_shared<DSort_Z_Desc>(),
					cs_dsort_priority_asc = std::make_shared<DSort_Priority_Asc>(),
					cs_dsort_priority_desc = std::make_shared<DSort_Priority_Desc>(),
					cs_dsort_techpass = std::make_shared<DSort_TechPass>(),
					cs_dsort_texture = std::make_shared<DSort_Texture>(),
					cs_dsort_buffer = std::make_shared<DSort_Buffer>();

	void DSort::apply(const DrawTag& /*d*/, IEffect& /*e*/) {}
	void DSort::DoSort(const DSort_V& alg, int cursor, typename DLObj_V::iterator itr0, typename DLObj_V::iterator itr1) {
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
	const uint32_t DSort_TechPass::cs_invalidValue(~0);
	bool DSort_TechPass::hasInfo(const DrawTag& d) const {
		return d.idTechPass.value != cs_invalidValue;
	}
	bool DSort_TechPass::compare(const DrawTag& d0, const DrawTag& d1) const {
		return d0.idTechPass.value < d1.idTechPass.value;
	}
	void DSort_TechPass::apply(const DrawTag& d, IEffect& e) {
		if(hasInfo(d)) {
			if(d.idTechPass.bId16) {
				e.setTechnique(d.idTechPass.tpId.tech);
				e.setPass(d.idTechPass.tpId.pass);
			} else {
				e.setTechPassId(d.idTechPass.preId);
			}
		}
	}

	namespace detail {
		void DSort_UniformPairBase::_refreshUniformId(IEffect& e, const std::string* name, GLint* id, size_t length) {
			if(_pFx != &e) {
				_pFx = &e;
				for(int i=0 ; i<static_cast<int>(length) ; i++) {
					auto& s = name[i];
					id[i] = (!s.empty()) ? *e.getUniformId(s) : -1;
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
		for(int i=0 ; i<length ; i++) {
			if(id[i] >= 0)
				e.setUniform(id[i], d.idTex[i]);
		}
	}

	// ------------------- DSort_Buffer -------------------
	bool DSort_Buffer::hasInfo(const DrawTag& d) const {
		return d.idVBuffer[0] || d.idIBuffer;
	}
	bool DSort_Buffer::compare(const DrawTag& d0, const DrawTag& d1) const {
		auto val0 = reinterpret_cast<uintptr_t>(d0.idIBuffer.get()),
			 val1 = reinterpret_cast<uintptr_t>(d1.idIBuffer.get());
		if(val0 < val1)
			return true;
		if(val0 > val1)
			return false;
		return d0.idVBuffer < d1.idVBuffer;
	}
	void DSort_Buffer::apply(const DrawTag& d, IEffect& e) {
		for(int i=0 ; i<length ; i++) {
			auto& hdl = d.idVBuffer[i];
			if(hdl)
				e.setVStream(hdl, i);
		}
		e.setIStream(d.idIBuffer);
	}
}
