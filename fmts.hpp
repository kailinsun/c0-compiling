#include "fmt/core.h"
#include "tokenizer/tokenizer.h"
#include "analyser/analyser.h"

namespace fmt {
	template<>
	struct formatter<miniplc0::ErrorCode> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const miniplc0::ErrorCode &p, FormatContext &ctx) {
			std::string name;
			switch (p) {
                case miniplc0::ErrNoError:
                    name = "No error.";
                    break;
                case miniplc0::ErrStreamError:
                    name = "Stream error.";
                    break;
                case miniplc0::ErrEOF:
                    name = "EOF.";
                    break;
                case miniplc0::ErrInvalidInput:
                    name = "The input is invalid.";
                    break;
                case miniplc0::ErrInvalidIdentifier:
                    name = "Identifier is invalid.";
                    break;
                case miniplc0::ErrIntegerOverflow:
                    name = "The integer is overflow.";
                    break;
                case miniplc0::ErrNeedIdentifier:
                    name = "Need an identifier here.";
                    break;
                case miniplc0::ErrConstantNeedValue:
                    name = "The constant need a value to initialize.";
                    break;
                case miniplc0::ErrNoSemicolon:
                    name = "Zai? Wei shen me bu xie fen hao.";
                    break;
                case miniplc0::ErrNoComma:
                    name = "Zai? Wei shen me bu xie dou hao.";
                    break;
                case miniplc0::ErrNoLeftBracket:
                    name = "Zai? Wei shen me bu xie zuo kuo hao.";
                    break;
			    case miniplc0::ErrNoRightBracket:
                    name = "Zai? Wei shen me bu xie you kuo hao.";
                    break;
                case miniplc0::ErrNoLeftBrace:
                    name = "Zai? Wei shen me bu xie zuo dadada kuo hao.";
                    break;
                case miniplc0::ErrNoRightBrace:
                    name = "Zai? Wei shen me bu xie you dadada kuo hao.";
                    break;
                case miniplc0::ErrInvalidVariableDeclaration:
                    name = "The variable declaration is invalid.";
                    break;
                case miniplc0::ErrIncompleteExpression:
                    name = "The expression is incomplete.";
                    break;
                case miniplc0::ErrIncompleteStatement:
                    name = "The statement is incomplete.";
                    break;
                case miniplc0::ErrIncompleteCondition:
                    name = "The condition is incomplete.";
                    break;
                case miniplc0::ErrInvalidFunctionDefinition:
                    name = "The function definition is invalid.";
                    break;
                case miniplc0::ErrNeedMainFunction:
                    name = "The program needs main function.";
                    break;
			    case miniplc0::ErrIncompleteFunctionCall:
                    name = "The function call is incomplete.";
                    break;
                case miniplc0::ErrNeedReturnValue:
                    name = "The function need return value.";
                    break;
                case miniplc0::ErrNoNeedReturnValue:
                    name = "The function does not have return value.";
                    break;
                case miniplc0::ErrNotDeclared:
                    name = "The variable or constant must be declared before being used.";
                    break;
                case miniplc0::ErrAssignToConstant:
                    name = "Trying to assign value to a constant.";
                    break;
                case miniplc0::ErrDuplicateDeclaration:
                    name = "The variable or constant has been declared.";
                    break;
                case miniplc0::ErrNotInitialized:
                    name = "The variable has not been initialized.";
                    break;
                case miniplc0::ErrInvalidAssignment:
                    name = "The assignment statement is invalid.";
                    break;
                case miniplc0::ErrInvalidPrint:
                    name = "The output statement is invalid.";
                    break;
                case miniplc0::ErrInvalidOperator:
                    name = "The operator is invalid.";
                    break;
                case miniplc0::ErrInvalidIntegerLiteral:
                    name = "The integer-literal is invalid.";
                    break;
                case miniplc0::ErrAnnotationUnmatched:
                    name = "The annotation is unmatched.";
                    break;
			}
			return format_to(ctx.out(), name);
		}
	};

	template<>
	struct formatter<miniplc0::CompilationError> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const miniplc0::CompilationError &p, FormatContext &ctx) {
			return format_to(ctx.out(), "Line: {} Column: {} Error: {}", p.GetPos().first, p.GetPos().second, p.GetCode());
		}
	};
}

namespace fmt {
	template<>
	struct formatter<miniplc0::Token> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const miniplc0::Token &p, FormatContext &ctx) {
			return format_to(ctx.out(),
				"Line: {} Column: {} Type: {} Value: {}",
				p.GetStartPos().first, p.GetStartPos().second, p.GetType(), p.GetValueString());
		}
	};

	template<>
	struct formatter<miniplc0::TokenType> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const miniplc0::TokenType &p, FormatContext &ctx) {
			std::string name;
			switch (p) {
                case miniplc0::NULL_TOKEN:
                    name = "NullToken";
                    break;
                case miniplc0::IDENTIFIER:
                    name = "Identifier";
                    break;
                case miniplc0::INT_DECIMAL:
                    name = "Integer_Decimal";
                    break;
                case miniplc0::INT_HEXADECIMAL:
                    name = "Integer_Hexadecimal";
                    break;
                case miniplc0::INT_LITERAL:
                    name = "Integer-Literal";
                    break;

                case miniplc0::CONST:
                    name = "Const";
                    break;
                case miniplc0::VOID:
                    name = "Void";
                    break;
                case miniplc0::INT:
                    name = "Integer";
                    break;
                case miniplc0::CHAR:
                    name = "Char";
                    break;
                case miniplc0::DOUBLE:
                    name = "Double";
                    break;
                case miniplc0::STRUCT:
                    name = "Struct";
                    break;
                case miniplc0::IF:
                    name = "If";
                    break;
                case miniplc0::ELSE:
                    name = "Else";
                    break;
                case miniplc0::SWITCH:
                    name = "Switch";
                    break;
                case miniplc0::CASE:
                    name = "Case";
                    break;
                case miniplc0::DEFAULT:
                    name = "Default";
                    break;
                case miniplc0::WHILE:
                    name = "While";
                    break;
                case miniplc0::FOR:
                    name = "For";
                    break;
                case miniplc0::DO:
                    name = "Do";
                    break;
                case miniplc0::RETURN:
                    name = "Return";
                    break;
                case miniplc0::BREAK:
                    name = "Break";
                    break;
                case miniplc0::CONTINUE:
                    name = "Continue";
                    break;
                case miniplc0::PRINT:
                    name = "Print";
                    break;
                case miniplc0::SCAN:
                    name = "Scan";
                    break;

                case miniplc0::PLUS_SIGN:
                    name = "PlusSign";
                    break;
                case miniplc0::MINUS_SIGN:
                    name = "MinusSign";
                    break;
                case miniplc0::MULTIPLICATION_SIGN:
                    name = "MultiplicationSign";
                    break;
                case miniplc0::DIVISION_SIGN:
                    name = "DivisionSign";
                    break;
                case miniplc0::EQUAL_SIGN:
                    name = "EqualSign";
                    break;
                case miniplc0::SEMICOLON:
                    name = "Semicolon";
                    break;
                case miniplc0::COMMA:
                    name = "Comma";
                    break;
                case miniplc0::LEFT_BRACKET:
                    name = "LeftBracket";
                    break;
                case miniplc0::RIGHT_BRACKET:
                    name = "RightBracket";
                    break;
			    case miniplc0::LEFT_BRACE:
                    name = "LeftBrace";
                    break;
                case miniplc0::RIGHT_BRACE:
                    name = "RightBrace";
                    break;
                case miniplc0::ASSIGNMENT_SIGN:
                    name = "AssignSign";
                    break;
                case miniplc0::LESS_SIGN:
                    name = "LessSign";
                    break;
                case miniplc0::LESS_EQUAL_SIGN:
                    name = "LessEqualSign";
                    break;
                case miniplc0::GREATER_SIGN:
                    name = "GreaterSign";
                    break;
                case miniplc0::GREATER_EQUAL_SIGN:
                    name = "GreaterEqualSign";
                    break;
                case miniplc0::NOT_EQUAL_SIGN:
                    name = "NotEqualSign";
                    break;
                case miniplc0::EXCLAMATION_SIGN:
                    name = "ExclamationSign";
                    break;
			}
			return format_to(ctx.out(), name);
		}
	};
}

namespace fmt {
	template<>
	struct formatter<miniplc0::Operation> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const miniplc0::Operation &p, FormatContext &ctx) {
			std::string name;
			switch (p) {
                case miniplc0::NOP:
                    name = "nop";
                    break;
                case miniplc0::BIPUSH:
                    name = "bipush";
                    break;
                case miniplc0::IPUSH:
                    name = "ipush";
                    break;
                case miniplc0::POP:
                    name = "pop";
                    break;
                case miniplc0::POP2:
                    name = "pop2";
                    break;
                case miniplc0::POPN:
                    name = "popn";
                    break;
                case miniplc0::DUP:
                    name = "dup";
                    break;
                case miniplc0::DUP2:
                    name = "dup2";
                    break;
                case miniplc0::LOADC:
                    name = "loadc";
                    break;
                case miniplc0::LOADA:
                    name = "loada";
                    break;
                case miniplc0::NEW:
                    name = "new";
                    break;
                case miniplc0::SNEW:
                    name = "snew";
                    break;
                case miniplc0::ILOAD:
                    name = "iload";
                    break;
                case miniplc0::DLOAD:
                    name = "dload";
                    break;
                case miniplc0::ALOAD:
                    name = "aload";
                    break;
                case miniplc0::IALOAD:
                    name = "iaload";
                    break;
                case miniplc0::DALOAD:
                    name = "daload";
                    break;
                case miniplc0::AALOAD:
                    name = "aaloada";
                    break;
                case miniplc0::ISTORE:
                    name = "istore";
                    break;
                case miniplc0::DSTORE:
                    name = "dstore";
                    break;
                case miniplc0::ASTORE:
                    name = "astore";
                    break;
                case miniplc0::IASTORE:
                    name = "iastore";
                    break;
                case miniplc0::DASTORE:
                    name = "dastore";
                    break;
                case miniplc0::AASTORE:
                    name = "aastore";
                    break;
                case miniplc0::IADD:
                    name = "iadd";
                    break;
                case miniplc0::DADD:
                    name = "dadd";
                    break;
                case miniplc0::ISUB:
                    name = "isub";
                    break;
                case miniplc0::DSUB:
                    name = "dsub";
                    break;
                case miniplc0::IMUL:
                    name = "imul";
                    break;
                case miniplc0::DMUL:
                    name = "dmul";
                    break;
                case miniplc0::IDIV:
                    name = "idiv";
                    break;
                case miniplc0::DDIV:
                    name = "ddiv";
                    break;
                case miniplc0::INEG:
                    name = "ineg";
                    break;
                case miniplc0::DNEG:
                    name = "dneg";
                    break;
                case miniplc0::ICMP:
                    name = "icmp";
                    break;
                case miniplc0::DCMP:
                    name = "dcmp";
                    break;
                case miniplc0::I2D:
                    name = "i2d";
                    break;
                case miniplc0::D2I:
                    name = "d2i";
                    break;
                case miniplc0::I2C:
                    name = "i2c";
                    break;
                case miniplc0::JMP:
                    name = "jmp";
                    break;
                case miniplc0::JE:
                    name = "je";
                    break;
                case miniplc0::JNE:
                    name = "jne";
                    break;
                case miniplc0::JL:
                    name = "jl";
                    break;
                case miniplc0::JGE:
                    name = "jge";
                    break;
                case miniplc0::JG:
                    name = "jg";
                    break;
                case miniplc0::JLE:
                    name = "jle";
                    break;
                case miniplc0::CALL:
                    name = "call";
                    break;
                case miniplc0::RET:
                    name = "ret";
                    break;
                case miniplc0::IRET:
                    name = "iret";
                    break;
                case miniplc0::DRET:
                    name = "dret";
                    break;
                case miniplc0::ARET:
                    name = "aret";
                    break;
                case miniplc0::IPRINT:
                    name = "iprint";
                    break;
                case miniplc0::DPRINT:
                    name = "dprint";
                    break;
                case miniplc0::CPRINT:
                    name = "cprint";
                    break;
                case miniplc0::SPRINT:
                    name = "sprint";
                    break;
                case miniplc0::PRINTL:
                    name = "printl";
                    break;
                case miniplc0::ISCAN:
                    name = "iscan";
                    break;
                case miniplc0::DSCAN:
                    name = "dscan";
                    break;
                case miniplc0::CSCAN:
                    name = "cscan";
                    break;
			}

			return format_to(ctx.out(), name);
		}
	};
	template<>
	struct formatter<miniplc0::Instruction> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const miniplc0::Instruction &p, FormatContext &ctx) {
			std::string name;
			switch (p.GetOperation())
			{
			    case miniplc0::NOP:
                case miniplc0::POP:
                case miniplc0::POP2:
                case miniplc0::DUP:
                case miniplc0::DUP2:
                case miniplc0::NEW:
                case miniplc0::ILOAD:
                case miniplc0::DLOAD:
                case miniplc0::ALOAD:
                case miniplc0::IALOAD:
                case miniplc0::DALOAD:
                case miniplc0::AALOAD:
                case miniplc0::ISTORE:
                case miniplc0::DSTORE:
                case miniplc0::ASTORE:
                case miniplc0::IASTORE:
                case miniplc0::DASTORE:
                case miniplc0::AASTORE:
                case miniplc0::IADD:
                case miniplc0::DADD:
                case miniplc0::ISUB:
                case miniplc0::DSUB:
                case miniplc0::IMUL:
                case miniplc0::DMUL:
                case miniplc0::IDIV:
                case miniplc0::DDIV:
                case miniplc0::INEG:
                case miniplc0::DNEG:
                case miniplc0::ICMP:
                case miniplc0::DCMP:
                case miniplc0::I2D:
                case miniplc0::D2I:
                case miniplc0::I2C:
                case miniplc0::RET:
                case miniplc0::IRET:
                case miniplc0::DRET:
                case miniplc0::ARET:
                case miniplc0::IPRINT:
                case miniplc0::DPRINT:
                case miniplc0::CPRINT:
                case miniplc0::SPRINT:
                case miniplc0::PRINTL:
                case miniplc0::ISCAN:
                case miniplc0::DSCAN:
                case miniplc0::CSCAN:
                    return format_to(ctx.out(), "{}", p.GetOperation());
                case miniplc0::BIPUSH:
                case miniplc0::IPUSH:
                case miniplc0::POPN:
                case miniplc0::LOADC:
                case miniplc0::SNEW:
                case miniplc0::JMP:
                case miniplc0::JE:
                case miniplc0::JNE:
                case miniplc0::JL:
                case miniplc0::JGE:
                case miniplc0::JG:
                case miniplc0::JLE:
                case miniplc0::CALL:
                    return format_to(ctx.out(), "{} {}", p.GetOperation(), p.GetX());
                case miniplc0::LOADA:
                    return format_to(ctx.out(), "{} {}, {}", p.GetOperation(), p.GetX(), p.GetY());
			}
			return format_to(ctx.out(), "NOP");
		}
	};
}