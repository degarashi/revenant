#pragma once
#include "beat/pose3d.hpp"
#include "spine/treenode.hpp"
#include "../handle/model.hpp"
#include "common.hpp"
#include "../debuggui_if.hpp"
#include "jat_if.hpp"

namespace rev::dc {
	class TfNode :
		public lubee::CheckAlign<TfNode>,
		public lubee::AAllocator<TfNode>,
		public spi::TreeNode<TfNode>,
		public IDebugGui
	{
		private:
			using Pose_t = beat::g3::Pose;
			struct Pose;
			struct Getter {
				using counter_t = spi::Optional<uint32_t>;
				auto operator()(const Pose_t& p, Pose*, const TfNode&) const {
					return p.getAccum();
				}
			};
			using Transform_t = spi::AcCheck<frea::Mat4, Getter>;
			#define SEQ \
				((Pose)(Pose_t)) \
				((Transform)(Transform_t)(Pose))
			RFLAG_DEFINE(TfNode, SEQ)

		public:
			JointId		id;
			Name_SP		jointName;
			Name		userName;

			TfNode(JointId id, const Name_SP& jointName, const Name& userName);
			static void OnParentChange(spi::TreeNode<TfNode>* self, const HTfNode& node);
			RFLAG_GETMETHOD_DEFINE(SEQ)
			RFLAG_SETMETHOD_DEFINE(SEQ)
			RFLAG_REFMETHOD_DEFINE(SEQ)
			#undef SEQ

			DEF_DEBUGGUI_ALL
	};
	class TfRoot :
		public NodeParam,
		public IJointQuery,
		public IDebugGui
	{
		private:
			using HTfNodeV = std::vector<HTfNode>;
			using NameMap = std::unordered_map<Name, const TfNode*>;
			using IdMap = std::unordered_map<JointId, const TfNode*>;

			#define SEQ \
				((Node)(HTfNodeV)) \
				((NameToNode)(NameMap)(Node)) \
				((IdToNode)(IdMap)(Node))
			RFLAG_DEFINE(TfRoot, SEQ)
			RFLAG_GETMETHOD_DEFINE(SEQ)
		public:
			RFLAG_REFMETHOD_DEFINE(SEQ)
			RFLAG_SETMETHOD_DEFINE(SEQ)
			#undef SEQ
			const TfNode* find(JointId id) const;
			const TfNode* find(const Name& name) const;

			Mat4 getLocal(JointId id) const override;
			Mat4 getGlobal(JointId id) const override;
			Mat4 getLocal(const Name& name) const override;
			Mat4 getGlobal(const Name& name) const override;
			const Mat4V& getJointMat(const Mat4& node_m, const SkinBindV_SP& bind, const Mat4& bs_m) const override;

			TfNode& query(JointId id) const override;
			TfNode& query(const Name& name) const override;

			DEF_DEBUGGUI_NAME
			DEF_DEBUGGUI_PROP
	};
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
			Mat4 getLocal(const Name& name) const override;
			Mat4 getGlobal(const Name& name) const override;
			const Mat4V& getJointMat(const Mat4& node_m, const SkinBindV_SP& bind, const Mat4& bs_m) const override;
	};
}
