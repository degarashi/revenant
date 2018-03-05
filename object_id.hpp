#pragma once
#include <cstdint>

namespace rev {
	namespace idtag {
		struct Object {};
	}
	using ObjTypeId = uint32_t;			//!< Object種別Id
	// ---- Objectの固有Idを生成 ----
	namespace detail {
		template <class Tag>
		struct ObjectIdT {
			static ObjTypeId GenerateObjTypeId() {
				static ObjTypeId s_id(0);
				return s_id++;
			}
		};
	}
	// 型Tはdetail::ObjectIdTにて新しいIdを生成する為に使用
	template <class T, class Tag>
	struct ObjectIdT {
		const static ObjTypeId Id;
	};
	template <class T, class Tag>
	const ObjTypeId ObjectIdT<T, Tag>::Id(detail::ObjectIdT<Tag>::GenerateObjTypeId());
}
