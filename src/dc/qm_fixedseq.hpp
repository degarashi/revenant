#pragma once
#include "qm_if.hpp"

namespace rev::dc {
	// QueryMatrixの呼び出し順が変化しないと仮定するキャッシュ機構
	class QM_FixedSequence : public IQueryMatrix {
		private:
			struct IGet {
				Mat4	result;

				virtual ~IGet();
				virtual void refresh(const IQueryMatrix& q) = 0;
				virtual std::size_t getHash() const noexcept = 0;
				bool operator == (const IGet& g) const noexcept;
			};
			struct GetById : IGet {
				JointId	id;
				bool	local;

				GetById(JointId id, bool local);
				void refresh(const IQueryMatrix& q) override;
				std::size_t getHash() const noexcept override;
				bool operator == (const GetById& g) const noexcept;
			};
			struct GetByName : IGet {
				SName	name;
				bool	local;

				GetByName(const SName& name, bool local);
				void refresh(const IQueryMatrix& q) override;
				std::size_t getHash() const noexcept override;
				bool operator == (const GetByName& g) const noexcept;
			};
			using Get_Up = std::unique_ptr<IGet>;

			struct Hash {
				std::size_t operator()(const IGet* g) const noexcept;
			};
			struct Equal {
				bool operator()(const IGet* g0, const IGet* g1) const noexcept;
			};
			using Cache = std::unordered_map<IGet*, Get_Up, Hash, Equal>;
			using CacheV = std::vector<Get_Up>;
			using CachePtrV = std::vector<IGet*>;
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
			const IQueryMatrix&	_source;
			bool				_initialized;

			template <class Ent>
			void _register(Ent e) const;

		public:
			QM_FixedSequence(const IQueryMatrix& q);
			const IQueryMatrix& prepareInterface();

			Mat4 getLocal(JointId id) const override;
			Mat4 getGlobal(JointId id) const override;
			Mat4 getLocal(const SName& name) const override;
			Mat4 getGlobal(const SName& name) const override;
	};
}
