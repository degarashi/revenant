#include "watch.hpp"

namespace rev {
	// ------------------------- FEv -------------------------
	FEv::FEv(const String_SP& base, const std::string& nm, bool bDir):
		basePath(base),
		name(nm),
		isDir(bDir)
	{}
	bool FEv::operator == (const FEv& f) const {
		return 	*basePath == *f.basePath &&
				name == f.name &&
				isDir == f.isDir &&
				getType() == f.getType();
	}
	void FEv::print(std::ostream& os) const {
		os << "type: " << getName() << std::endl
		   << "basePath: " << *basePath << std::endl
			  << "name: " << name << std::endl
			  << "isDir: " << std::boolalpha << isDir << std::endl;
	}
	// ------------------------- FEv_Create -------------------------
	FileEvent FEv_Create::getType() const {
		return FileEvent::Create;
	}
	const char* FEv_Create::getName() const {
		return "FEv_Create";
	}
	// ------------------------- FEv_Access -------------------------
	FileEvent FEv_Access::getType() const {
		return FileEvent::Access;
	}
	const char* FEv_Access::getName() const {
		return "FEv_Access";
	}
	// ------------------------- FEv_Modify -------------------------
	FileEvent FEv_Modify::getType() const {
		return FileEvent::Modify;
	}
	const char* FEv_Modify::getName() const {
		return "FEv_Modify";
	}
	// ------------------------- FEv_Remove -------------------------
	FileEvent FEv_Remove::getType() const {
		return FileEvent::Remove;
	}
	const char* FEv_Remove::getName() const {
		return "FEv_Remove";
	}
	// ------------------------- FEv_Attr -------------------------
	FileEvent FEv_Attr::getType() const {
		return FileEvent::Attribute;
	}
	const char* FEv_Attr::getName() const {
		return "FEv_Attribute";
	}
	// ------------------------- FEv_MoveFrom -------------------------
	FEv_MoveFrom::FEv_MoveFrom(const String_SP& base, const std::string& name, const bool bDir, const uint32_t cookieID):
		FEv(base, name, bDir),
		cookie(cookieID)
	{}
	bool FEv_MoveFrom::operator == (const FEv& e) const {
		if(static_cast<const FEv&>(*this) == static_cast<const FEv&>(e))
			return cookie == reinterpret_cast<const FEv_MoveFrom&>(e).cookie;
		return false;
	}
	FileEvent FEv_MoveFrom::getType() const {
		return FileEvent::MoveFrom;
	}
	const char* FEv_MoveFrom::getName() const {
		return "FEv_MoveFrom";
	}
	void FEv_MoveFrom::print(std::ostream& os) const {
		FEv::print(os);
		os << "cookie: " << cookie << std::endl;
	}
	// ------------------------- FEv_MoveTo -------------------------
	FEv_MoveTo::FEv_MoveTo(const String_SP& base, const std::string& name, const bool bDir, const uint32_t cookieID):
		FEv(base, name, bDir),
		cookie(cookieID)
	{}
	bool FEv_MoveTo::operator == (const FEv& e) const {
		if(static_cast<const FEv&>(*this) == static_cast<const FEv&>(e))
			return cookie == reinterpret_cast<const FEv_MoveTo&>(e).cookie;
		return false;
	}
	FileEvent FEv_MoveTo::getType() const {
		return FileEvent::MoveTo;
	}
	const char* FEv_MoveTo::getName() const {
		return "FEv_MoveTo";
	}
	void FEv_MoveTo::print(std::ostream& os) const {
		FEv::print(os);
		os << "cookie: " << cookie << std::endl;
	}

	// ------------------------- FNotify -------------------------
	void FNotify::addWatch(const std::string& path, const uint32_t mask, const Data_SP& udata) {
		remWatch(path);
		const FNotifyDep::DSC dsc = _dep.addWatch(path, mask);
		_path2ent.emplace(path, Ent{dsc, String_SP(new std::string(path)), udata});
		_dsc2ent.emplace(dsc, &_path2ent.at(path));
	}
	void FNotify::remWatch(const std::string& path) {
		const auto itr = _path2ent.find(path);
		if(itr != _path2ent.end()) {
			_dep.remWatch(itr->second.dsc);
			_dsc2ent.erase(_dsc2ent.find(itr->second.dsc));
			_path2ent.erase(itr);
		}
	}
	void FNotify::procEvent(const std::function<void (const FEv&, const Data_SP&)>& cb) {
		struct Tmp : FRecvNotify {
			const std::function<void (const FEv&, const Data_SP&)>& _cb;
			Tmp(const std::function<void (const FEv&, const Data_SP&)>& cb): _cb(cb) {}

			void event(const FEv_Create& e, const Data_SP& ud) { _cb(e, ud); }
			void event(const FEv_Access& e, const Data_SP& ud) { _cb(e, ud); }
			void event(const FEv_Modify& e, const Data_SP& ud) { _cb(e, ud); }
			void event(const FEv_Remove& e, const Data_SP& ud) { _cb(e, ud); }
			void event(const FEv_MoveFrom& e, const Data_SP& ud) { _cb(e, ud); }
			void event(const FEv_MoveTo& e, const Data_SP& ud) { _cb(e, ud); }
			void event(const FEv_Attr& e, const Data_SP& ud) { _cb(e, ud); }
		} tmp(cb);
		procEvent(tmp);
	}
	void FNotify::procEvent(FRecvNotify& ntf) {
		_dep.procEvent([&ntf, this](const FNotifyDep::Event& e){
			const auto itr = _dsc2ent.find(e.dsc);
			D_Assert0(itr != _dsc2ent.end());
			auto& ent = itr->second;
			auto& ud = ent->udata;
			#define MAKE_EVENT(typ) ntf.event(typ(ent->basePath, e.path.c_str(), e.bDir), ud);
			#define MAKE_EVENTM(typ) ntf.event(typ(ent->basePath, e.path.c_str(), e.bDir, e.cookie), ud);
			switch(e.event) {
				case FileEvent::Create:
					MAKE_EVENT(FEv_Create) break;
				case FileEvent::Access:
					MAKE_EVENT(FEv_Access) break;
				case FileEvent::Modify:
					MAKE_EVENT(FEv_Modify) break;
				case FileEvent::Remove:
					MAKE_EVENT(FEv_Remove) break;
				case FileEvent::MoveFrom:
					MAKE_EVENTM(FEv_MoveFrom) break;
				case FileEvent::MoveTo:
					MAKE_EVENTM(FEv_MoveTo) break;
				default: break;
			}
			#undef MAKE_EVENTM
			#undef MAKE_EVENT
		});
	}
}
