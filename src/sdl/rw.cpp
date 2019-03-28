#include "rw.hpp"
#include "../fs/dir.hpp"
#include "lubee/src/random.hpp"
#include <SDL_filesystem.h>

namespace rev {
	// --------------------- RWE_Error ---------------------
	RWops::RWE_Error::RWE_Error(const std::string& /*title*/):
		std::runtime_error("")
	{}
	void RWops::RWE_Error::setMessage(const std::string& msg) {
		std::stringstream ss;
		ss << "RWops - " << _title << "(" << msg << ")";
		reinterpret_cast<std::runtime_error&>(*this) = std::runtime_error(ss.str());
	}
	// --------------------- RWE_File ---------------------
	RWops::RWE_File::RWE_File(const std::string& path):
		RWE_Error("can't open file"),
		_path(path)
	{
		setMessage(path);
	}
	// --------------------- RWE_OutOfRange ---------------------
	RWops::RWE_OutOfRange::RWE_OutOfRange(const int64_t pos, const Pos::e hence, const int64_t size):
		RWE_Error("file pointer out of range"),
		_hence(hence),
		_pos(pos),
		_size(size)
	{
		std::stringstream ss;
		ss << "position: ";
		switch(_hence) {
			case Pos::Begin:
				ss << "Begin"; break;
			case Pos::Current:
				ss << "Current"; break;
			default:
				ss << "End";
		}
		ss << std::endl;
		ss << "file length: " << size << std::endl;
		ss << "current: " << pos << std::endl;
		setMessage(ss.str());
	}
	// --------------------- RWE_Permission ---------------------
	RWops::RWE_Permission::RWE_Permission(const Access::e have, const Access::e tr):
		RWE_Error("invalid permission"),
		_have(have),
		_try(tr)
	{
		std::stringstream ss;
		ss << "permission: "<< std::endl;
		ss << "(having): " << std::hex << have << std::endl;
		ss << "(needed): " << std::hex << tr << std::endl;
		setMessage(ss.str());
	}
	// --------------------- RWE_IO ---------------------
	RWops::RWE_IO::RWE_IO(const bool bW, const std::size_t bs, const std::size_t nb, const char* msg):
		RWE_Error("error while read/write operation"),
		_bW(bW),
		_bsize(bs),
		_nblock(nb),
		_message(msg)
	{
		std::stringstream ss;
		ss << "write: " << bW;
		ss << "block size: " << bs;
		ss << "n block: " << nb;
		ss << "message: " << msg;
		setMessage(ss.str());
	}
	// --------------------- RWE_NullMemory ---------------------
	RWops::RWE_NullMemory::RWE_NullMemory():
		RWE_Error("null memory pointer detected")
	{}

	// --------------------- RWops::Data ---------------------
	RWops::Data::Data(SDL_RWops* ops) NOEXCEPT_IF_RELEASE {
		_init(ops);
	}
	void RWops::Data::_init(SDL_RWops* ops) NOEXCEPT_IF_RELEASE {
		_ops = ops;
		D_Assert0(ops);
	}
	RWops::Data::~Data() {
		// (ファイルが開けるか試す場合があるため)
		if(_ops)
			SDL_RWclose(_ops);
	}
	int64_t RWops::Data::tell() const noexcept {
		return SDL_RWtell(_ops);
	}
	SDL_RWops* RWops::Data::getOps() const noexcept {
		return _ops;
	}
	void RWops::Data::_seek(const int64_t pos) {
		Assert0(_ops);
		SDL_RWseek(_ops, pos, Pos::Begin);
		D_SDLAssert0();
	}

	// --------------------- RWops::TempData ---------------------
	RWops::TempData::TempData(void* ptr, const std::size_t s):
		Data(SDL_RWFromMem(ptr, s)),
		_ptr(ptr),
		_size(s)
	{}
	bool RWops::TempData::isMemory() const noexcept {
		return true;
	}
	typename RWops::Type::e RWops::TempData::getType() const noexcept {
		return Type::Temporal;
	}
	spi::Optional<const URI&> RWops::TempData::getUri() const noexcept {
		return spi::none;
	}
	std::size_t RWops::TempData::size() const noexcept {
		return _size;
	}
	typename RWops::DataPtr RWops::TempData::getMemory() {
		return {_ptr, _size};
	}
	typename RWops::DataPtrC RWops::TempData::getMemoryC() const {
		return {_ptr, _size};
	}

	// --------------------- RWops::VectorData ---------------------
	RWops::VectorData::VectorData(const HURI& uri, ByteBuff&& b):
		_uri(uri),
		_buff(std::move(b))
	{
		_init(SDLAssert(SDL_RWFromMem, _buff.data(), _buff.size()));
	}
	RWops::VectorData::VectorData(const HURI& uri, const void* ptr, const std::size_t size):
		VectorData(
			uri,
			ByteBuff(
				reinterpret_cast<const uint8_t*>(ptr),
				reinterpret_cast<const uint8_t*>(ptr) + size
			)
		)
	{}
	bool RWops::VectorData::isMemory() const noexcept {
		return true;
	}
	typename RWops::Type::e RWops::VectorData::getType() const noexcept {
		return Type::Vector;
	}
	spi::Optional<const URI&> RWops::VectorData::getUri() const noexcept {
		if(_uri)
			return *_uri;
		return spi::none;
	}
	std::size_t RWops::VectorData::size() const noexcept {
		return _buff.size();
	}
	typename RWops::DataPtr RWops::VectorData::getMemory() {
		return {_buff.data(), size()};
	}
	typename RWops::DataPtrC RWops::VectorData::getMemoryC() const {
		return {_buff.data(), size()};
	}

	// --------------------- RWops::FileData ---------------------
	bool RWops::FileData::isMemory() const noexcept {
		return false;
	}
	typename RWops::Type::e RWops::FileData::getType() const noexcept {
		return Type::File;
	}
	spi::Optional<const URI&> RWops::FileData::getUri() const noexcept {
		return _uri;
	}
	std::size_t RWops::FileData::size() const noexcept {
		const auto pos = SDL_RWtell(_ops);
		SDL_RWseek(_ops, 0, Pos::End);
		const std::size_t ret = SDL_RWtell(_ops);
		SDL_RWseek(_ops, pos, Pos::Begin);
		return ret;
	}
	typename RWops::DataPtr RWops::FileData::getMemory() {
		return {nullptr, 0};
	}
	typename RWops::DataPtrC RWops::FileData::getMemoryC() const {
		return {nullptr, 0};
	}
	SDL_RWops* RWops::FileData::_LoadFromFile(const PathBlock& path, const int access) {
		const auto pathstr = path.plain_utf8();
		auto* str = pathstr.data();
		str = PathBlock::RemoveDriveLetter(str, str + pathstr.length());
		SDLError err;
		err.reset();
		SDL_RWops* ops = SDL_RWFromFile(str, ReadModeStr(access).c_str());
		if(err.errorDesc())
			throw RWE_File(str);
		D_Assert0(ops);
		return ops;
	}
	RWops::FileData::FileData(const PathBlock& path, const int access):
		Data(_LoadFromFile(path, access)),
		_uri(path.plain_utf8()),
		_access(access)
	{}
	// --------------------- RWops ---------------------
	RWops RWops::_FromTemporal(const int mode, void* mem, const std::size_t size, const Callback_SP& cb) {
		if(!mem)
			throw RWE_NullMemory();
		return RWops(
			mode,
			std::make_unique<TempData>(mem, size),
			cb
		);
	}
	RWops RWops::FromByteBuffMove(const HURI& uri, ByteBuff&& buff, const Callback_SP& cb) {
		return RWops(
			Access::Read|Access::Write,
			std::make_unique<VectorData>(uri, std::move(buff)),
			cb
		);
	}
	RWops RWops::FromVector(const HURI& uri, const void* mem, const std::size_t size, const Callback_SP& cb) {
		return RWops(
			Access::Read|Access::Write,
			std::make_unique<VectorData>(uri, mem, size),
			cb
		);
	}
	RWops RWops::FromTemporal(void* mem, const size_t size, const Callback_SP& cb) {
		return _FromTemporal(Access::Read | Access::Write, mem, size, cb);
	}
	RWops RWops::FromConstTemporal(const void* mem, const std::size_t size, const Callback_SP& cb) {
		return _FromTemporal(Access::Read, const_cast<void*>(mem), size, cb);
	}
	RWops RWops::FromFile(const PathBlock& path, const int access) {
		return RWops(
			access,
			std::make_unique<FileData>(path, access),
			nullptr
		);
	}
	int RWops::ReadMode(const char* mode) noexcept {
		int ret = 0;
		const std::pair<char,int> c_flag[] = {
			{'r', Access::Read},
			{'w', Access::Write},
			{'b', Access::Binary}
		};
		const char* c = mode;
		while(*c != '\0') {
			for(auto& p : c_flag) {
				if(p.first == *c)
					ret |= p.second;
			}
			++c;
		}
		return ret;
	}
	std::string RWops::ReadModeStr(const int mode) {
		std::string ret;
		ret.resize(3);
		auto* pDst = &ret[0];
		if(mode & Access::Read)
			*pDst++ = 'r';
		if(mode & Access::Write)
			*pDst++ = 'w';
		if(mode & Access::Binary)
			*pDst++ = 'b';
		ret.resize(pDst - &ret[0]);
		return ret;
	}
	RWops::~RWops() {
		close();
	}
	void RWops::close() noexcept {
		if(_data && _endCB)
			_endCB->onRelease(*this);
		_access = 0;
		_endCB.reset();
		_data.reset();
	}
	RWops::RWops(RWops&& ops) noexcept:
		_access(ops._access),
		_data(std::move(ops._data)),
		_endCB(std::move(ops._endCB))
	{
		ops.close();
	}
	RWops& RWops::operator = (RWops&& ops) noexcept {
		this->~RWops();
		return *(new(this) RWops(std::move(ops)));
	}
	int RWops::getAccessFlag() const noexcept {
		return _access;
	}
	std::size_t RWops::read(void* dst, const std::size_t blockSize, const std::size_t nblock) {
		SDLError err;
		err.reset();
		const std::size_t ret = SDL_RWread(_data->getOps(), dst, blockSize, nblock);
		if(const char* msg = err.errorDesc())
			throw RWE_IO(false, blockSize, nblock, msg);
		return ret;
	}
	std::size_t RWops::write(const void* src, const std::size_t blockSize, const std::size_t nblock) {
		SDLError err;
		err.reset();
		const std::size_t ret = SDL_RWwrite(_data->getOps(), src, blockSize, nblock);
		if(const char* msg = err.errorDesc())
			throw RWE_IO(true, blockSize, nblock, msg);
		return ret;
	}
	int64_t RWops::seek(const int64_t offset, const Pos::e hence) {
		const auto res = SDL_RWseek(_data->getOps(), offset, hence);
		if(res < 0)
			throw RWE_OutOfRange(offset, hence, tell());
		return res;
	}
	uint16_t RWops::readBE16() {
		return SDL_ReadBE16(_data->getOps());
	}
	uint32_t RWops::readBE32() {
		return SDL_ReadBE32(_data->getOps());
	}
	uint64_t RWops::readBE64() {
		return SDL_ReadBE64(_data->getOps());
	}
	uint16_t RWops::readLE16() {
		return SDL_ReadLE16(_data->getOps());
	}
	uint32_t RWops::readLE32() {
		return SDL_ReadLE32(_data->getOps());
	}
	uint64_t RWops::readLE64() {
		return SDL_ReadLE64(_data->getOps());
	}
	bool RWops::writeBE(uint16_t value) {
		return SDL_WriteBE16(_data->getOps(), value) == 1;
	}
	bool RWops::writeBE(uint32_t value) {
		return SDL_WriteBE32(_data->getOps(), value) == 1;
	}
	bool RWops::writeBE(uint64_t value) {
		return SDL_WriteBE64(_data->getOps(), value) == 1;
	}
	bool RWops::writeLE(uint16_t value) {
		return SDL_WriteLE16(_data->getOps(), value) == 1;
	}
	bool RWops::writeLE(uint32_t value) {
		return SDL_WriteLE32(_data->getOps(), value) == 1;
	}
	bool RWops::writeLE(uint64_t value) {
		return SDL_WriteLE64(_data->getOps(), value) == 1;
	}
	namespace {
		template <class T>
		T ReadAll(RWops& ops) {
			const auto pos = ops.tell();
			const std::size_t sz = ops.size();
			T buff;
			buff.resize(sz);
			ops.seek(0, RWops::Pos::Begin);
			ops.read(&buff[0], sz, 1);
			ops.seek(pos, RWops::Pos::Begin);
			return buff;
		}
	}
	ByteBuff RWops::readAll() {
		return ReadAll<ByteBuff>(*this);
	}
	std::string RWops::readAllAsString() {
		return ReadAll<std::string>(*this);
	}
	RWops::Type::e RWops::getType() const noexcept { return _data->getType(); }
	bool RWops::isReadable() const noexcept {
		return _access & Access::Read;
	}
	bool RWops::isWritable() const noexcept {
		return _access & Access::Write;
	}
	bool RWops::isBinary() const noexcept {
		return _access & Access::Binary;
	}
	bool RWops::isMemory() const noexcept { return _data->isMemory(); }
	int64_t RWops::size() const noexcept { return _data->size(); }
	int64_t RWops::tell() const noexcept { return _data->tell(); }
	typename RWops::DataPtr RWops::getMemory() { return _data->getMemory(); }
	typename RWops::DataPtrC RWops::getMemoryC() const { return _data->getMemoryC(); }
	URIRef_OP RWops::getUri() const noexcept { return _data->getUri(); }
	const char* RWops::getResourceName() const noexcept {
		return "RWops";
	}
	SDL_RWops* RWops::getOps() const noexcept { return _data->getOps(); }

	// ---------------------------- RWMgr ----------------------------
	namespace {
		template <class T>
		class CharRange {
			private:
				T		_c0, _c1;
			public:
				CharRange(const T& c0, const T& c1):
					_c0(std::min(c0,c1)),
					_c1(std::max(c0,c1))
				{}
				CharRange(const T& c0):
					_c0(c0),
					_c1(c0)
				{}
				int size() const {
					return _c1 - _c0 + 1;
				}
				T* output(T* dst) const {
					auto cur = _c0,
						curE = _c1;
					for(;;) {
						*dst++ = static_cast<T>(cur);
						if(cur++ == curE)
							break;
					}
					return dst;
				}
		};
		template <class T>
		struct CharVec {
			using CharV = std::vector<T>;
			CharV	_charV;

			CharVec(std::initializer_list<CharRange<T>> il) {
				int count = 0;
				for(auto& cr : il)
					count += cr.size();

				_charV.resize(count);
				auto* ptr = _charV.data();
				for(auto& cr : il)
					ptr = cr.output(ptr);
			}
			int size() const {
				return _charV.size();
			}
			const T& get(int n) const {
				return _charV[n];
			}
		};
		constexpr int RANDOMLEN_MIN = 8,
						RANDOMLEN_MAX = 16,
						MAX_RETRY_COUNT = 256;
		const CharVec<char> c_charVec{
			{'A', 'Z'},
			{'a', 'z'},
			{'0', '9'},
			{'_'}
		};
		const std::string c_tmpDirName("tmp");
		static spi::Optional<lubee::RandomMT> g_rnd;
	}
	RWMgr::RWMgr(const std::string& org_name, const std::string& app_name):
		_orgName(org_name),
		_appName(app_name)
	{
		// 初回だけランダム生成器を初期化
		if(!g_rnd) {
			g_rnd = lubee::RandomMT::Make<4>();
			// 一時ファイルを掃除
			_cleanupTemporaryFile();
		}
	}
	HRW RWMgr::fromURI(const URI& uri, const int access) {
		HRW ret = procHandler(uri, access);
		if(!ret)
			throw RWops::RWE_File(uri.plain().c_str());
		return ret;
	}
	HRW RWMgr::fromFile(const PathBlock& path, const int access) {
		return base_t::emplace(RWops::FromFile(path, access));
	}
	HRW RWMgr::fromVector(ByteBuff&& buff) {
		return base_t::emplace(RWops::FromByteBuffMove(nullptr, std::move(buff), nullptr));
	}
	HRW RWMgr::fromMemory(const void* buff, const std::size_t size) {
		return base_t::emplace(RWops::FromVector(nullptr, buff, size, nullptr));
	}
	HRW RWMgr::fromConstTemporal(const void* p, const std::size_t size, const typename RWops::Callback_SP& cb) {
		return base_t::emplace(RWops::FromConstTemporal(p, size, cb));
	}
	HRW RWMgr::fromTemporal(void* p, const std::size_t size, const typename RWops::Callback_SP& cb) {
		return base_t::emplace(RWops::FromTemporal(p,size, cb));
	}

	namespace {
		struct SDLDeleter {
			void operator ()(void* ptr) const {
				SDL_free(ptr);
			}
		};
		using SDLPtr = std::unique_ptr<void, SDLDeleter>;
	}
	PathBlock RWMgr::makeFilePath() const {
		#ifdef ANDROID
			PathBlock path(SDL_AndroidGetInternalStoragePath());
		#else
			SDLPtr str(SDL_GetPrefPath(_orgName.c_str(), _appName.c_str()));
			PathBlock path(reinterpret_cast<const char*>(str.get()));
		#endif
		return path;
	}
	std::pair<HRW,std::string> RWMgr::createTemporaryFile() {
		// Temporaryディレクトリ構造を作る
		Dir tmpdir = makeFilePath();
		tmpdir.pushBack(c_tmpDirName);
		// 既に別のファイルがあるなどしてディレクトリが作れなければ内部で例外を投げる
		tmpdir.mkdir(FStatus::GroupRead | FStatus::OtherRead | FStatus::UserRWX);
		// ランダムなファイル名[A-Za-z0-9_]{8,16}を重複考えず作る
		std::string str;
		int retry_count = MAX_RETRY_COUNT;
		while(--retry_count >= 0) {
			const int length = g_rnd->getUniform<int>({RANDOMLEN_MIN, RANDOMLEN_MAX});
			const int csize = c_charVec.size();
			for(int i=0 ; i<length ; i++)
				str.append(1, c_charVec.get(g_rnd->getUniform<int>({0, csize-1})));
			// 同名のファイルが既に存在しないかチェック
			Dir dir(tmpdir);
			dir.pushBack(str);
			if(!dir.isFile()) {
				const std::string fpath = dir.plain_utf8();
				return std::make_pair(fromFile(dir, Access::Write), dir.plain_utf8());
			}
		}
		// 指定回数リトライしても駄目な場合はエラーとする
		Assert(false, "can't create temporary file");
		return std::make_pair(HRW(), std::string());
	}
	void RWMgr::_cleanupTemporaryFile() {
		PathBlock path = makeFilePath();
		path.pushBack(c_tmpDirName);
		path.pushBack("*");
		auto strlist = Dir::EnumEntryWildCard(path.plain_utf8());
		for(auto& s : strlist) {
			Dir dir(s);
			dir.remove();
		}
	}
}
