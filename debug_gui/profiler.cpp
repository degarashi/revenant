#include "debug_gui/profiler.hpp"
#include "state_storage.hpp"
#include "../profiler_global.hpp"
#include "../imgui/imgui.h"
#include "print.hpp"
#include "indent.hpp"
#include "tree.hpp"
#include "column.hpp"
#include "sstream.hpp"
#include "header.hpp"
#include "child.hpp"
#include "spacing.hpp"
#include "../rev_time.hpp"
#include "../drawthread.hpp"
#include "../mainthread.hpp"
#include <iomanip>
#include "../dc/pos_sampler.hpp"
#include "frea/interpolation.hpp"
#include "style.hpp"

namespace rev::debug {
	namespace {
		template <class T>
		using Vec = std::vector<T>;
		const Vec<float> g_posV = {0, 25, 50, 75, 100};
		const dc::SVec<float>	g_posSV = std::make_shared<Vec<float>>(g_posV);
		const dc::SeekFrame g_posSamp(g_posSV);

		struct ColSampler : dc::FrameArray {
			Vec<frea::Vec4>	value;

			ColSampler(const Vec<frea::Vec4>& v):
				value(v)
			{}
			std::size_t numFrame() const override {
				return value.size();
			}
			ImVec4 sample(std::size_t idx, float t) const {
				frea::Vec4 v;
				if(t == 0.f)
					v = value[idx];
				else
					v = frea::Lerp(value[idx], value[idx+1], t);
				return {v.x, v.y, v.z, v.w};
			}
		};
		const ColSampler g_colorSamp({
			{1,1,1,1},
			{0,1,1,1},
			{1,1,0,1},
			{1,0.5,0,1},
			{1,0,0,1},
		});
	}
	Profiler::Profiler():
		_widthRatio(512.f),
		_height(24.f),
		_color {
			.frame = IM_COL32(255,255,255,192),
			.rect = IM_COL32(255,255,255,64),
			.select_rect = IM_COL32(128,128,255,128),
			.select_frame = IM_COL32(128,128,255,255),
		},
		_select(nullptr)
	{}
	namespace {
		float ToMicro(const Duration t) {
			using namespace std::chrono;
			return duration_cast<Nanoseconds>(t).count()/1000.f;
		}
		float ToMilli(const Duration t) {
			using namespace std::chrono;
			return duration_cast<Microseconds>(t).count()/1000.f;
		}
		float ToSecond(const Duration t) {
			using namespace std::chrono;
			return duration_cast<Milliseconds>(t).count()/1000.f;
		}
		void ShowTree_Node(const prof::Block& node, const Duration parentLength, const bool sibling) {
			const auto& h = node.hist;
			StringStream s;
			const float parentMilli = ToMicro(parentLength);
			const float percentage = (parentMilli > 0) ? (ToMicro(h.tAccum) / parentMilli * 100.f) : 0.f;
			s << node.name;
			s << "(" << h.nCalled << ")";
			s << " " << std::fixed << std::setprecision(2) << percentage << "%";
			s << " Avg:" << ToMilli(h.getAverageTime());
			s << "(" << ToMilli(node.getAverageTime(true)) << ")";
			s << " Accum:" << ToMilli(h.tAccum);
			s << "(" << ToMilli(h.tAccum - node.getLowerTime()) << ")";
			s << " [Min:" << ToMilli(h.tMin);
			s << ", Max=" << ToMilli(h.tMax) << "]";
			s << "##" << &node;
			{
				const auto p = g_posSamp.position(percentage);
				const auto color = g_colorSamp.sample(p.idx, p.time);
				const auto _ = StylePush(ImGuiCol_Text, color);
				if(!node.getChild()) {
					TreePush(s.output().c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
				} else {
					if(const auto _ = TreePush(s.output().c_str())) {
						if(const auto& c = node.getChild()) {
							ShowTree_Node(*c, node.hist.tAccum, true);
						}
					}
				}
			}
			if(sibling) {
				using Node = std::decay_t<decltype(node.getSibling())>;
				Node s = node.getSibling();
				while(s) {
					ShowTree_Node(*s, parentLength, false);
					s = s->getSibling();
				}
			}
		}
		void ShowTree_Interval(const prof::Interval& i) {
			const auto dur = ToSecond(i.beginTime - GetBeginTime());
			Show(nullptr, "At: ");
			ImGui::SameLine();
			Show(nullptr, dur);
			ShowTree_Node(*i.root, i.root->hist.tAccum, true);
		}
		struct CalcWidth {
			Duration	base;
			float		ratio;

			float operator()(const Duration dur) const {
				return ToMilli(dur) / ToMilli(base) * ratio;
			}
		};
		struct IntervalDrawer {
			CalcWidth	cw;
			float		height;
			Profiler::Color	color;
			Timepoint	beginTime;

			bool operator()(const frea::Vec2& ofs, const Timepoint begin, const Duration dur, const bool selecting) const {
				const float w = cw(dur);
				// オフセット位置を開始時刻に合わせる
				const ImVec2 from{ofs.x+cw(begin - beginTime), ofs.y},
							to{from.x+w, from.y+height};
				ImDrawList* dl = ImGui::GetWindowDrawList();
				dl->AddRectFilled(from, to, selecting ? color.select_rect : color.rect);
				dl->AddRect(from, to, selecting ? color.select_frame : color.frame);
				if(ImGui::IsMouseHoveringRect(from, to)) {
					ImGui::BeginTooltip();
					ImGui::Text("%0.2f(ms)", ToMilli(dur));
					ImGui::EndTooltip();
					if(ImGui::IsMouseClicked(0))
						return true;
				}
				return false;
			}
		};
	}
	void Profiler::draw() {
		{
			auto op = DrawThread::ref().getInfo();
			ImGui::Checkbox("VSync", &op->vsync);
		}
		ImGui::Spacing();
		{
			auto op = MainThread::ref().getInfo();
			ImGui::Checkbox("SpinWait", &op->spinwait);
		}
		ImGui::Separator();

		if(prof::IsCapturing())
			ImGui::TextUnformatted("Capturing...");
		else {
			if(ImGui::Button("Capture")) {
				_data.clear();
				_select = nullptr;
				prof::SetCaptureUntil(Clock::now() + Milliseconds(100));
			}
		}
		if(auto data = prof::GetCaptured()) {
			_data = std::move(*data);
			// sort by thread name
			std::sort(_data.begin(), _data.end(), [](const auto& t0, const auto& t1){
				return t0.threadName > t1.threadName;
			});
		}
		if(!_data.empty()) {
			Timepoint minTime = Timepoint::max(),
					  maxTime = Timepoint::min();
			for(auto& a : _data) {
				minTime = std::min(minTime, a.interval.front().beginTime);
				maxTime = std::max(maxTime, a.interval.back().getEndTime());
			}
			ImGui::PushItemWidth(512.f);
			ImGui::DragFloat("##WidthScaling", &_widthRatio, 1.f, 64.f, 2048.f);
			ImGui::PopItemWidth();
			IntervalDrawer idraw{
				CalcWidth{
					Milliseconds(100),
					_widthRatio
				},
				_height,
				_color,
				minTime
			};
			const float childHeight = Spacing::Item()
									+ Spacing::Item()*_data.size()
									+ Spacing::ItemSpacing()*2*_data.size()
									+ Spacing::ItemSpacing()*(_data.size()-1)
									+ _height * _data.size()
									+ Spacing::Child();
			if(const auto _ = ChildPush("Graph", {-1, childHeight}, true, ImGuiWindowFlags_HorizontalScrollbar)) {
				const float width = idraw.cw(maxTime - minTime);
				const float origin_y = ImGui::GetCursorScreenPos().y;
				for(auto& a : _data) {
					Show(nullptr, a.threadName);
					ImGui::Separator();
					const auto s = ImGui::GetCursorScreenPos();
					std::size_t idx = 0;
					for(auto& i : a.interval) {
						if(const auto& c = i.root->getChild()) {
							const auto _ = IdPush(idx++);
							if(idraw({s.x, s.y}, i.beginTime, c->hist.tAccum, _select == &i)) {
								_select = &i;
							}
						}
					}
					if(a.threadName == "MainThread") {
						StringStream ss;
						const auto n = a.interval.size();
						for(std::size_t i=0 ; i<n ; i++) {
							auto& in = a.interval[i];
							const float x = s.x + idraw.cw(in.beginTime - minTime);
							if(i > 0) {
								ImDrawList* dl = ImGui::GetWindowDrawList();
								dl->AddLine({x, origin_y}, {x, origin_y+childHeight}, idraw.color.frame);
								const float diff = ToMilli(in.beginTime - a.interval[i-1].beginTime);
								ss << "< " << diff << "(ms)";
								dl->AddText({x+2, origin_y}, idraw.color.frame, ss.output().c_str());
							}
						}
					}
					ImGui::Dummy({width, _height});
				}
			}
			if(_select) {
				ShowTree_Interval(*_select);
			}
		}
	}
}
