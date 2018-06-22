#pragma once
#include "gltf/v2/skinbind.hpp"
#include "gltf/v2/morph.hpp"

namespace rev {
	class U_Matrix3D;
	class IEffect;
	namespace dc {
		struct IQueryMatrix;
		struct BBox;
	}
}
namespace rev::gltf::v2 {
	using SkinBindSet_SP = std::shared_ptr<SkinBindSet>;
	// -------------- Skin Module --------------
	struct ISkinModule {
		virtual ~ISkinModule() {}
		virtual std::string getTechName() const = 0;
		virtual frea::AMat4 proc(U_Matrix3D& m3, const dc::IQueryMatrix& qm) const = 0;
		virtual const char* typeString() const noexcept = 0;
	};
	class SkinModule : public ISkinModule {
		private:
			SkinBindSet_SP		_bind;
		public:
			SkinModule(const SkinBindSet_SP& bind);
			std::string getTechName() const override;
			frea::AMat4 proc(U_Matrix3D& m3, const dc::IQueryMatrix& qm) const override;
			const char* typeString() const noexcept override;
	};
	class NoSkinModule : public ISkinModule {
		private:
			dc::JointId		_jointId;
		public:
			NoSkinModule(dc::JointId id);
			std::string getTechName() const override;
			frea::AMat4 proc(U_Matrix3D& m3, const dc::IQueryMatrix& qm) const override;
			const char* typeString() const noexcept override;
	};

	using WeightV = std::vector<float>;
	using WeightV_S = std::shared_ptr<WeightV>;
	// -------------- Other Module --------------
	struct IModule {
		virtual ~IModule() {}
		virtual void applyUniform(IEffect& e) const;
		virtual std::string getFlagString() const;
	};
	class BumpModule : public IModule {
		public:
			std::string getFlagString() const override;
	};
	class MorphModule : public IModule {
		private:
			Morph			_morph;
			WeightV_S		_weight;
		public:
			MorphModule(const Morph& m, const WeightV_S& w);
			void applyUniform(IEffect& e) const override;
			std::string getFlagString() const override;
	};
}
