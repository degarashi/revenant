//! リアルタイムプロファイラ
#pragma once
#include "dataswitch.hpp"
#include "clock.hpp"
#include "spine/src/treenode.hpp"
#include "spine/src/rflag.hpp"

#ifdef PROFILER_ENABLED
	#define RevBeginProfile(name)	::rev::profiler.beginBlock(#name)
	#define	RevEndProfile(name)		::rev::profiler.endBlock(#name)
	#define RevProfile(name)		const auto name##__LINE__ = ::rev::profiler(#name)
#else
	#define RevBeginProfile(name)
	#define	RevEndProfile(name)
	#define RevProfile(name)
#endif

namespace rev {
	namespace prof {
		struct Parameter {
			const static Duration DefaultInterval;
			//!< 履歴を更新する間隔
			#define SEQ \
				((Interval)(Duration)) \
				((Accum)(uint32_t)(Interval))
			RFLAG_DEFINE(Parameter, SEQ)
			RFLAG_GETMETHOD_DEFINE(SEQ)
			RFLAG_SETMETHOD_DEFINE(SEQ)
			RFLAG_SETMETHOD(Accum)

			Parameter();
			#undef SEQ
		};
		void SetInterval(Duration dur);

		using Name = const char*;
		struct History {
			uint_fast32_t		nCalled;	//!< 呼び出し回数
			Duration			tMax,		//!< 最高値
								tMin,		//!< 最低値
								tAccum;		//!< 累積値
			History();
			void addTime(Duration t);
			Duration getAverageTime() const;
		};
		//! プロファイラブロック
		struct Block : spi::TreeNode<Block> {
			Name			name;				//!< ブロック名
			History			hist;

			Block(const Name& name);
			Duration getLowerTime() const;			//!< 下層にかかった時間
			Duration getAverageTime(bool omitLower) const;
		};
		using BlockSP = std::shared_ptr<Block>;

		class Profiler {
			public:
				//! 1インターバル間に集計される情報
				struct IntervalInfo {
					using ByName = std::unordered_map<Name, History>;

					Timepoint	tmBegin;			//!< インターバル開始時刻
					BlockSP		root;				//!< ツリー構造ルート
					ByName		byName;				//!< 名前ブロック毎の集計(最大レイヤー数を超えた分も含める)
				};
			private:
				using IntervalInfoSW = DataSwitcher<IntervalInfo>;
				IntervalInfoSW	_intervalInfo;

				using TPStk = std::vector<Timepoint>;
				Name			_rootName;
				uint32_t		_param_accum;
				Block*			_curBlock;			//!< 現在計測中のブロック
				std::size_t		_maxLayer;			//!< 最大ツリー深度
				TPStk			_tmBegin;			//!< レイヤー毎の計測開始した時刻
				//! 閉じられていないブロックを閉じる(ルートノード以外)
				void _closeAllBlocks();
				void _prepareRootNode();

				//! スコープを抜けると同時にブロックを閉じる
				class Scope {
					private:
						bool	_bValid;
						Name	_name;
						friend class Profiler;
						Scope(const Name& name);
					public:
						Scope(Scope&& b);
						~Scope();
				};
			public:
				const static Name DefaultRootName;
				const static std::size_t DefaultMaxLayer;

				Profiler();
				//! 任意のルートノード名とレイヤー数で再初期化
				void initialize(
					const Name& rootName = DefaultRootName,
					const std::size_t ml=DefaultMaxLayer
				);
				//! 何かしらのブロックを開いているか = 計測途中か
				bool accumulating() const;
				//! フレーム間の区切りに呼ぶ
				/*! 必要に応じてインターバル切り替え */
				bool checkIntervalSwitch();
				void clear();
				void beginBlock(const Name& name);
				void endBlock(const Name& name);
				Scope beginScope(const Name& name);
				Scope operator()(const Name& name);
				//! 同じ名前のブロックを合算したものを取得(前のインターバル)
				const IntervalInfo& getPrev() const;
				const IntervalInfo& getCurrent() const;
		};
	}
	// スレッド毎に集計(=結果をスレッド毎に取り出す)
	extern thread_local prof::Profiler profiler;
}
