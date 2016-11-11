#include "drawsort.hpp"

namespace rev {
	// -------------------- DrawTag --------------------
	DrawTag::DrawTag() {
		zOffset = DSort_Z_Asc::cs_border;
		priority = 0;
		idTechPass.value = DSort_TechPass::cs_invalidValue;
	}
	DrawTag::TPId::TPId():
		preId()
	{}
	DrawTag::TPId& DrawTag::TPId::operator = (const GL16Id& id) {
		value = 0x80000000 | (id.tech << sizeof(id.tech)) | id.pass;
		return *this;
	}
	DrawTag::TPId& DrawTag::TPId::operator = (const IdValue& id) {
		value = id._value;
		return *this;
	}
}
