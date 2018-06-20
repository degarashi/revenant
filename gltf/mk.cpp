#include "mk.hpp"

namespace rev::gltf {
	// ---------------- MKStack ----------------
	MKStack::MKStack(const std::size_t len):
		_tvec(len)
	{
		for(std::size_t i=0 ; i<len ; i++)
			_tvec[i].firstIndex = i;
	}
	uint32_t MKStack::addTangent(const uint32_t index, const Vec4& tangent) {
		auto& ent = _tvec[index];
		if(!ent.tangent) {
			ent.tangent = tangent;
			return index;
		} else {
			uint32_t idx = index;
			for(;;) {
				auto& ent = _tvec[idx];
				if(ent.tangent->dot(tangent) > 1.f - 1e-3f) {
					return index;
				} else {
					if(ent.nextIndex)
						idx = *ent.nextIndex;
					else {
						const auto len = _tvec.size();
						_tvec.resize(len+1);
						_tvec[len].firstIndex = index;
						_tvec[len].tangent = tangent;
						ent.nextIndex = len;
						return len;
					}
				}
			}
		}
	}

	// ---------------- MKInput ----------------
	MKInput::MKInput(Idx&& idx, FV&& pos, FV&& nml, FV&& uv):
		_index(std::move(idx)),
		_pos(std::move(pos)),
		_normal(std::move(nml)),
		_uv(std::move(uv)),
		_stack(_pos.size()/3)
	{
		const auto pLen = _pos.size();
		const auto nLen = _normal.size();
		const auto uvLen = _uv.size();
		D_Assert0(pLen % 3 == 0 &&
				nLen % 3 == 0 &&
				uvLen % 2 == 0);
		D_Assert0(pLen/3 == nLen/3 &&
				pLen/3 == uvLen/2);
		_result.prevLen = pLen/3;
	}
	void MKInput::_makeInterface(IF& i) const {
		i = IF{
			.m_getNumFaces = [](const Ctx* ctx) -> int {
				auto* mk = reinterpret_cast<MKInput*>(ctx->m_pUserData);
				return mk->_index.size()/3;
			},
			.m_getNumVerticesOfFace = [](const Ctx*, const int) {
				return 3;
			},
			.m_getPosition = [](const Ctx* ctx, float pos[], const int iFace, const int iVert) {
				auto* mk = reinterpret_cast<MKInput*>(ctx->m_pUserData);
				const auto* src = mk->_pos.data() + mk->_index[iFace*3 + iVert]*3;
				pos[0] = src[0];
				pos[1] = src[1];
				pos[2] = src[2];
			},
			.m_getNormal = [](const Ctx* ctx, float nml[], const int iFace, const int iVert) {
				auto* mk = reinterpret_cast<MKInput*>(ctx->m_pUserData);
				const auto* src = mk->_normal.data() + mk->_index[iFace*3 + iVert]*3;
				nml[0] = src[0];
				nml[1] = src[1];
				nml[2] = src[2];
			},
			.m_getTexCoord = [](const Ctx* ctx, float uv[], const int iFace, const int iVert) {
				auto* mk = reinterpret_cast<MKInput*>(ctx->m_pUserData);
				const auto* src = mk->_uv.data() + mk->_index[iFace*3 + iVert]*2;
				uv[0] = src[0];
				uv[1] = src[1];
			},
			.m_setTSpaceBasic = [](const Ctx* ctx, const float tangent[], const float fSign, const int iFace, const int iVert) {
				auto* mk = reinterpret_cast<MKInput*>(ctx->m_pUserData);
				auto& res = mk->_result;
				const uint32_t from = mk->_index[iFace*3 + iVert];
				const frea::Vec4 tan(tangent[0], tangent[1], tangent[2], -fSign);

				const auto ni = mk->_stack.addTangent(from, tan);
				res.copy.emplace_back(
					Result::Ent {
						.from = from,
						.to = ni,
						.tangent = tan
					}
				);
				res.index.emplace_back(ni);
				res.maxIndex = std::max(res.maxIndex, ni);
			}
		};
	}
	const MKInput::Result& MKInput::calcResult() {
		_result.maxIndex = 0;
		IF i_f;
		_makeInterface(i_f);
		Ctx ctx {
			.m_pInterface = &i_f,
			.m_pUserData = this
		};
		genTangSpaceDefault(&ctx);
		return _result;
	}
}
