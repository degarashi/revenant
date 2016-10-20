#pragma once
#include "path.hpp"
#include <cereal/types/base_class.hpp>

namespace rev {
	class URI : public PathBlock {
		private:
			const static std::string SEP;
			const static std::u32string SEP32;
			std::string		_type;

			friend class cereal::access;
			template <class Ar>
			void serialize(Ar& ar) {
				ar(cereal::base_class<PathBlock>(this), _type);
			}

		public:
			URI() = default;
			URI(To8Str p);
			URI(const URI& u) = default;
			URI(URI&& u) noexcept;
			URI(To8Str typ, To8Str path);
			URI(To8Str typ, const PathBlock& pb);

			URI& operator = (const URI&) = default;
			URI& operator = (URI&& u) noexcept;
			bool operator == (const URI& u) const noexcept;
			bool operator != (const URI& u) const noexcept;

			void setPath(To8Str p);
			const std::string& getType_utf8() const noexcept;
			std::string plainUri_utf8() const;
			std::u32string plainUri_utf32() const;
			void setType(To8Str typ);
			const PathBlock& path() const noexcept;
			PathBlock& path() noexcept;
	};
}
namespace std {
	template <>
	struct hash<rev::URI> {
		std::size_t operator()(const rev::URI& uri) const noexcept {
			return std::hash<std::u32string>()(uri.plain_utf32());
		}
	};
}
namespace cereal {
	template <class Ar>
	struct specialize<Ar, ::rev::URI, cereal::specialization::member_serialize> {};
}
