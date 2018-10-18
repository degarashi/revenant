#include "gltf/v2/skin.hpp"
#include "gltf/v2/node.hpp"
#include "gltf/v2/accessor.hpp"
#include "skinbind.hpp"
#include "../value_loader.hpp"

namespace rev::gltf::v2 {
	namespace L = gltf::loader;
	Skin::Skin(const JValue& v, const IDataQuery& q):
		Resource(v),
		invBindMat(L::Optional<DRef_Accessor>(v, "inverseBindMatrices", q)),
		skeleton(L::Optional<DRef_Node>(v, "skeleton", q)),
		joints(L::Required<L::Array<DRef_Node>>(v, "joints", q))
	{}
	Resource::Type Skin::getType() const noexcept {
		return Type::Skin;
	}
	namespace {
		bool HasRoot(const typename Node::base_t* node,
					const typename Node::base_t* target)
		{
			if(node == target)
				return true;
			if(node->parent)
				return HasRoot(node->parent, target);
			return false;
		}
	}
	const SkinBindSet_SP& Skin::getBind() const {
		if(!bind_cached) {
			bind_cached = std::make_shared<SkinBindSet>();
			auto& bc = *bind_cached;
			if(skeleton)
				bc.rootId = (*skeleton)->jointId;
			else {
				// (ルートノード{=0}はZ反転で固定の為)
				bc.rootId = 1;
			}

			const auto n = joints.size();
			bc.bind.resize(n);
			const Node* root = skeleton->data();
			for(std::size_t i=0 ; i<n ; i++) {
				auto& b = bc.bind[i];
				b.jointId = joints[i]->jointId;
				b.hasRoot = HasRoot(joints[i].data(), root);
			}
			if(invBindMat) {
				const auto& mat = (*invBindMat)->getAs<frea::Mat4>();
				const auto* m = reinterpret_cast<const frea::Mat4*>(mat.pointer);
				for(std::size_t i=0 ; i<n ; i++) {
					// glTfの入力は右からベクトルを掛ける仕様の為, 転置
					bc.bind[i].invmat = m[i].transposition();
				}
			} else {
				const auto IM = frea::Mat4::Identity();
				for(auto& b : bc.bind)
					b.invmat = IM;
			}
		}
		return bind_cached;
	}
}
