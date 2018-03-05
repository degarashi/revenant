#pragma once
#include "object_if.hpp"
#include "object_id.hpp"
#include "flagptr.hpp"

namespace rev {
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
						// LuaのonUpdate()があるなら既に呼ばれてる筈なのでフラグはfalse
						self.Base::onUpdate(false);
					}
					virtual bool recvMsg(T& self, LCValue& dst, const GMessageStr& msg, const LCValue& arg) {
						return self.Base::recvMsg(dst, msg, arg);
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
				template <class Tag, class D=State>
				struct StateT : D {
					StateT() = default;
					StateT(const D& d):
						D(d)
					{}
					StateT(D&& d):
						D(std::move(d))
					{}
					ObjTypeId getStateId() const override {
						return GetStateId();
					}
					using IdT = ::rev::ObjectIdT<Tag, tagObjectState>;
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
				bool isNode() const noexcept override {
					return false;
				}
				ObjTypeId getTypeId() const override {
					return IdT::Id;
				}
				static ObjTypeId GetTypeId() {
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
				void onUpdate(bool /*execLua*/) override {
					_callWithSwitchState([&](){ return _state->onUpdate(getRef()); });
				}
				bool recvMsg(LCValue& dst, const GMessageStr& msg, const LCValue& arg) override {
					return _callWithSwitchState([&](){ return _state->recvMsg(getRef(), dst, msg, arg); });
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
	template <class T, class Base=IObject>
	class ObjectT : public detail::ObjectT<T, Base> {
		using base = detail::ObjectT<T, Base>;
		public:
			using base::base;
	};
	#define DefineObject(name, base) \
		class name : public ::rev::ObjectT<name, base> { \
			private: \
				using base_t = ::rev::ObjectT<name, base>; \
			public: \
				using base_t::base_t; \
		};
}
