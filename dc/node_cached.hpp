#pragma once
#include "common.hpp"

namespace rev::dc {
	class NodeParam_cached : public NodeParam {
		private:
			struct Hash {
				template <class T>
				std::size_t operator()(const T& t) const noexcept {
					return t.hash();
				}
			};
			struct MatP {
				Mat4		node_m,
							bs_m;

				std::size_t hash() const noexcept;
				bool operator == (const MatP& m) const noexcept;
			};
			struct KeyS {
				MatP		mat;
				SkinBind	bind;

				std::size_t hash() const noexcept;
				bool operator == (const KeyS& k) const noexcept;
			};
			using SingleM = std::unordered_map<KeyS, Mat4, Hash>;

			struct KeyV {
				MatP			mat;
				SkinBindV_SP	bind;

				std::size_t hash() const noexcept;
				bool operator == (const KeyV& k) const noexcept;
			};
			using VecM = std::unordered_map<KeyV, Mat4V, Hash>;

			mutable SingleM		_single;
			mutable VecM		_vec;
			const NodeParam&	_np;

			const Mat4& _getSingle(const MatP& mat, const SkinBind& bind) const;
		public:
			NodeParam_cached(const NodeParam& np);
			Mat4 getLocal(JointId id) const override;
			Mat4 getGlobal(JointId id) const override;
			Mat4 getLocal(const SName& name) const override;
			Mat4 getGlobal(const SName& name) const override;
			const Mat4V& getJointMat(const Mat4& node_m, const SkinBindV_SP& bind, const Mat4& bs_m) const override;
	};
}
