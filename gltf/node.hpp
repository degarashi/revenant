#pragma once
#include "../handle/camera.hpp"
#include "beat/pose3d.hpp"
#include "json_types.hpp"

namespace rev::dc {
	class TfNode;
	using JointId = uint32_t;
}
namespace rev::gltf {
	template <class N>
	struct NodeVisitor {
		virtual ~NodeVisitor() {}
		virtual void upNode() {}
		virtual void addNode(const N& /*node*/) {}
		virtual void addCamera(const HCam3& /*c*/) {}
	};
	template <class D_Node, class Q>
	class Node {
		private:
			using Pose3 = beat::g3::Pose;
			using NodeV = std::vector<D_Node>;

		public:
			Pose3				pose;
			NodeV				child;
			dc::JointId			jointId;
			static dc::JointId	s_id;

		protected:
			template <class Self, class V, class CB>
			static void _Visit(Self& self, V& v, CB&& cb) {
				v.addNode(self);
				for(auto& c : self.child) {
					c.data()->visit(v);
				}
				cb();
				v.upNode();
			}

			Node() = default;
			Node(const JValue& v, const Q& q);
	};
}

#include "gltf/exceptions.hpp"
namespace rev::gltf::loader {
	template <class N, class... Ts>
	struct Node : std::shared_ptr<N> {
		using value_t = std::shared_ptr<N>;

		template <class T, class... Args>
		static value_t _Load(std::tuple<T>*, const Args&... args) {
			return std::make_shared<T>(args...);
		}
		template <class T0, class T1, class... Remain, class... Args>
		static value_t _Load(std::tuple<T0, T1, Remain...>*, const Args&... args) {
			if(const auto ret = T0::Load(args...))
				return ret;
			return _Load((std::tuple<T1, Remain...>*)nullptr, args...);
		}
		static const JValue& _CheckType(const JValue& v) {
			if(!v.IsObject())
				throw InvalidProperty("not an object");
			return v;
		}
		static bool CanLoad(const JValue& v) {
			return v.IsObject();
		}
		template <class... Args>
		Node(const JValue& v, const Args&... args):
			value_t(_Load((std::tuple<Ts...>*)nullptr, _CheckType(v), args...))
		{}
	};
}
