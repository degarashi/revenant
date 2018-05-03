#pragma once
#include "../handle/camera.hpp"
#include "lubee/rect.hpp"
#include "node_cached_if.hpp"

namespace rev::gltf {
	class NodeParam_USemCached :
		public dc::NodeParam_cached,
		public NodeParam_USem
	{
		private:
			using base_t = dc::NodeParam_cached;
			using Mat4 = frea::Mat4;
			using Vec4 = frea::Vec4;
			// ----------------- for USemantics -----------------
			struct USemKey {
				dc::JointId		jointId;
				USemantic		sem;

				std::size_t operator()(const USemKey& k) const noexcept;
				bool operator == (const USemKey& k) const noexcept;
			};
			using USemM = std::unordered_map<USemKey, Mat4, USemKey>;
			using USemM_T = std::unordered_map<USemKey, draw::Token_SP, USemKey>;
			mutable USemM	_matrix;
			mutable USemM_T	_semantic;

			HCam3			_camera;
			Mat4			_view,
							_proj,
							_viewProj;
			struct {
				draw::Token_SP	view,
								proj;
			} _token;
			// ----------------- for Viewport -----------------
			// Viewport
			Vec4			_viewport;
			Mat4 _calcMat(JointId id, USemantic sem) const;

		public:
			NodeParam_USemCached(const HCam3& cam, const lubee::RectF& vp, const NodeParam& np);

			draw::Token_SP getSemantic(JointId id, USemantic sem) const override;
			draw::Token_SP getViewport() const override;

			dc::Mat4 getLocal(JointId id) const override;
			dc::Mat4 getGlobal(JointId id) const override;
			dc::Mat4 getLocal(const dc::Name& name) const override;
			dc::Mat4 getGlobal(const dc::Name& name) const override;
			const dc::Mat4V& getJointMat(const dc::Mat4& node_m, const dc::SkinBindV_SP& bind, const dc::Mat4& bs_m) const override;
	};
}
