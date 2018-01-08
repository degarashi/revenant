#pragma once
#include "lubee/error.hpp"
#include "lubee/meta/enable_if.hpp"
#include "lubee/none.hpp"
#include "spine/resmgr.hpp"
#include "urihandler.hpp"
#include "uri.hpp"
#include "spine/singleton.hpp"
#include <SDL_rwops.h>
#include <stdexcept>
#include <vector>
#include "sdl_error.hpp"

namespace cereal {
	class access;
	template <class T>
	struct LoadAndConstruct;
}
namespace rev {
	using ByteBuff = std::vector<uint8_t>;
	template <class Itr>
	ByteBuff ToByteBuff(Itr itr, const Itr itrE) {
		constexpr auto unit = sizeof(decltype(*itr));
		const auto n = std::distance(itr, itrE);
		ByteBuff buff(unit * n);
		auto* dst = buff.data();
		while(itr != itrE) {
			*dst++ = *itr++;
		}
		return buff;
	}
	class URI;
	using URIRef_OP = spi::Optional<const URI&>;
	class RWMgr;

	class RWops : public Resource {
		public:
			struct Callback {
				virtual void onRelease(RWops& rw) = 0;
			};
			using Callback_SP = std::shared_ptr<Callback>;

			struct Type {
				enum e {
					Temporal,
					Vector,
					File,
					_Num
				};
			};
			template <class T>
			struct _DataPtr {
				T				data;
				std::size_t		length;

				explicit operator bool() const noexcept {
					return data;
				}
			};
			using DataPtr = _DataPtr<void*>;
			using DataPtrC = _DataPtr<const void*>;

			class Data {
				protected:
					SDL_RWops*	_ops;
					template <class T>
					friend struct cereal::LoadAndConstruct;
					void _seek(int64_t pos);
				protected:
					Data() = default;
					Data(SDL_RWops* ops) NOEXCEPT_IF_RELEASE;
					void _init(SDL_RWops* ops) NOEXCEPT_IF_RELEASE;
				public:
					SDL_RWops* getOps() const noexcept;
					int64_t tell() const noexcept;
					virtual ~Data();

					virtual bool isMemory() const noexcept = 0;
					virtual Type::e getType() const noexcept = 0;
					virtual URIRef_OP getUri() const noexcept = 0;
					virtual std::size_t size() const noexcept = 0;
					virtual DataPtr getMemory() = 0;
					virtual DataPtrC getMemoryC() const = 0;
			};
			using Data_UP = std::unique_ptr<Data>;
			#define DEF_METHODS \
				bool isMemory() const noexcept override; \
				Type::e getType() const noexcept override; \
				URIRef_OP getUri() const noexcept override; \
				std::size_t size() const noexcept override; \
				DataPtr getMemory() override; \
				DataPtrC getMemoryC() const override;

			//! 一時的なメモリバッファ(シリアライズ不可)
			class TempData : public Data {
				private:
					void*		_ptr;
					std::size_t	_size;

					template <class Ar>
					friend void serialize(Ar&, TempData&);
				public:
					TempData(void* ptr, std::size_t s);
					DEF_METHODS
			};
			//! 内部(メモリ上の)データ
			class VectorData : public Data {
				private:
					URI_SP			_uri;
					ByteBuff		_buff;

					template <class Ar>
					friend void serialize(Ar&, VectorData&);
					friend struct cereal::LoadAndConstruct<VectorData>;
				public:
					VectorData(const URI_SP& uri, ByteBuff&& b);
					VectorData(const URI_SP& uri, const void* ptr, std::size_t size);
					DEF_METHODS
			};
			//! ファイルシステムと関連付けされた外部データ
			class FileData : public Data {
				private:
					FileURI		_uri;
					int			_access;

					template <class Ar>
					friend void serialize(Ar&, FileData&);
					friend struct cereal::LoadAndConstruct<FileData>;

					static SDL_RWops* _LoadFromFile(const PathBlock& path, int access);
				public:
					FileData(const PathBlock& path, int access);
					DEF_METHODS
			};
			#undef DEF_METHODS

			struct Pos {
				enum e : uint32_t {
					Begin = RW_SEEK_SET,
					Current = RW_SEEK_CUR,
					End = RW_SEEK_END
				};
			};

			//! RWopsエラー基底
			struct RWE_Error : std::runtime_error {
				const std::string	_title;

				RWE_Error(const std::string& title);
				void setMessage(const std::string& msg);
			};
			//! ファイル開けないエラー
			struct RWE_File : RWE_Error {
				const std::string	_path;

				RWE_File(const std::string& path);
			};
			//! 範囲外アクセス
			struct RWE_OutOfRange : RWE_Error {
				const Pos::e	_hence;
				const int64_t	_pos, _size;

				RWE_OutOfRange(int64_t pos, Pos::e hence, int64_t size);
			};
			//! アクセス権限違反(読み取り専用ファイルに書き込みなど)
			struct RWE_Permission : RWE_Error {
				const Access::e		_have,
									_try;

				RWE_Permission(Access::e have, Access::e tr);
			};
			//! ファイル読み書きエラー
			struct RWE_IO: RWE_Error {
				const bool			_bW;
				const std::size_t	_bsize,
									_nblock;
				const char*			_message;

				RWE_IO(bool bW, std::size_t bs, std::size_t nb, const char* msg);
			};
			//! ぬるぽ指定
			struct RWE_NullMemory : RWE_Error {
				RWE_NullMemory();
			};

		private:
			//! アクセス許可フラグ
			uint32_t	_access;
			Data_UP		_data;
			//! RWopsが解放される直前に呼ばれる関数
			Callback_SP	_endCB;

			friend class cereal::access;
			template <class Ar>
			friend void serialize(Ar&, RWops&);

			RWops() = default;
			RWops(const int access, Data_UP data, const Callback_SP& cb) {
				_access = access;
				_data = std::move(data);
				_endCB = cb;
			}
			static RWops _FromTemporal(int mode, void* mem, std::size_t size, const Callback_SP& cb);

		public:
			static int ReadMode(const char* mode) noexcept;
			static std::string ReadModeStr(int mode);

			// メモリ上のデータ(ムーブ)
			static RWops FromByteBuffMove(const URI_SP& uri, ByteBuff&& buff, const Callback_SP& cb);
			// メモリ上のデータ(コピー)
			static RWops FromVector(const URI_SP& uri, const void* mem, std::size_t size, const Callback_SP& cb);
			// 外部データ(ポインタ+データ長)
			static RWops FromTemporal(void* mem, std::size_t size, const Callback_SP& cb);
			static RWops FromConstTemporal(const void* mem, std::size_t size, const Callback_SP& cb);
			// ファイルシステム上のデータ
			static RWops FromFile(const PathBlock& path, int access);

			RWops(RWops&& ops) noexcept;
			RWops& operator = (RWops&& ops) noexcept;
			~RWops();

			void close() noexcept;
			int getAccessFlag() const noexcept;
			std::size_t read(void* dst, std::size_t blockSize, std::size_t nblock);
			std::size_t write(const void* src, std::size_t blockSize, std::size_t nblock);
			int64_t size() const noexcept;
			int64_t seek(int64_t offset, Pos::e hence);
			int64_t tell() const noexcept;
			uint16_t readBE16();
			uint32_t readBE32();
			uint64_t readBE64();
			uint16_t readLE16();
			uint32_t readLE32();
			uint64_t readLE64();
			bool writeBE(uint16_t value);
			bool writeBE(uint32_t value);
			bool writeBE(uint64_t value);
			bool writeLE(uint16_t value);
			bool writeLE(uint32_t value);
			bool writeLE(uint64_t value);
			SDL_RWops* getOps() const noexcept;
			ByteBuff readAll();
			std::string readAllAsString();
			Type::e getType() const noexcept;
			bool isReadable() const noexcept;
			bool isWritable() const noexcept;
			bool isBinary() const noexcept;
			bool isMemory() const noexcept;
			//! isMemory()==true, isWritable()==trueの時だけ有効
			DataPtr getMemory();
			//! isMemory()==trueの時だけ有効
			DataPtrC getMemoryC() const;
			URIRef_OP getUri() const noexcept;

			const char* getResourceName() const noexcept override;
	};

	#define mgr_rw (::rev::RWMgr::ref())
	class RWMgr : public spi::ResMgr<RWops>, public UriHandlerV, public spi::Singleton<RWMgr> {
		private:
			using base_t = spi::ResMgr<RWops>;
			std::string		_orgName,
							_appName;
			//! 一時ファイルディレクトリのファイルを全て削除
			void _cleanupTemporaryFile();

			template <class Ar>
			friend void serialize(Ar&, RWMgr&);
			template <class T>
			friend struct cereal::LoadAndConstruct;

		public:
			RWMgr(const std::string& org_name, const std::string& app_name);

			//! 任意のURIからハンドル作成
			HRW fromURI(const URI& uri, int access);
			//! ローカルファイルからハンドル作成
			HRW fromFile(const PathBlock& pb, int access);
			// ------ メモリ上のデータからハンドル作成 ------
			HRW fromVector(ByteBuff&& buff);
			template <class T>
			HRW fromVector(const std::vector<T>& buff) {
				constexpr std::size_t s = sizeof(T);
				return fromMemory(buff.data(), reinterpret_cast<uintptr_t>(buff.data())+buff.size()*s);
			}
			template <class Itr>
			HRW fromIterator(const Itr itr, const Itr itrE) {
				return fromVector(ToByteBuff(itr, itrE));
			}
			HRW fromMemory(const void* buff, std::size_t size);
			//! ポインタから一時的なハンドル作成(const)
			/*!  \param[in]	cb		ハンドルが削除される時に呼び出される関数 */
			HRW fromConstTemporal(const void* p, std::size_t size, const typename RWops::Callback_SP& cb=nullptr);
			//! ポインタから一時的なハンドル作成(mutable)
			/*!  \param[in]	cb		ハンドルが削除される時に呼び出される関数 */
			HRW fromTemporal(void* p, std::size_t size, const typename RWops::Callback_SP& cb=nullptr);
			//! ランダムな名前の一時ファイルを作ってそのハンドルとファイルパスを返す
			std::pair<HRW, std::string> createTemporaryFile();
			//! OrgNameとAppNameからなるプライベートなディレクトリパス
			PathBlock makeFilePath() const;
	};
}
