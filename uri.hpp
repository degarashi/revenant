#pragma once
#include "path.hpp"
#include "lubee/operators.hpp"
#include "spine/enum.hpp"
#include "resource.hpp"
#include <regex>

namespace cereal {
	template <class T>
	struct LoadAndConstruct;
}
namespace rev {
	class URI;
	using URI_SP = std::shared_ptr<URI>;
	class URI :
		public lubee::op::Ne<URI>,
		public Resource
	{
		public:
			DefineEnum(Type,
				(Id)
				(User)
				(File)
				(Data)
			);
			virtual std::string path() const = 0;
			virtual const std::string& scheme() const noexcept = 0;
			virtual std::string plain() const = 0;
			virtual Type getType() const noexcept = 0;
			virtual std::size_t getHash() const noexcept = 0;
			virtual URI_SP clone() const = 0;
			const char* getResourceName() const noexcept override;

			bool operator == (const URI& u) const noexcept;
	};
	URI_SP MakeURIFromString(const char* s);
	std::ostream& operator << (std::ostream& os, const URI& u);

	#define DEF_URIMETHOD \
		std::string path() const override; \
		const std::string& scheme() const noexcept override; \
		std::string plain() const override; \
		Type getType() const noexcept override; \
		std::size_t getHash() const noexcept override; \
		URI_SP clone() const override; \
		const char* getDebugName() const noexcept override;

	// 内部管理用
	class IdURI;
	using IdURI_SP = std::shared_ptr<IdURI>;
	class IdURI : public URI {
		private:
			uint64_t	_num;
		public:
			IdURI(uint64_t num);
			uint64_t getId() const noexcept;
			DEF_URIMETHOD
			bool operator == (const IdURI& u) const noexcept;
			#ifdef DEBUGGUI_ENABLED
				void guiViewer(bool redirect=false) override;
			#endif
	};

	class UserURI;
	using UserURI_SP = std::shared_ptr<UserURI>;
	class UserURI : public URI {
		private:
			template <class Ar>
			friend void serialize(Ar&, UserURI&);
			template <class T>
			friend struct cereal::LoadAndConstruct;

			std::string		_name;
		public:
			UserURI(const std::string& name);
			UserURI(const std::smatch& s);
			static UserURI_SP Interpret(const std::string& uri);
			const std::string& getName() const noexcept;
			DEF_URIMETHOD
			bool operator == (const UserURI& u) const noexcept;
			#ifdef DEBUGGUI_ENABLED
				void guiViewer(bool redirect=false) override;
			#endif
	};

	class FileURI;
	using FileURI_SP = std::shared_ptr<FileURI>;
	class FileURI : public URI {
		private:
			template <class Ar>
			friend void serialize(Ar&, FileURI&);
			template <class T>
			friend struct cereal::LoadAndConstruct;

			PathBlock	_path;
		public:
			FileURI(To8Str p);
			FileURI(const std::smatch& s);
			static FileURI_SP Interpret(const std::string& uri);
			PathBlock& pathblock() noexcept;
			const PathBlock& pathblock() const noexcept;
			DEF_URIMETHOD
			bool operator == (const FileURI& f) const noexcept;
			#ifdef DEBUGGUI_ENABLED
				void guiViewer(bool redirect=false) override;
			#endif
	};

	class DataURI;
	using DataURI_SP = std::shared_ptr<DataURI>;
	namespace debug {
		namespace inner {
			void _Show(const DataURI&);
		}
	}
	class DataURI : public URI {
		private:
			template <class Ar>
			friend void serialize(Ar&, DataURI&);
			template <class T>
			friend struct cereal::LoadAndConstruct;
			friend void debug::inner::_Show(const DataURI&);

			using Data_t = std::string;
			using MediaType = std::pair<std::string, std::string>;
			using MediaTypeV = std::vector<MediaType>;
			MediaTypeV		_mediaType;
			bool			_bBase64;
			Data_t			_data;
		public:
			static DataURI_SP Interpret(const std::string& uri);
			DataURI() = default;
			DataURI(const std::string& media,
					bool base64,
					const std::string& data);
			DataURI(const std::smatch& s);
			DEF_URIMETHOD

			const Data_t& data() const noexcept;
			bool operator == (const DataURI& d) const noexcept;
			#ifdef DEBUGGUI_ENABLED
				void guiViewer(bool redirect=false) override;
			#endif
	};
	#undef DEF_URIMETHOD

	template <class CB>
	auto BranchURI(const URI& u, CB&& cb) {
		using Type = URI::Type;
		switch(u.getType()) {
			case Type::User:
				return cb(static_cast<const UserURI&>(u));
			case Type::File:
				return cb(static_cast<const FileURI&>(u));
			case Type::Data:
				return cb(static_cast<const DataURI&>(u));
			case Type::Id:
				return cb(static_cast<const IdURI&>(u));
			default:
				AssertF0();
		}
		AssertF("");
	}
}
#define DEF_HASH(typ)	\
	template <> \
	struct hash<typ> { \
		std::size_t operator()(const typ& uri) const noexcept { \
			return uri.getHash(); \
		} \
	};
namespace std {
	DEF_HASH(rev::URI)
	DEF_HASH(rev::UserURI)
	DEF_HASH(rev::FileURI)
	DEF_HASH(rev::DataURI)
}
#undef DEF_HASH
