#include "uri.hpp"
#include "lubee/meta/assume.hpp"
#include "text.hpp"

namespace {
	template <class T, class F>
	std::shared_ptr<T> _Match(F&& func) {
		std::smatch m;
		if(func(m))
			return std::make_shared<T>(m);
		return nullptr;
	}
	template <class T>
	auto Match(const std::string& uri, const std::regex& re) {
		return _Match<T>([&](auto& m){
			return std::regex_match(uri, m, re);
		});
	}
	template <class T>
	auto Search(const std::string& uri, const std::regex& re) {
		return _Match<T>([&](auto& m){
			return std::regex_search(uri, m, re);
		});
	}
}
namespace rev {
	URI_SP MakeURIFromString(const char* s) {
		if(auto ret = FileURI::Interpret(s))
			return ret;
		if(auto ret = DataURI::Interpret(s))
			return ret;
		if(auto ret = UserURI::Interpret(s))
			return ret;
		return nullptr;
	}
	URI::~URI() {}
	bool URI::URI::operator == (const URI& u) const noexcept {
		if(getType() == u.getType()) {
			return BranchURI(*this, [&u](const auto& self){
				return self == static_cast<decltype(self)>(u);
			});
		}
		return false;
	}
	std::ostream& operator << (std::ostream& os, const URI& u) {
		return os << u.plain();
	}
	// ----------------- IdURI -----------------
	namespace {
		const std::string s_id("id://");
	}
	IdURI::IdURI(const uint64_t num):
		_num(num)
	{}
	uint64_t IdURI::getId() const noexcept {
		return _num;
	}
	bool IdURI::operator == (const IdURI& u) const noexcept {
		return _num == u._num;
	}
	std::string IdURI::path() const {
		return std::to_string(_num);
	}
	const std::string& IdURI::scheme() const noexcept {
		return s_id;
	}
	std::string IdURI::plain() const {
		return s_id + path();
	}
	std::size_t IdURI::getHash() const noexcept {
		return std::hash<uint64_t>()(_num);
	}
	URI_SP IdURI::clone() const {
		return std::make_shared<IdURI>(_num);
	}
	URI::Type IdURI::getType() const noexcept {
		return Type::Id;
	}

	// ----------------- UserURI -----------------
	namespace {
		const std::string s_user("user://");
		#define REP_USER_SEGMENT	"[a-zA-Z0-9+._-]+"
		// user://(REP_USER_SEGMENT)
		const std::regex re_user(std::string("^") + s_user + "(" REP_USER_SEGMENT ")");
	}
	UserURI::UserURI(const std::smatch& s):
		UserURI(s[1].str())
	{}
	UserURI::UserURI(const std::string& name):
		_name(name)
	{}
	const std::string& UserURI::getName() const noexcept {
		return _name;
	}
	UserURI_SP UserURI::Interpret(const std::string& uri) {
		return ::Match<UserURI>(uri, re_user);
	}
	bool UserURI::operator == (const UserURI& u) const noexcept {
		return _name == u._name;
	}
	std::string UserURI::path() const {
		return _name;
	}
	const std::string& UserURI::scheme() const noexcept {
		return s_user;
	}
	std::string UserURI::plain() const {
		return s_user + _name;
	}
	URI::Type UserURI::getType() const noexcept {
		return Type::User;
	}
	std::size_t UserURI::getHash() const noexcept {
		return std::hash<std::string>()(_name);
	}
	URI_SP UserURI::clone() const {
		return std::make_shared<UserURI>(*this);
	}

	// ----------------- FileURI -----------------
	namespace {
		const std::string s_file("file://");
		#define REP_SEGMENT		"[a-zA-Z0-9+.-]+"
		// file://((?:/)?(?:REP_SEGMENT)(?:/REP_SEGMENT)*)?
		// ex.	file:///first/second/third/file.ext
		// 1: /first/second/third/file.ext
		const std::string r_file = std::string("^") + s_file + "((?:/)?(?:" REP_SEGMENT ")(?:/" REP_SEGMENT ")*)?";
		const std::regex re_file(r_file);
	}
	FileURI::FileURI(To8Str p):
		_path(p)
	{}
	FileURI::FileURI(const std::smatch& s):
		FileURI(s[1].str())
	{}
	FileURI_SP FileURI::Interpret(const std::string& uri) {
		return ::Match<FileURI>(uri, re_file);
	}
	PathBlock& FileURI::pathblock() noexcept {
		return _path;
	}
	const PathBlock& FileURI::pathblock() const noexcept {
		return _path;
	}
	std::string FileURI::path() const {
		return _path.plain_utf8();
	}
	const std::string& FileURI::scheme() const noexcept {
		return s_file;
	}
	std::string FileURI::plain() const {
		return s_file + _path.plain_utf8();
	}
	URI::Type FileURI::getType() const noexcept {
		return Type::File;
	}
	std::size_t FileURI::getHash() const noexcept {
		return std::hash<PathBlock>()(_path);
	}
	bool FileURI::operator == (const FileURI& f) const noexcept {
		return _path == f._path;
	}
	URI_SP FileURI::clone() const {
		return std::make_shared<FileURI>(*this);
	}

	// ----------------- DataURI -----------------
	namespace {
		#define REP_MEDIAENT0	"(?:[a-zA-Z0-9+./-]+)"
		#define REP_MEDIAPAIR0	"(?:" REP_MEDIAENT0 "(?:=" REP_MEDIAENT0 ")?)"
		#define BASE64			"base64"
		// data:(REP_MEDIAPAIR0(?:;REP_MEDIAPAIR0)*?)?((?:;)base64)?,
		// ex. data:text/vnd-example+xyz;foo=bar;base64,R0lGODdh
		//		1: text/vnd-example+xyz;foo=bar
		//		2: base64
		//		suffix: R0lGODdh
		const std::string s_data("data:");
		const std::regex re_data(std::string("^") + s_data + "(" REP_MEDIAPAIR0 "(?:;" REP_MEDIAPAIR0 ")*?)?((?:;)" BASE64 ")?,");
		#define REP_MEDIAENT	"([a-zA-Z0-9+./-]+)"
		#define REP_MEDIAPAIR	"(?:(" REP_MEDIAENT ")(?:=(" REP_MEDIAENT "))?)"
		const std::regex re_media(";?" REP_MEDIAPAIR);
	}
	DataURI::DataURI(const std::smatch& s):
		DataURI(s[1].str(),
				s[2].length() != 0,
				s.suffix().str())
	{}
	DataURI::DataURI(
		const std::string& media,
		const bool base64,
		const std::string& data
	):
		_bBase64(base64)
	{
		{
			std::sregex_iterator itr{media.cbegin(), media.cend(), re_media},
								itrE{};
			while(itr != itrE) {
				auto& it = *itr;
				_mediaType.emplace_back(it[1].str(), it[2].str());
				++itr;
			}
		}
		_data = base64 ? text::Base64ToBinary(data) : data;
	}
	DataURI_SP DataURI::Interpret(const std::string& uri) {
		return ::Search<DataURI>(uri, re_data);
	}
	std::string DataURI::path() const {
		return std::string();
	}
	const std::string& DataURI::scheme() const noexcept {
		return s_data;
	}
	std::string DataURI::plain() const {
		std::string ret;
		bool bFirst = true;
		for(auto& m : _mediaType) {
			if(!bFirst)
				ret += ';';
			bFirst = false;

			ret += m.first;
			if(!m.second.empty()) {
				ret += '=';
				ret += m.second;
			}
		}
		if(_bBase64) {
			if(!bFirst)
				ret += ';';
			ret += BASE64;
		}
		ret += ',';
		ret += text::BinaryToBase64(_data);
		return ret;
	}
	URI::Type DataURI::getType() const noexcept {
		return Type::Data;
	}
	std::size_t DataURI::getHash() const noexcept {
		std::size_t ret = 0xdeadbeef;
		std::hash<std::string> h;
		for(auto& m : _mediaType)
			ret += h(m.first) + h(m.second);
		ret += std::hash<bool>()(_bBase64);
		ret += h(_data);
		return ret;
	}
	const DataURI::Data_t& DataURI::data() const noexcept {
		return _data;
	}
	bool DataURI::operator == (const DataURI& d) const noexcept {
		return _bBase64 == d._bBase64 &&
				_data == d._data &&
				_mediaType == d._mediaType;
	}
	URI_SP DataURI::clone() const {
		return std::make_shared<DataURI>(*this);
	}
}
