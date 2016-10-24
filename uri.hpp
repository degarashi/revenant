#pragma once
#include "path.hpp"

namespace rev {
	class URI : public PathBlock {
		private:
			const static std::string SEP;
			const static std::u32string SEP32;
			std::string		_type;

			template <class Ar>
			friend void serialize(Ar&, URI&);

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
