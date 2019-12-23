#pragma once

#include "error/error.h"
#include "instruction/instruction.h"
#include "tokenizer/token.h"
#include "systable/systable.h"

#include <vector>
#include <optional>
#include <utility>
#include <map>
#include <cstdint>
#include <cstddef> // for std::size_t

namespace miniplc0 {

//    struct variableTable{   //.constants
//        std::string name;
//        int32_t type;       //常量，变量的类型
//        int32_t value;      //常量，变量的值
//    };
//    struct startInit{
//        std::string opcode;     //指令名
//        int32_t operands;       //操作数
//    };
//    struct functionsTable{
//        int32_t name_index;     // 函数名在.constants中的下标
//        int32_t params_size;    //参数占用的slot数
//        int32_t level;          //函数嵌套的层级
//    };
//    struct functionBodyTable{
//        int32_t name_index;     //函数名.constants中的下标
//        std::string opcode;     //指令名
//        int32_t operands;       //操作数
//    };

	class Analyser final {
	private:
		using uint64_t = std::uint64_t;
		using int64_t = std::int64_t;
		using uint32_t = std::uint32_t;
		using int32_t = std::int32_t;
	public:
		Analyser(std::vector<Token> v)
			: _tokens(std::move(v)), _offset(0), _instructions({}), _current_pos(0, 0),
			_var({}),_start({}),_fun({}),_indexTable({}), _nextTokenIndex(0),
			_nextVarAddress(0),_instructionIndex(-1) {}
		Analyser(Analyser&&) = delete;
		Analyser(const Analyser&) = delete;
		Analyser& operator=(Analyser) = delete;

		// 唯一接口
		std::pair<std::vector<functionBodyTable>, std::optional<CompilationError>> Analyse();
        std::vector<Instruction> getStartCode();
        std::vector<variableTable> getVarTable();
        std::vector<functionsTable> getFunctionTable();

	private:
		// 所有的递归子程序

        //<C0-program>
        std::optional<CompilationError> analyseC0Program();
        // <变量声明>
        std::optional<CompilationError> analyseVariableDeclaration(bool isGlobal);
        //<函数定义>
        std::optional<CompilationError> analyseFunctionDefinition();
		//<init-declarator> ::= <identifier>['='<expression>]
		std::optional<CompilationError> analyseInitDeclarator(bool isConstant, bool isGlobal);
        std::optional<CompilationError> analyseFunctionCall();

		std::optional<CompilationError> analyseExpression();
        std::optional<CompilationError> analyseMultiplicativeExpression();
        std::optional<CompilationError> analyseUnaryExpression();
        std::optional<CompilationError> analysePrimaryExpression();

        std::optional<CompilationError> analyseParameterDeclaration();

        std::optional<CompilationError> analyseCompoundStatement();
        std::optional<CompilationError> analyseStatementSeq();
        std::optional<CompilationError> analyseStatement();

        std::optional<CompilationError> analyseCondition();



        // <常表达式>
        // 这里的 out 是常表达式的值
        std::optional<CompilationError> analyseConstantExpression(int32_t& out);
		// Token 缓冲区相关操作

		// 返回下一个 token
		std::optional<Token> nextToken();
		// 回退一个 token
		void unreadToken();

		// 下面是符号表相关操作

		// helper function
		void _add(const Token&,int32_t type, int32_t level);
		// 添加变量、常量、未初始化的变量
		void addVariable(const Token&, int32_t level);
		void addConstant(const Token&, int32_t level);
		void addUninitializedVariable(const Token&, int32_t level);
        void addFunction(const Token&, int32_t level);
		// 是否被声明过
		bool isDeclared(const std::string&,int32_t level);
		// 是否是未初始化的变量
		bool isUninitializedVariable(const std::string&,int32_t level);
		// 是否是已初始化的变量
		bool isInitializedVariable(const std::string&,int32_t level);
		// 是否是常量
		bool isConstant(const std::string&,int32_t level);
		//
        bool isFunctionName(const std::string&s);

		// 获得 {变量，常量} 在栈上的偏移
		int32_t getIndex(const std::string&,int32_t level);
	private:
		std::vector<Token> _tokens;
		std::size_t _offset;
		std::vector<Instruction> _instructions;
		std::pair<uint64_t, uint64_t> _current_pos;

		std::vector<variableTable> _var;
		std::vector<Instruction> _start;
        std::vector<functionsTable> _fun;
        std::vector<std::vector<Instruction>> _fun_body;
        std::vector<functionBodyTable> _funInstruction;

		std::vector<int32_t> _indexTable;


		// 下一个 token 在栈的偏移
		int32_t _nextTokenIndex;
		// 符号在栈上的偏移
		int32_t _nextVarAddress;
		//函数指令集的索引，为-1则是全局初始化，否则是对应函数的指令集的索引
		int32_t _instructionIndex;
	};
}