#pragma once
#include "handle.hpp"
#include "lcv.hpp"
#include "flagptr.hpp"
#include "sdl_tls.hpp"
#include <vector>

namespace rev {
	using Priority = uint32_t;
	constexpr static Priority DefaultPriority = std::numeric_limits<Priority>::max() / 2;

	using GroupTypeId = uint32_t;		//!< Object種別Id
	using ObjName = std::string;
	using ObjTypeId = uint32_t;			//!< Object種別Id
	using ObjTypeId_OP = spi::Optional<ObjTypeId>;
	using GMessageStr = std::string;
	using CBFindGroup = std::function<bool (const HGroup&)>;
	using CBUpdProc = std::function<void (const HObj&)>;
	class LCValue;

	//! ゲームオブジェクト基底インタフェース
	class Object : public Resource {
		private:
			bool _bDestroy;
		public:
			Object();
			virtual ~Object() {}
			virtual Priority getPriority() const;
			bool isDead() const;
			bool onUpdateBase();

			//! UpdGroupに登録された時に呼ばれる
			virtual void onConnected(const HGroup& hGroup);
			//! UpdGroupから削除される時に呼ばれる
			virtual void onDisconnected(const HGroup& hGroup);
			//! 各Objが実装するアップデート処理
			virtual void onUpdate(bool bFirst);

			virtual bool isNode() const = 0;
			virtual bool hasLuaState() const;
			virtual void destroy();
			//! 一意に割り振られるオブジェクトの識別IDを取得
			virtual ObjTypeId getTypeId() const = 0;
			// ---- ObjectGroup用メソッド ----
			virtual void enumGroup(const CBFindGroup& cb, GroupTypeId id, int depth) const;
			//! 特定の優先度範囲のオブジェクトを処理
			virtual void proc(const CBUpdProc& p, bool bRecursive,
								Priority prioBegin=std::numeric_limits<Priority>::lowest(),
								Priority prioEnd=std::numeric_limits<Priority>::max());
			// ---- Message用メソッド ----
			virtual LCValue recvMsg(const GMessageStr& msg, const LCValue& arg=LCValue());
			// ---------- Object/Scene用メソッド ----------
			virtual void onDraw(IEffect& e) const;
			// ---------- Scene用メソッド ----------
			virtual void onDown(ObjTypeId_OP prevId, const LCValue& arg);
			virtual bool onPause();
			virtual void onStop();
			virtual void onResume();
			virtual void onReStart();

			const char* getResourceName() const noexcept override;
	};

	// ---- Objectの固有Idを生成 ----
	namespace detail {
		template <class Tag>
		struct ObjectIdT {
			static ObjTypeId GenerateObjTypeId() {
				static ObjTypeId s_id(0);
				return s_id++;
			}
		};
		struct IdDummy;
	}
	// 型Tはdetail::ObjectIdTにて新しいIdを生成する為に使用
	template <class T, class Tag>
	struct ObjectIdT {
		const static ObjTypeId Id;
	};
	template <class T, class Tag>
	const ObjTypeId ObjectIdT<T, Tag>::Id(detail::ObjectIdT<Tag>::GenerateObjTypeId());

	namespace idtag {
		struct Object {};
		struct Group {};
		struct DrawGroup {};
	}

	class U_Object : public Object, public ObjectIdT<detail::IdDummy, idtag::Object> {
		private:
			using IdT = ObjectIdT<detail::IdDummy, idtag::Object>;
		public:
			bool isNode() const noexcept override;
			ObjTypeId getTypeId() const noexcept override;
	};

	namespace detail {
		//! オブジェクト基底
		/*!
			UpdatableなオブジェクトやDrawableなオブジェクトはこれから派生して作る
			Sceneと共用
		*/
		template <class T, class Base>
		class ObjectT : public Base, public ::rev::ObjectIdT<T, ::rev::idtag::Object> {
			private:
				using ThisT = ObjectT<T,Base>;
				using IdT = ::rev::ObjectIdT<T, ::rev::idtag::Object>;
			protected:
				struct State {
					virtual ~State() {}
					virtual ObjTypeId getStateId() const = 0;
					virtual void onUpdate(T& self) {
						self.Base::onUpdate(false);
					}
					virtual LCValue recvMsg(T& self, const GMessageStr& msg, const LCValue& arg) {
						return self.Base::recvMsg(msg, arg);
					}
					// onEnterとonExitは継承しない
					virtual void onEnter(T& /*self*/, ObjTypeId_OP /*prevId*/) {}
					virtual void onExit(T& /*self*/, ObjTypeId_OP /*nextId*/) {}
					virtual void onConnected(T& self, const HGroup& hGroup) {
						self.Base::onConnected(hGroup);
					}
					virtual void onDisconnected(T& self, const HGroup& hGroup) {
						self.Base::onDisconnected(hGroup);
					}
					// --------- Scene用メソッド ---------
					virtual void onDraw(const T& self, IEffect& e) const {
						self.Base::onDraw(e);
					}
					virtual void onDown(T& self, ObjTypeId_OP prevId, const LCValue& arg) {
						self.Base::onDown(prevId, arg);
					}
					virtual bool onPause(T& self) {
						return self.Base::onPause();
					}
					virtual void onStop(T& self) {
						self.Base::onStop();
					}
					virtual void onResume(T& self) {
						self.Base::onResume();
					}
					virtual void onReStart(T& self) {
						self.Base::onReStart();
					}
				};
				struct tagObjectState {};
				template <class ST, class D=State>
				struct StateT : D {
					StateT() = default;
					StateT(const D& d):
						D(d)
					{}
					StateT(D&& d):
						D(std::move(d))
					{}
					using IdT = ::rev::ObjectIdT<ST, tagObjectState>;
					const static IdT	s_idt;
					ObjTypeId getStateId() const override {
						return GetStateId();
					}
					static ObjTypeId GetStateId() {
						return IdT::Id;
					}
				};
				using FPState = FlagPtr<State>;

				static StateT<void> _nullState;
				static FPState _GetNullState() {
					return FPState(&_nullState, false);
				}

			private:
				bool	_bSwState = false;
				FPState	_state = FPState(&_nullState, false),
						_nextState;
				bool _isNullState() const {
					return _state.get() == &_nullState;
				}
				void _setNullState() {
					setStateUse(&_nullState);
				}
			protected:
				using Base::Base;

				// ステートをNewするためのヘルパー関数
				template <class ST, class... Args>
				void setStateNew(Args&&... args) {
					setState(FPState(new ST(std::forward<Args>(args)...), true));
				}
				// ステートを再利用するヘルパー関数
				template <class ST>
				void setStateUse(ST* st) {
					setState(FPState(st, false));
				}
				bool isNode() const override {
					return false;
				}
				ObjTypeId getTypeId() const override {
					return IdT::Id;
				}
				T& getRef() { return *reinterpret_cast<T*>(this); }
				const T& getRef() const { return *reinterpret_cast<const T*>(this); }
				void setState(FPState&& st) {
					// もし既に有効なステートがセットされていたら無視 | nullステートは常に適用
					if(!st.get() || !_bSwState) {
						bool bNull = _isNullState();
						_bSwState = true;
						std::swap(_nextState, st);
						if(bNull)
							_doSwitchState();
					} else {
						// ステートを2度以上セットするのはロジックが何処かおかしいと思われる
						Expect(false, "state set twice");
					}
				}
				//! 前後をdoSwitchStateで挟む
				/*! not void バージョン */
				template <class CB>
				auto _callWithSwitchState(CB&& cb, std::false_type) {
					// 前後をdoSwitchStateで挟む
					_doSwitchState();
					auto ret = std::forward<CB>(cb)();
					_doSwitchState();
					return ret;
				}
				//! 前後をdoSwitchStateで挟む
				/*! void バージョン */
				template <class CB>
				void _callWithSwitchState(CB&& cb, std::true_type) {
					_doSwitchState();
					std::forward<CB>(cb)();
					_doSwitchState();
				}
				template <class CB>
				auto _callWithSwitchState(CB&& cb) {
					// DebugBuild & NullState時に警告を出す
					D_Expect(_state.get() != &_nullState, "null state detected");
					using Rt = typename std::is_same<void, decltype(std::forward<CB>(cb)())>::type;
					return _callWithSwitchState(std::forward<CB>(cb), Rt());
				}
				void _doSwitchState() {
					if(_bSwState) {
						_bSwState = false;
						ObjTypeId_OP prevId;
						// 現在のステートのonExitを呼ぶ
						if(_state.get()) {
							ObjTypeId_OP nextId;
							if(_nextState.get())
								nextId = _nextState->getStateId();
							_state->onExit(getRef(), nextId);
							prevId = _state->getStateId();
						}
						std::swap(_state, _nextState);
						_nextState.reset();
						// 次のステートのonEnterを呼ぶ
						if(_state.get())
							_state->onEnter(getRef(), prevId);
					}
				}
				State* getState() {
					return _state.get();
				}
			public:
				Priority getPriority() const override {
					return 0x0000;
				}
				ObjTypeId getStateId() const {
					return _state->getStateId();
				}
				void destroy() override {
					if(!Base::isDead()) {
						Base::destroy();
						// 終端ステートに移行
						_setNullState();
					}
				}
				//! 毎フレームの描画 (Scene用)
				void onDraw(IEffect& e) const override {
					// ステート遷移はナシ
					_state->onDraw(getRef(), e);
				}
				//! 上の層のシーンから抜ける時に戻り値を渡す (Scene用)
				void onDown(ObjTypeId_OP prevId, const LCValue& arg) override {
					_callWithSwitchState([&](){ _state->onDown(getRef(), prevId, arg); });
				}
				// ----------- 以下はStateのアダプタメソッド -----------
				void onUpdate(bool /*bFirst*/) override {
					_callWithSwitchState([&](){ return _state->onUpdate(getRef()); });
				}
				LCValue recvMsg(const GMessageStr& msg, const LCValue& arg) override {
					return _callWithSwitchState([&](){ return _state->recvMsg(getRef(), msg, arg); });
				}
				//! Updaterノードツリーに追加された時に呼ばれる
				/*! DrawGroupに登録された時は呼ばれない( */
				void onConnected(const HGroup& hGroup) override {
					return _callWithSwitchState([&](){ return _state->onConnected(getRef(), hGroup); });
				}
				void onDisconnected(const HGroup& hGroup) override {
					return _callWithSwitchState([&](){ return _state->onDisconnected(getRef(), hGroup); });
				}
		};
		template <class T, class Base>
		typename ObjectT<T, Base>::template StateT<void> ObjectT<T, Base>::_nullState;

	}
	template <class T, class Base>
	template <class ST, class D>
	const ObjectIdT<ST,typename detail::ObjectT<T,Base>::tagObjectState> detail::ObjectT<T,Base>::StateT<ST,D>::s_idt;

	template <class T, class Base=Object>
	class ObjectT : public detail::ObjectT<T, Base> {
		using base = detail::ObjectT<T, Base>;
		public:
			using base::base;
	};
}
DEF_LUAIMPORT(rev::Object)
DEF_LUAIMPORT(rev::U_Object)
