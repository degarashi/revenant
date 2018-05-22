#pragma once
#include "beat/pose3d.hpp"
#include "spine/treenode.hpp"
#include "../handle/model.hpp"
#include "common.hpp"
#include "../debuggui_if.hpp"
#include "jat_if.hpp"

namespace rev::dc {
	using Name = std::string;
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
			SName		jointName;
			Name		userName;

			TfNode(JointId id, const SName& jointName, const Name& userName);
			static void OnParentChange(spi::TreeNode<TfNode>* self, const HTfNode& node);
			RFLAG_GETMETHOD_DEFINE(SEQ)
			RFLAG_SETMETHOD_DEFINE(SEQ)
			RFLAG_REFMETHOD_DEFINE(SEQ)
			#undef SEQ

			DEF_DEBUGGUI_ALL
	};
	class TfRoot :
		public IQueryMatrix,
		public IJointQuery,
		public IDebugGui
	{
		private:
			using HTfNodeV = std::vector<HTfNode>;
			using NameMap = std::unordered_map<SName, const TfNode*>;
			using IdMap = std::unordered_map<JointId, const TfNode*>;

			#define SEQ \
				((Node)(HTfNodeV)) \
				((NameToNode)(NameMap)(Node)) \
				((IdToNode)(IdMap)(Node))
			RFLAG_DEFINE(TfRoot, SEQ)
			RFLAG_GETMETHOD_DEFINE(SEQ)
			mutable Mat4V _jointMat;
		public:
			RFLAG_REFMETHOD_DEFINE(SEQ)
			RFLAG_SETMETHOD_DEFINE(SEQ)
			#undef SEQ
			const TfNode* find(JointId id) const;
			const TfNode* find(const SName& name) const;

			Mat4 getLocal(JointId id) const override;
			Mat4 getGlobal(JointId id) const override;
			Mat4 getLocal(const SName& name) const override;
			Mat4 getGlobal(const SName& name) const override;
			const Mat4V& getJointMat(const Mat4& node_m, const SkinBindSet_SP& bind) const override;

			TfNode& query(JointId id) const override;
			TfNode& query(const SName& name) const override;

			DEF_DEBUGGUI_NAME
			DEF_DEBUGGUI_PROP
	};
}
