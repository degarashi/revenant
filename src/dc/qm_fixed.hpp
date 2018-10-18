#pragma once
#include "qm_if.hpp"

namespace rev::dc {
	class TfNode;
	struct IJointQuery;
	// 参照元のノード構造、およびQueryMatrixの呼び出し順が変化しないと仮定するキャッシュ機構
	class QM_Fixed : public IQueryMatrix {
		private:
			struct Ent {
				const TfNode*	node;
				bool			local;

				std::size_t		serialId;
				mutable Mat4	result;

				Ent() = default;
				Ent(const TfNode* node, bool local, std::size_t sid);
				void refresh() const;
				bool operator == (const Ent& e) const noexcept;
			};
			struct Hash {
				std::size_t operator()(const Ent& e) const noexcept;
			};

			using Cache = std::unordered_set<Ent, Hash>;
			using CacheV = std::vector<Ent>;
			using CachePtrV = std::vector<const Ent*>;
			mutable Cache		_cache;
			mutable CacheV		_cacheV;
			mutable CachePtrV	_cachePtr;

			class CacheUse : public IQueryMatrix {
				private:
					const CachePtrV&		_cache;
					mutable std::size_t		_cursor;

					const Mat4& _getMat4() const;
				public:
					CacheUse(const CachePtrV& cache);
					void resetCursor() const;

					Mat4 getLocal(JointId id) const override;
					Mat4 getGlobal(JointId id) const override;
					Mat4 getLocal(const SName& name) const override;
					Mat4 getGlobal(const SName& name) const override;
			};
			CacheUse			_cacheUse;
			const IJointQuery&	_source;
			mutable std::size_t	_serialId;
			bool				_initialized;

			void _register(const TfNode* node, bool local) const;
			template <class Key>
			Mat4 _getMat(const Key& key, bool local) const;

		public:
			QM_Fixed(const QM_Fixed&) = delete;
			QM_Fixed(const IJointQuery& q);
			const IQueryMatrix& prepareInterface();
			void clearCache();

			Mat4 getLocal(JointId id) const override;
			Mat4 getGlobal(JointId id) const override;
			Mat4 getLocal(const SName& name) const override;
			Mat4 getGlobal(const SName& name) const override;
	};
}
