#include "gltf/v1/skin.hpp"
#include "gltf/v1/accessor.hpp"
#include "gltf/v1/dc_common.hpp"

namespace rev::gltf::v1 {
	namespace {
		const frea::AMat4 c_defaultMat = frea::AMat4::Identity();
	}
	using namespace loader;
	Skin::Skin(const JValue& v, const IDataQuery& q):
		Resource(v),
		bindShapeMat(OptionalDefault<Mat4>(v, "bindShapeMatrix", c_defaultMat)),
		invBindMat(Required<DRef_Accessor>(v, "inverseBindMatrices", q))
	{
		const auto names = Required<Array<StdString>>(v, "jointNames");
		const auto len = names.size();
		jointName.resize(len);
		for(std::size_t i=0 ; i<len ; i++) {
			jointName[i] = names[i];
		}
	}
	Resource::Type Skin::getType() const noexcept {
		return Type::Skin;
	}

	const SkinBindSet_SP& Skin::getBind() const {
		if(!bind_cached) {
			bind_cached = std::make_shared<SkinBindSet>();
			auto& bc = *bind_cached;
			const int n = jointName.size();
			bc.bind.resize(n);
			bc.bs_m = bindShapeMat.transposition();

			const auto& mat = invBindMat->getAs<frea::Mat4>();
			const auto* m = reinterpret_cast<const frea::Mat4*>(mat.pointer);
			auto* dst = bc.bind.data();
			for(int i=0 ; i<n ; i++) {
				dst->jointName = jointName[i];
				dst->invmat = m[i].transposition();
				++dst;
			}
		}
		return bind_cached;
	}
}
