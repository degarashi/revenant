#pragma once
#include "common.hpp"
#include "../cache.hpp"

namespace rev::dc {
	class NodeParam_cached : public NodeParam {
		private:
			struct KeyS {
				SkinBind	bind;
				Mat4		bs_m;

				std::size_t operator()(const KeyS& k) const noexcept;
				bool operator == (const KeyS& k) const noexcept;
			};
			mutable Cache<KeyS, Mat4, KeyS>			_single;
			mutable Cache<SkinBindSet_SP, Mat4V>	_vec;
			const NodeParam&	_np;
			JointId				_jointId;
			Mat4				_node_m;
		public:
			NodeParam_cached(const NodeParam& np);
			NodeParam_cached(const NodeParam_cached& np);
			void setNodeJointId(JointId id);
			Mat4 getLocal(JointId id) const override;
			Mat4 getGlobal(JointId id) const override;
			Mat4 getLocal(const SName& name) const override;
			Mat4 getGlobal(const SName& name) const override;
			const Mat4V& getJointMat(const Mat4& node_m, const SkinBindSet_SP& bind) const override;
	};
}
