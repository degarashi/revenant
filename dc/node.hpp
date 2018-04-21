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
			struct Name_Node {
				const Name*		name;
				const TfNode*	node;
			};
			using Name_NodeV = std::vector<Name_Node>;

			struct Id_Node {
				JointId			id;
				const TfNode*	node;
			};
			using Id_NodeV = std::vector<Id_Node>;

			#define SEQ \
				((Node)(HTfNodeV)) \
				((NameToNode)(Name_NodeV)(Node)) \
				((IdToNode)(Id_NodeV)(Node))
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

			TfNode& query(JointId id) const override;
			TfNode& query(const Name& name) const override;

			DEF_DEBUGGUI_NAME
			DEF_DEBUGGUI_PROP
	};
}
