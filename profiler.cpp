#include "profiler.hpp"

namespace rev {
	thread_local prof::Profiler profiler;
	namespace prof {
		const Duration Parameter::DefaultInterval = std::chrono::seconds(1);
		Parameter::Parameter() {
			setInterval(DefaultInterval);
			setAccum(0);
		}
		bool Parameter::_refresh(Accum::value_t& a, Accum*) const {
			getInterval();
			++a;
			return true;
		}
		SpinLock<Parameter> g_param;

		// -------------------- Block --------------------
		Block::Block(const Name& name):
			name(name)
		{}
		Unit Block::getLowerTime() const {
			Unit sum(0);
			iterateChild([&sum](const auto& nd){
				sum += nd->hist.tAccum;
				return true;
			});
			return sum;
		}
		Unit Block::getAverageTime(const bool omitLower) const {
			if(omitLower)
				return (hist.tAccum - getLowerTime()) / hist.nCalled;
			return hist.getAverageTime();
		}
		// -------------------- History --------------------
		History::History():
			nCalled(0),
			tMax(std::numeric_limits<Unit::rep>::lowest()),
			tMin(std::numeric_limits<Unit::rep>::max()),
			tAccum(0)
		{}
		void History::addTime(Unit t) {
			++nCalled;
			tMax = std::max(tMax, t);
			tMin = std::min(tMin, t);
			tAccum += t;
		}
		Unit History::getAverageTime() const {
			return tAccum / nCalled;
		}
		// -------------------- Profiler::Scope --------------------
		Profiler::Scope::Scope(const Name& name):
			_bValid(true),
			_name(name)
		{}
		Profiler::Scope::Scope(Scope&& b):
			_bValid(b._bValid),
			_name(std::move(b._name))
		{
			b._bValid = false;
		}
		Profiler::Scope::~Scope() {
			if(_bValid)
				profiler.endBlock(_name);
		}
		// -------------------- Profiler --------------------
		const std::size_t Profiler::DefaultMaxLayer = 8;
		const Name Profiler::DefaultRootName = "Root";

		void Profiler::initialize(const Name& rootName, const std::size_t ml) {
			_param_accum = g_param.lockC()->getAccum();
			_rootName = rootName;
			// 計測中だった場合は警告を出す
			Expect(!accumulating(), "reinitializing when accumulating");
			Assert(ml > 0, "invalid maxLayer");
			_maxLayer = ml;
			clear();
		}
		Profiler::Profiler() {
			initialize();
		}
		void Profiler::clear() {
			_closeAllBlocks();
			_intervalInfo.clear();
			_intervalInfo.current().tmBegin = Clock::now();
			_curBlock = nullptr;
			_prepareRootNode();
		}
		void Profiler::_closeAllBlocks() {
			auto* cur = _curBlock;
			while(cur) {
				endBlock(cur->name);
				cur = cur->getParent().get();
			}
			_curBlock = nullptr;
		}
		void Profiler::_prepareRootNode() {
			beginBlock(_rootName);
		}
		bool Profiler::checkIntervalSwitch() {
			// ルート以外のブロックを全て閉じてない場合は警告を出す
			Expect(!accumulating(), "profiler block leaked");
			bool b;
			{
				auto lk = g_param.lockC();
				// インターバル時間が変更されていたら履歴をクリア
				const auto ac = lk->getAccum();
				if(ac != _param_accum) {
					_param_accum = ac;
					clear();
					return false;
				}
				b = (Clock::now() - getCurrent().tmBegin) >= lk->getInterval();
			}
			_closeAllBlocks();
			if(b) {
				_intervalInfo.advance_clear();
				_intervalInfo.current().tmBegin = Clock::now();
				_curBlock = nullptr;
			}
			_prepareRootNode();
			return b;
		}
		bool Profiler::accumulating() const {
			// ルートノードの分を加味
			return _tmBegin.size() > 1;
		}
		void Profiler::beginBlock(const Name& name) {
			// 最大レイヤー深度を超えた分のブロック累積処理はしない
			const auto n = _tmBegin.size();
			auto& ci = _intervalInfo.current();
			if(n < _maxLayer) {
				// 子ノードに同じ名前のノードが無いか確認
				auto& cur = _curBlock;
				Block::SP node;
				if(cur) {
					cur->iterateChild([&node, &name](auto& nd){
						if(nd->name == name) {
							node = nd->shared_from_this();
							return false;
						}
						return true;
					});
				}
				if(!node) {
					Assert0(ci.root || n==0);
					if(!cur && ci.root && ci.root->name == name)
						cur = ci.root.get();
					else {
						// 新たにノードを作成
						const auto blk = std::make_shared<Block>(name);
						if(cur) {
							// 現在ノードの子に追加
							cur->addChild(blk);
						} else {
							// 新たにルートノードとして設定
							ci.root = blk;
						}
						cur = blk.get();
					}
				} else {
					cur = node.get();
				}
			}
			// エントリを確保
			ci.byName[name];
			// 現在のレイヤーの開始時刻を記録
			_tmBegin.emplace_back(Clock::now());
		}
		Profiler::Scope Profiler::beginScope(const Name& name) {
			beginBlock(name);
			return Scope(name);
		}
		void Profiler::endBlock(const Name& name) {
			// endBlockの呼び過ぎはエラー
			Assert0(!_tmBegin.empty());
			auto& cur = _curBlock;

			// かかった時間を加算
			const Unit dur = std::chrono::duration_cast<Unit>(Clock::now() - _tmBegin.back());
			{
				const auto n = _tmBegin.size();
				if(n <= _maxLayer) {
					cur->hist.addTime(dur);
					// ネストコールが崩れた時にエラーを出す
					Assert0(name==cur->name);
					// ポインタを親に移す
					cur = cur->getParent().get();
				}
			}
			auto& ci = _intervalInfo.current();
			Assert0(ci.byName.count(name)==1);
			ci.byName.at(name).addTime(dur);
			_tmBegin.pop_back();
		}
		const Profiler::IntervalInfo& Profiler::getPrev() const {
			return _intervalInfo.prev();
		}
		const Profiler::IntervalInfo& Profiler::getCurrent() const {
			return _intervalInfo.current();
		}
		Profiler::Scope Profiler::operator()(const Name& name) {
			return beginScope(name);
		}
	}
}
