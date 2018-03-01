#include "pose2d.hpp"
#include "beat/pose2d.hpp"
#include "vector.hpp"
#include "../../imgui/imgui.h"
#include "angle.hpp"
#include "../id.hpp"
#include "../indent.hpp"
#include "../child.hpp"

namespace rev {
	namespace debug {
		namespace {
			const char	*lb_posewindow = "pose-window",
						*lb_offset = "Offset",
						*lb_scale = "Scale",
						*lb_rotation = "Rotation";
		}
		// ----------------- Pose2DC -----------------
		Pose2DC::Pose2DC(const beat::g2::Pose& p):
			_p(p)
		{}
		void Pose2DC::show() const {
			if(const auto child = ChildPush(lb_posewindow, {0, ImGui::GetTextLineHeightWithSpacing()*7}, true, ImGuiWindowFlags_NoScrollbar)) {
				ImGui::TextUnformatted(lb_offset);
				{
					const IndentPush ind;
					const IdPush id("#0");
					VectorC<frea::Vec2>(_p.getOffset()).show();
				}
				ImGui::TextUnformatted(lb_scale);
				{
					const IndentPush ind;
					const IdPush id("#1");
					VectorC<frea::Vec2>(_p.getScaling()).show();
				}
				ImGui::TextUnformatted(lb_rotation);
				{
					const IndentPush ind;
					const IdPush id("#2");
					AngleC<frea::DegF>(_p.getRotation()).show();
				}
			}
		}

		// ----------------- Pose2D -----------------
		Pose2D::Pose2D(beat::g2::Pose& p):
			Pose2DC(p),
			_p(p)
		{}
		bool Pose2D::edit() const {
			bool ret = false;
			if(const auto child = ChildPush(lb_posewindow, {0, ImGui::GetTextLineHeightWithSpacing()*5}, true, ImGuiWindowFlags_NoScrollbar)) {
				{
					ImGui::TextUnformatted(lb_offset);
					ImGui::SameLine();
					auto tmp = _p.getOffset();
					if(FVector<frea::AVec2>(tmp).edit("#0")) {
						_p.setOffset(tmp);
						ret = true;
					}
				}
				{
					ImGui::TextUnformatted(lb_scale);
					ImGui::SameLine();
					auto tmp = _p.getScaling();
					if(FVector<frea::AVec2>(tmp).edit("#1")) {
						_p.setScaling(tmp);
						ret = true;
					}
				}
				{
					ImGui::TextUnformatted(lb_rotation);
					ImGui::SameLine();
					auto tmp = _p.getRotation();
					if(Angle<frea::RadF>(tmp).edit("#2")) {
						_p.setRotation(tmp);
						ret = true;
					}
				}
			}
			return ret;
		}
		namespace inner {
			void _Show(const ::beat::g2::Pose& p) {
				Pose2DC(p).show();
			}
			bool _Edit(::beat::g2::Pose& p) {
				return Pose2D(p).edit();
			}
		}
	}
}
