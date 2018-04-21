#include "gltf/exceptions.hpp"
#include "lubee/output.hpp"
#include <cstring>

namespace rev {
	namespace gltf {
		namespace {
			struct RowColumn {
				int				row,
								column,
								lastRowOfs;
				std::string		fragment;
				RowColumn(const char* str, const int ofs, const int fragLen) {
					const char *p = str,
								*pe = p + ofs;
					int ro = 1,
						co = 1,
						lastRow = 0;
					while(p != pe) {
						++co;
						if(*p == '\n') {
							++ro;
							co = 1;
							lastRow = p-str+1;
						}
						++p;
					}

					row = ro;
					column = co;
					lastRowOfs = lastRow;

					p = str + lastRow;
					fragment.assign(p, p+std::min<int>(std::strlen(p), fragLen));
				}
			};
		}
		SyntaxError::SyntaxError(const char* src, const int offset, const char* errmsg):
			ParseError("")
		{
			const RowColumn rc(src, offset, 128);
			static_cast<ParseError&>(*this) = 
				ParseError(
					lubee::log::MakeMessage(
						"SyntaxError: at[%d:%d] %s\n%s",
						rc.row,
						rc.column,
						rc.fragment.c_str(),
						errmsg
					)
				);
		}
	}
}
