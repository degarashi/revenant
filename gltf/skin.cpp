#include "gltf/skin.hpp"
#include "gltf/accessor.hpp"

namespace rev::gltf {
	namespace {
		const frea::AMat4 c_defaultMat = frea::AMat4::Identity();
	}
	using namespace loader;
	Skin::Skin(const JValue& v):
		Resource(v),
		bindShapeMat(Optional<Mat4>(v, "bindShapeMatrix", c_defaultMat)),
		invBindMat(Required<TagAccessor>(v, "inverseBindMatrices")),
		jointName(Required<Array<Shared<StdString>>>(v, "jointNames"))
	{}
	Resource::Type Skin::getType() const noexcept {
		return Type::Skin;
	}
	void Skin::resolve(const ITagQuery& q) {
		invBindMat.resolve(q);
	}

	const dc::SkinBindV_SP& Skin::getBind() const {
		if(!bind_cached) {
			bind_cached = std::make_shared<dc::SkinBindV>();
			auto& bc = *bind_cached;
			const int n = jointName.size();
			bc.resize(n);

			const auto& mat = invBindMat->getDataAs<frea::Mat4>();
			auto* dst = bc.data();
			for(int i=0 ; i<n ; i++) {
				dst->jointName = jointName[i];
				dst->invmat = (*mat)[i];
				++dst;
			}
		}
		return bind_cached;
	}
}
