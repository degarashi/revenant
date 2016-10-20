#include "uri.hpp"
#include <regex>

namespace rev {
	const std::string URI::SEP(u8"://");
	const std::u32string URI::SEP32(U"://");
	URI::URI(URI&& u) noexcept:
		PathBlock(std::move(u)),
		_type(std::move(u._type))
	{}
	URI::URI(To8Str p) {
		setPath(p);
	}
	URI::URI(To8Str typ, To8Str path):
		PathBlock(path),
		_type(typ.moveTo())
	{}
	URI::URI(To8Str typ, const PathBlock& pb):
		PathBlock(pb),
		_type(typ.moveTo())
	{}

	URI& URI::operator = (URI&& u) noexcept {
		static_cast<PathBlock&>(*this) = std::move(u);
		_type = std::move(u._type);
		return *this;
	}
	void URI::setPath(To8Str p) {
		std::string path(p.moveTo());
		std::regex re("^([\\w\\d_]+)://");
		std::smatch m;
		if(std::regex_search(path, m, re)) {
			_type = m.str(1);
			PathBlock::setPath(path.substr(m[0].length()));
		} else
			PathBlock::setPath(std::move(path));
	}
	const std::string& URI::getType_utf8() const noexcept {
		return _type;
	}
	void URI::setType(To8Str typ) {
		_type = typ.moveTo();
	}
	const PathBlock& URI::path() const noexcept {
		return *this;
	}
	PathBlock& URI::path() noexcept {
		return *this;
	}
	std::string URI::plainUri_utf8() const {
		auto ret = _type;
		ret.append(SEP);
		ret.append(plain_utf8());
		return ret;
	}
	std::u32string URI::plainUri_utf32() const {
		auto ret = To32Str(_type).moveTo();
		ret.append(SEP32);
		ret.append(plain_utf32());
		return ret;
	}
	bool URI::operator == (const URI& u) const noexcept {
		return _type == u._type
				&& static_cast<const PathBlock&>(*this) == u;
	}
	bool URI::operator != (const URI& u) const noexcept {
		return !(this->operator == (u));
	}
}
