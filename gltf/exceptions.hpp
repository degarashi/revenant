#pragma once
#include <stdexcept>

namespace rev {
	namespace gltf {
		struct ParseError : std::invalid_argument {
			using invalid_argument::invalid_argument;
		};
		// Json文法エラー
		struct SyntaxError : ParseError {
			SyntaxError(const char* src, int offset, const char* errmsg);
		};
		// 必須値が定義されていない
		struct LackOfPrerequisite : ParseError {
			using ParseError::ParseError;
		};
		// 同じIDが複数箇所に記述されている
		struct DuplicateID : ParseError {
			using ParseError::ParseError;
		};
		// 定義されたデータに不整合がある
		struct InvalidProperty : ParseError {
			using ParseError::ParseError;
		};
		// IDタグとして不正な文字列
		struct InvalidID : ParseError {
			using ParseError::ParseError;
		};
		// IDタグが解決できなかった
		struct UnknownID : ParseError {
			using ParseError::ParseError;
		};
		// 不正な範囲の値
		struct OutOfRange : ParseError {
			using ParseError::ParseError;
		};
		// Enum値が不正
		struct InvalidEnum : ParseError {
			using ParseError::ParseError;
		};

		// 余計なプロパティ値が定義されている
		struct UnnecessaryProperty : ParseError {
			using ParseError::ParseError;
		};
		// glTFでは規定されているがライブラリがまだ対応していない
		struct NotSupported : ParseError {
			using ParseError::ParseError;
		};
	}
}
