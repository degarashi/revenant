#include "glx.hpp"
#include "gl_error.hpp"
#include "gl_resource.hpp"
#include "gl_program.hpp"
#include "argchecker.hpp"
#include <boost/format.hpp>

namespace rev {
	namespace {
		class TPSDupl {
			private:
				using TPList = std::vector<const TPStruct*>;
				const BlockSet& _bset;
				const TPStruct &_tTech, &_tPass;
				//! 優先度順に並べたTPStructのポインタリスト [Pass][Tech][Tech(Base0)][Tech(Base1)]...
				TPList _tpList;

				//! Techの継承元Techをリストアップ
				void _listupBlocks(TPList& dst, const TPStruct* tp) const {
					dst.push_back(tp);
					for(auto& name : tp->derive) {
						auto op = _bset.findTechPass(name);
						// 継承Techは必ずGLEffectが持っている筈
						Assert0(op);
						_listupBlocks(dst, &op.get());
					}
				}
				template <class ST, class MFunc>
				void _extractBlocks(std::vector<const ST*>& dst, const ST* attr, MFunc&& mfunc) const {
					for(auto itr=attr->derive.rbegin() ; itr!=attr->derive.rend() ; itr++) {
						auto op = mfunc(_bset, *itr);
						Assert0(op);
						auto* der = &(*op);
						_extractBlocks(dst, der, mfunc);
					}
					if(std::find(dst.begin(), dst.end(), attr) == dst.end())
						dst.push_back(attr);
				}
			public:
				TPSDupl(const BlockSet& bs, const TPStruct& tech, const TPStruct& pass):
					_bset(bs),
					_tTech(tech),
					_tPass(pass)
				{
					// 継承関係をリストアップ
					// Pass, Tech, Tech(Base0) ... Tech(BaseN) の順番
					_tpList.push_back(&_tPass);
					_listupBlocks(_tpList, &_tTech);
				}

				template <class ST, class ENT, class MFunc>
				std::vector<const ENT*> exportEntries(uint32_t blockId, MFunc&& mfunc) const {
					// 使用されるAttributeブロックを収集
					std::vector<const ST*> tmp, tmp2;
					// 配列末尾から処理をする = Pass, Tech, TechBase... の順
					for(auto itr=_tpList.rbegin() ; itr!=_tpList.rend() ; itr++) {
						const TPStruct* tp = (*itr);
						// ブロックは順方向で操作 ( Block = A,B,C ならAが優先 )
						for(auto& blk : tp->blkL) {
							if(blk.type == blockId) {
								// += 演算子でないならエントリを初期化
								if(!blk.bAdd)
									tmp.clear();
								for(auto& name : blk.name) {
									auto op = mfunc(_bset, name);
									if(!op)
										throw GLE_LogicalError((boost::format("entry block \"%1%\" not found") % name).str());
									tmp.push_back(&(*op));
								}
							}
						}
					}
					// ブロック継承展開
					for(auto& p : tmp) {
						// 既に同じブロックが登録されていたら何もしない(エントリの重複を省く)
						_extractBlocks(tmp2, p, mfunc);
					}
					// エントリ抽出: 同じ名前のエントリがあればエラー = 異なるエントリに同じ変数が存在している
					std::vector<const ENT*> ret;
					for(auto& p : tmp2) {
						for(auto& e : p->entry) {
							if(std::find_if(ret.begin(), ret.end(), [&e](const ENT* tmp){return e.name==tmp->name;}) != ret.end())
								throw GLE_LogicalError((boost::format("duplication of entry \"%1%\"") % e.name).str());
							ret.push_back(&e);
						}
					}
					return ret;
				}
				using MacroMap = TPStructR::MacroMap;
				using MacroPair = MacroMap::value_type;
				MacroMap exportMacro() const {
					MacroMap mm;
					for(auto itr=_tpList.rbegin() ; itr!=_tpList.rend() ; itr++) {
						for(auto& mc : (*itr)->mcL) {
							MacroPair mp(mc.fromStr, mc.toStr ? (*mc.toStr) : std::string());
							mm.insert(std::move(mp));
						}
					}
					return mm;
				}
				GLState_SPV exportSetting() const {
					struct Hash {
						std::size_t operator()(const GLState_SP& s) const noexcept {
							return s->getHash();
						}
					};
					struct Equal {
						bool operator()(const GLState_SP& s0, const GLState_SP& s1) const noexcept {
							return *s0 == *s1;
						}
					};
					std::unordered_set<GLState_SP, Hash, Equal> set;
					const auto ovr = [&set](GLState_SP&& sp){
						const auto itr = set.find(sp);
						if(itr != set.end())
							set.erase(itr);
						set.emplace(std::move(sp));
					};

					for(auto itr=_tpList.rbegin() ; itr!=_tpList.rend() ; itr++) {
						const TPStruct* tp = (*itr);
						// フラグ設定エントリ
						for(auto& bs : tp->bsL) {
							ovr(MakeBoolSetting(bs));
						}
						// 値設定エントリ
						for(auto& vs : tp->vsL) {
							ovr(MakeValueSetting(vs));
						}
					}
					GLState_SPV ret;
					for(auto& s : set)
						ret.emplace_back(s);
					return ret;
				}
		};
		template <class DST, class SRC>
		void OutputS(DST& dst, const SRC& src) {
			for(auto& p : src) {
				dst << *p << std::endl;
			}
		}
	}

	// ----------------- TPStructR -----------------
	UnifPool TPStructR::s_unifPool(DefaultUnifPoolSize);
	bool TPStructR::findSetting(const GLState& s) const {
		return std::find_if(_setting.cbegin(), _setting.cend(),
			[&s](const auto& s1){
				return s == *s1;
			}
		) != _setting.cend();
	}
	GLState_SPV TPStructR::CalcDiff(const TPStructR& from, const TPStructR& to) {
		// toと同じ設定がfrom側にあればスキップ
		// fromに無かったり、異なっていればエントリに加える
		GLState_SPV ret;
		for(auto& s : to._setting) {
			if(!from.findSetting(*s)) {
				ret.push_back(s);
			}
		}
		return ret;
	}
	namespace {
		struct ArgVisitor : boost::static_visitor<> {
			std::ostream&	_os;
			const ArgItem*	_arg;
			ArgVisitor(std::ostream& os, const ArgItem* arg):
				_os(os),
				_arg(arg)
			{}

			void _outputArgL() const {
				_os << GLType_::cs_typeStr[_arg->type] << ' ' << _arg->name;
			}
			template <class T, ENABLE_IF(frea::is_vector<T>{})>
			void _outputArgR(const T& t) const {
				if(T::size == 1)
					_outputArgR(t[0]);
				else {
					_os << '=' << GLType_::cs_typeStr[_arg->type] << '(';
					for(int i=0 ; i<T::size-1 ; i++)
						_os << t[i] << ',';
					_os << t[T::size-1] << ");" << std::endl;
				}
			}
			template <class T, ENABLE_IF(!frea::is_vector<T>{})>
			void _outputArgR(const T& t) const {
				_os << '=' << t << std::endl;
			}
			template <class T>
			void operator()(const T& value) const {
				_outputArgL();
				_outputArgR(value);
			}
		};
	}
	TPStructR::TPStructR(const BlockSet& bs, const TPStruct& tech, const TPStruct& pass) {
		const ShSetting* selectSh[ShType::_Num] = {};
		// PassかTechからシェーダー名を取ってくる
		for(auto& a : tech.shL)
			selectSh[a.type] = &a;
		for(auto& a : pass.shL)
			selectSh[a.type] = &a;

		// VertexとPixelシェーダは必須、Geometryは任意
		if(!selectSh[ShType::Vertex] || !selectSh[ShType::Fragment])
			throw GLE_LogicalError("no vertex or fragment shader found");

		std::stringstream ss;
		TPSDupl dupl(bs, tech, pass);
		HSh shP[ShType::_Num];
		for(int i=0 ; i<static_cast<int>(countof(selectSh)) ; i++) {
			auto* shp = selectSh[i];
			if(!shp)
				continue;
			auto s = bs.findShader(shp->shName);
			if(!s)
				throw GLE_LogicalError((boost::format("requested shader \"%1%\" not found") % shp->shName).str());
			// シェーダーバージョンを出力
			ss << "#version " << s->versionString() << std::endl;
			{
				// マクロを定義をGLSLソースに出力
				auto mc = dupl.exportMacro();
				for(auto& p : mc)
					ss << "#define " << p.first << ' ' << p.second << std::endl;
			}
			if(i==ShType::Vertex) {
				// Attribute定義は頂点シェーダの時だけ出力
				_attrL = dupl.exportEntries<AttrStruct,AttrEntry>(GLBlocktype_::attributeT,
						[](auto& bs, auto& name) -> decltype(auto) {
							return bs.findAttribute(name);
						});
				OutputS(ss, _attrL);
			}
			// それぞれ変数ブロックをGLSLソースに出力
			// :Varying
			_varyL = dupl.exportEntries<VaryStruct,VaryEntry>(GLBlocktype_::varyingT,
					[](auto& bs, auto& name) -> decltype(auto) {
						return bs.findVarying(name);
					});
			OutputS(ss, _varyL);
			// :Const
			_constL = dupl.exportEntries<ConstStruct,ConstEntry>(GLBlocktype_::constT,
					[](auto& bs, auto& name) -> decltype(auto) {
						return bs.findConst(name);
					});
			OutputS(ss, _constL);
			// :Uniform
			_unifL = dupl.exportEntries<UnifStruct,UnifEntry>(GLBlocktype_::uniformT,
					[](auto& bs, auto& name) -> decltype(auto) {
						return bs.findUniform(name);
					});
			OutputS(ss, _unifL);

			// コードブロック出力
			for(auto& cn : s->code) {
				auto code = bs.findCode(cn);
				if(!code)
					throw GLE_LogicalError((boost::format("requested code block %1% not found (in shader %2%)") % cn % s->name).str());
				ss << *code << std::endl;
			}
			{
				// シェーダー引数の型チェック
				// ユーザー引数はグローバル変数として用意
				ArgChecker acheck(shp->shName, s->args);
				for(auto& a : shp->args)
					boost::apply_visitor(acheck, a);
				acheck.finalizeCheck();
			}
			{
				// シェーダー引数の出力
				const auto* p_args = s->args.data();
				for(auto& a : shp->args) {
					boost::apply_visitor(ArgVisitor(ss, p_args++), a);
				}
			}
			OutputCommentBlock(ss, s->name);
			// 関数名はmain()に書き換え
			ss << "void main() " << s->getShaderString() << std::endl;
	#ifdef DEBUG
			std::cout << ss.str();
			std::cout.flush();
	#endif
			shP[i] = mgr_gl.makeShader(static_cast<ShType::e>(i), ss.str());

			ss.str("");
			ss.clear();
		}
		// シェーダーのリンク処理
		_prog = mgr_gl.makeProgram(shP[0], shP[1], shP[2]);
		// OpenGLステート設定リストを形成
		_setting = dupl.exportSetting();
	}
	void TPStructR::ts_onDeviceLost() {
		D_Assert0(_bInit);
		_bInit = false;
		// OpenGLのリソースが絡んでる変数を消去
		_vattr.clear();
		_noDefValue.clear();

		for(auto& p : _defaultValue)
			s_unifPool.destroy(p.second);
		_defaultValue.clear();
	}
	namespace {
		struct Visitor : boost::static_visitor<> {
			GLuint		pgId;
			GLint		uniId;
			UniMap		result;
			const IEffect&	glx;
			UnifPool&	pool;
			Visitor(const IEffect& g, UnifPool& p): glx(g), pool(p) {}

			bool setKey(const std::string& key) {
				uniId = GL.glGetUniformLocation(pgId, key.c_str());
				// ここでキーが見つからない = uniformブロックで宣言されているがGLSLコードで使われない場合なのでエラーではない
				D_Expect(uniId>=0, "Uniform argument \"%s\" not found", key.c_str());
				return uniId >= 0;
			}
			template <class T, ENABLE_IF(frea::is_vector<T>{})>
			void operator()(const T& t) {
				if(uniId >= 0) {
					auto* buff = MakeUniformTokenBuffer(result, pool, uniId);
					glx._makeUniformToken(*buff, uniId, &t, 1, false);
				}
			}
		};
	}
	void TPStructR::ts_onDeviceReset(const IEffect& e) {
		D_Assert0(!_bInit);
		_bInit = true;
		auto& prog = *_prog;
		prog.onDeviceReset();

		struct UniqueChk {
			GLint		id;
			VSem::e		sem;
			const char*	name;
			bool operator == (const UniqueChk& u) const noexcept {
				return id == u.id;
			}
			bool operator < (const UniqueChk& u) const noexcept {
				return id < u.id;
			}
		};
		std::vector<UniqueChk> uc;
		// 頂点セマンティクス対応リストを生成
		for(auto& p : _attrL) {
			const char* name = p->name.c_str();
			if(const auto at = prog.getAttribId(name)) {
				const auto sem = static_cast<VSem::e>(p->sem);
				VSemAttr a;
				a.sem = VSemantic {
					sem,
					(p->index) ? *p->index : 0
				};
				a.attrId = *at;
				_vattr.emplace_back(a);
				uc.emplace_back(UniqueChk{a.attrId, sem, name});
			} else {
				// 該当する変数が見付からない旨の警告を出す(もしかしたらシェーダー内で使ってないだけかもしれない)
				D_Expect(false, R"(vertex attribute "%s[%s]" not found)", name, GLSem_::cs_typeStr[p->sem]);
			}
		}
		{
			// セマンティクスの重複チェック
			std::sort(uc.begin(), uc.end());
			const auto itr = std::unique(uc.begin(), uc.end());
			if(itr != uc.end()) {
				throw GLE_LogicalError(
					(
						 boost::format("duplication of vertex semantics \"%1% : %2%\"")
						 % itr->name
						 % GLSem_::cs_typeStr[itr->sem]
					).str()
				);
			}
		}

		// Uniform変数にデフォルト値がセットしてある物をリストアップ
		Visitor visitor(e, s_unifPool);
		visitor.pgId = _prog->getProgramId();
		for(const auto* p : _unifL) {
			if(visitor.setKey(p->name)) {
				const auto& defVal = p->defaultValue;
				if(defVal) {
					// 変数名をIdに変換
					boost::apply_visitor(visitor, *defVal);
				} else
					_noDefValue.insert(visitor.uniId);
			}
		}
		_defaultValue.swap(visitor.result);
	}

	void TPStructR::applySetting() const {
		for(auto& st : _setting)
			st->apply();
	}
	const UniMap& TPStructR::getUniformDefault() const noexcept {
		return _defaultValue;
	}
	const TPStructR::UniIdSet& TPStructR::getRequiredUniformEntries() const noexcept {
		return _noDefValue;
	}
	const HProg& TPStructR::getProgram() const noexcept {
		return _prog;
	}
	const VSemAttrV& TPStructR::getVAttr() const noexcept {
		return _vattr;
	}
}
