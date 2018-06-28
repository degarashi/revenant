#pragma once
#include "spine/optional.hpp"
#include "frea/vector.hpp"
#include "../external/mikktspace.h"

namespace rev::gltf {
	class MKStack {
		private:
			using Vec4 = frea::Vec4;
			using Vec4_Op = spi::Optional<Vec4>;
			using Vec4V = std::vector<Vec4>;
			using Idx_Op = spi::Optional<int32_t>;

			struct TItem {
				Vec4_Op		tangent;
				uint32_t	firstIndex;
				Idx_Op		nextIndex;
			};
			using TVec = std::vector<TItem>;
			TVec	_tvec;

		public:
			MKStack(const std::size_t len);
			uint32_t addTangent(const uint32_t index, const Vec4& tangent);
	};
	struct DuplInfo {
		using Idx = std::vector<int32_t>;
		struct Ent {
			uint32_t	from,
						to;
		};
		using EntV = std::vector<Ent>;

		EntV		copy;			//!< 頂点複製情報
		Idx			index;			//!< 拡張後のインデックスバッファ
		uint32_t	postLen,		//!< 拡張後の頂点個数
					prevLen;		//!< 拡張前の頂点個数
	};
	class MKInput {
		private:
			using Idx = std::vector<int32_t>;
			using FV = std::vector<float>;
			Idx			_index;
			FV			_pos,
						_normal,
						_uv;
			MKStack		_stack;

			using Ctx = SMikkTSpaceContext;
			using IF = SMikkTSpaceInterface;
			void _makeInterface(IF& i) const;

			DuplInfo	_dupl;
			using TangentV = std::vector<frea::Vec4>;
			TangentV	_tangent;

		public:
			MKInput(Idx&& idx, FV&& pos, FV&& nml, FV&& uv);
			void calcResult();
			const DuplInfo& getDuplInfo() const noexcept;
			const TangentV& getTangent() const noexcept;
	};
}
