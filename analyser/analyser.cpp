#include "analyser.h"

#include <climits>

namespace miniplc0 {
	std::pair<std::vector<functionBodyTable>, std::optional<CompilationError>> Analyser::Analyse() {
		auto err = analyseC0Program();
		if (err.has_value())
			return std::make_pair(std::vector<functionBodyTable>(), err);
		else
			return std::make_pair(_funInstruction, std::optional<CompilationError>());
	}

	//<C0-program> ::= {<variable-declaration>}{<function-definition>}
	std::optional<CompilationError> Analyser::analyseC0Program() {
        _indexTable.emplace_back(0);

	    while (true){
            auto next=nextToken();
            if (!next.has_value())
                return {};
            if(next.value().GetType()==CONST){//如果读到const，跳转变量声明语句
                unreadToken();
                auto err=analyseVariableDeclaration(true);
                if(err.has_value())
                    return err;
            }
            else if(next.value().GetType()==INT||next.value().GetType()==VOID){
                next=nextToken();
                if(!next.has_value()||next.value().GetType()!=IDENTIFIER){
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
                }
                next=nextToken();
                if(!next.has_value()){
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidVariableDeclaration);
                }
                else if(next.value().GetType()==LEFT_BRACKET){//函数声明
                    unreadToken();
                    unreadToken();
                    unreadToken();
                    break;
                }
                else{
                    unreadToken();
                    unreadToken();
                    unreadToken();
                    auto err=analyseVariableDeclaration(true);
                    if(err.has_value())
                        return err;
                }
            }
            else{
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidVariableDeclaration);
            }
	    }
	    while(true){
            auto next=nextToken();
            if (!next.has_value())
                break;
            unreadToken();
            auto err=analyseFunctionDefinition();
            if(err.has_value())
                return err;
	    }
	    int nf=_fun.size();
	    for(int i=0;i<nf;i++){
	        if(_fun[i]._value=="main")
                return {};
	    }

        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedMainFunction);
	}
	//<variable-declaration> ::= [<const-qualifier>]<type-specifier><init-declarator-list>';'
    //<init-declarator-list> ::= <init-declarator>{','<init-declarator>}
    //<init-declarator> ::= <identifier>['='<expression>]
    std::optional<CompilationError> Analyser::analyseVariableDeclaration(bool isGlobal) {
	    auto next=nextToken();
	    if(next.value().GetType()==VOID)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidVariableDeclaration);
        else if(next.value().GetType()==CONST){
            next=nextToken();
            if(!next.has_value()||next.value().GetType()!=INT)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidVariableDeclaration);
            else{
                auto err=analyseInitDeclarator(true,isGlobal);
                if(err.has_value())
                    return err;
                while(true){
                    next=nextToken();
                    if(next.value().GetType()==SEMICOLON){
                        unreadToken();
                        break;
                    }
                    else if(next.value().GetType()==COMMA){
                        auto err=analyseInitDeclarator(true, isGlobal);
                        if(err.has_value())
                            return err;
                    }
                    else{
                        unreadToken();
                        break;
                    }
                }
                next=nextToken();
                if(next.value().GetType()!=SEMICOLON)
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
            }
        }
        else if(next.value().GetType()==INT){
            auto err=analyseInitDeclarator(false, isGlobal);
            if(err.has_value())
                return err;
            while(true){
                next=nextToken();
                if(next.value().GetType()==SEMICOLON){
                    unreadToken();
                    break;
                }
                else if(next.value().GetType()==COMMA){
                    auto err=analyseInitDeclarator(false, isGlobal);
                    if(err.has_value())
                        return err;
                }
                else{
                    unreadToken();
                    break;
                }
            }
            next=nextToken();
            if(next.value().GetType()!=SEMICOLON)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
        }
        else{
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidVariableDeclaration);
        }
        return {};
	}
    //<init-declarator> ::= <identifier>['='<expression>]
    std::optional<CompilationError> Analyser::analyseInitDeclarator(bool isConstant, bool isGlobal) {
        auto next=nextToken();
        if(!next.has_value()||next.value().GetType()!=IDENTIFIER)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidVariableDeclaration);
        auto preToken=next;//标识符
        next=nextToken();
        if(isConstant){//是常量
            if(next.value().GetType()!=ASSIGNMENT_SIGN)//未赋值
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrConstantNeedValue);
            else{//要赋值
                if(isGlobal){//是全局常量
                    if(isDeclared(preToken.value().GetValueString(),0))
                        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);

//                    _start.emplace_back(IPUSH,_nextVarAddress-1,0);
                    auto err=analyseExpression();
                    if(err.has_value())
                        return err;
                    addConstant(preToken.value(),0);
                }
                else{//局部常量
                    if(isDeclared(preToken.value().GetValueString(),1))
                        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);

                    auto err=analyseExpression();
                    if(err.has_value())
                        return err;
                    addConstant(preToken.value(),1);
                }
            }
        }
        else{//是变量
            if(next.value().GetType()==TokenType::COMMA||next.value().GetType()==TokenType::SEMICOLON){//未赋值
                unreadToken();
                if(isGlobal){//是全局变量
                    if(isDeclared(preToken.value().GetValueString(),0))
                        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);
                    addUninitializedVariable(preToken.value(),0);
                    _start.emplace_back(SNEW,1,0);
//                    auto err=analyseExpression();
//                    if(err.has_value())
//                        return err;
                }
                else{//是局部变量
                    if(isDeclared(preToken.value().GetValueString(),1))
                        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);
                    addUninitializedVariable(preToken.value(),1);
                    _funInstruction[_instructionIndex]._funins.emplace_back(SNEW,1,0);
//                    auto err=analyseExpression();
//                    if(err.has_value())
//                        return err;
                }
                return {};
            }
            else{//为变量赋值
                if(isGlobal){//是全局变量
                    if(isDeclared(preToken.value().GetValueString(),0))
                        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);

                    auto err=analyseExpression();
                    if(err.has_value())
                        return err;
                    addVariable(preToken.value(),0);
                }
                else{//局部变量
                    if(isDeclared(preToken.value().GetValueString(),1))
                        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);

                    addUninitializedVariable(preToken.value(),1);//将局部变量先声明为未赋值变量,type=1;
                    auto err=analyseExpression();
                    if(err.has_value())
                        return err;
                    unsigned int nn=_var.size();
                    _var[nn-1]._type=2;
                }
            }
        }
        return {};
    }

	//<function-definition> ::= <type-specifier><identifier><parameter-clause><compound-statement>
    //<parameter-clause> ::= '(' [<parameter-declaration-list>] ')'
    //<parameter-declaration-list> ::= <parameter-declaration>{','<parameter-declaration>}
    std::optional<CompilationError> Analyser::analyseFunctionDefinition() {
	    int slot=0;
	    auto next=nextToken();
	    auto preNext=next;
	    if(!next.has_value() || (next.value().GetType()!=TokenType::INT && next.value().GetType()!=TokenType::VOID))
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDefinition);
	    next=nextToken();//函数名标识符
        if(!next.has_value() || next.value().GetType()!=TokenType::IDENTIFIER)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDefinition);
        if(isDeclared(next.value().GetValueString(),0)){
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);
        }
        addFunction(next.value(),1);

//        std::cout<<"???"<<_nextVarAddress<<"???"<<_instructionIndex<<std::endl;
//        std::cout<<_indexTable.size()<<std::endl;
        _indexTable.emplace_back(_nextVarAddress);//指向该函数第一个参数的下一个地址
        int oldAddress=_nextVarAddress;

        int tmp=_fun.size();
        if(preNext.value().GetType()==TokenType::INT){
            _fun[tmp-1]._haveReturnValue=1;
        }
        else{
            _fun[tmp-1]._haveReturnValue=0;
        }

        next=nextToken();
        if(!next.has_value() || next.value().GetType()!=TokenType::LEFT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDefinition);
        next=nextToken();
        if(!next.has_value())
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDefinition);
        else if(next.value().GetType()==TokenType::RIGHT_BRACKET){//函数参数为0
            unreadToken();
        }
        else{//函数参数有多个
//            std::cout<<_nextVarAddress<<"aaaaaaaaaaaaaaaaa"<<std::endl;
            unreadToken();
            auto err=analyseParameterDeclaration();
            if(err.has_value())
                return err;
            slot++;
            while (true){
                next=nextToken();
                if(!next.has_value())
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDefinition);
                if(next.value().GetType()==TokenType::COMMA){//后面还有参数
                    auto err=analyseParameterDeclaration();
                    if(err.has_value())
                        return err;
                    slot++;
                }
                else if(next.value().GetType()==TokenType::RIGHT_BRACKET){
                    unreadToken();
                    break;
                }
                else{
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDefinition);
                }
            }
        }
        next=nextToken();
        if(!next.has_value()||next.value().GetType()!=TokenType::RIGHT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoRightBracket);

        int n=_fun.size();
        _fun[n-1]._params_size=slot;
        _funInstruction.emplace_back(functionBodyTable());
//        _funInstruction[_instructionIndex]._funins.emplace_back(SNEW,slot,0);
//        _instructions.emplace_back(SNEW,slot,0);

        auto err=analyseCompoundStatement();
        if(err.has_value())
            return err;

        if(_fun[_instructionIndex]._haveReturnValue==0){
            _funInstruction[_instructionIndex]._funins.emplace_back(RET,0,0);
        }
        else if(_fun[_instructionIndex]._haveReturnValue==1){
            _funInstruction[_instructionIndex]._funins.emplace_back(IRET,0,0);
        }
        int nvar=_var.size();
        while (nvar>oldAddress){
            _var.pop_back();
            nvar=_var.size();
        }
        _indexTable[1]=oldAddress;
        _nextVarAddress=oldAddress;
        return {};
    }
    //<parameter-declaration> ::= [<const-qualifier>]<type-specifier><identifier>
    std::optional<CompilationError> Analyser::analyseParameterDeclaration(){
	    auto next=nextToken();
	    if(!next.has_value())
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDefinition);
	    else if(next.value().GetType()==TokenType::CONST){
	        next=nextToken();
	        if(!next.has_value()||next.value().GetType()!=TokenType::INT)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDefinition);
            next=nextToken();
            if(!next.has_value()||next.value().GetType()!=TokenType::IDENTIFIER)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
            //
            addConstant(next.value(),1);
	    }
	    else if(next.value().GetType()==TokenType::INT){
            next=nextToken();
            if(!next.has_value()||next.value().GetType()!=TokenType::IDENTIFIER)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
            addVariable(next.value(),1);
	    }
	    else{
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDefinition);
	    }
	    return {};
	}
    //<compound-statement> ::= '{' {<variable-declaration>} <statement-seq> '}'
    //<statement-seq> ::= {<statement>}
    //<statement> ::= '{' <statement-seq> '}'|<condition-statement>|<loop-statement>|<jump-statement>|<print-statement>
    //    |<scan-statement>|<assignment-expression>';'|<function-call>';'|';'
    std::optional<CompilationError> Analyser::analyseCompoundStatement(){
	    auto next=nextToken();
        if(!next.has_value()||next.value().GetType()!=TokenType::LEFT_BRACE)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoLeftBrace);
        //<variable-declaration> ::= [<const-qualifier>]<type-specifier><init-declarator-list>';'
        while (true){
            next=nextToken();
            if(!next.has_value())
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDefinition);
            if(next.value().GetType()==TokenType::CONST||next.value().GetType()==TokenType::INT){
                unreadToken();
                auto err=analyseVariableDeclaration(false);
                if(err.has_value())
                    return err;
            }
            else{
                unreadToken();
                break;
            }
        }
        while (true){
            next=nextToken();
            if(!next.has_value())
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDefinition);
            if(next.value().GetType()==TokenType::RIGHT_BRACE){
                unreadToken();
                break;
            }
            unreadToken();
            auto err=analyseStatementSeq();
            if(err.has_value())
                return err;
        }

        next=nextToken();
        if(!next.has_value()||next.value().GetType()!=TokenType::RIGHT_BRACE)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoRightBrace);
        return {};
	}
    //<statement-seq> ::= {<statement>}
    //<statement> ::= '{' <statement-seq> '}'|<condition-statement>|<loop-statement>|<jump-statement>|<print-statement>
    //    |<scan-statement>|<assignment-expression>';'|<function-call>';'|';'
    std::optional<CompilationError> Analyser::analyseStatementSeq() {
	    while (true){
            auto next=nextToken();
            if(!next.has_value()){
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoRightBrace);
            }
            if(next.value().GetType()==TokenType::RIGHT_BRACE){
                unreadToken();
                break;
            }
            unreadToken();
            auto err=analyseStatement();
            if(err.has_value())
                return err;
	    }
        return {};
	}

    std::optional<CompilationError> Analyser::analyseStatement(){
	    auto next=nextToken();
	    if(!next.has_value())
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteStatement);
        switch (next.value().GetType()){
            case LEFT_BRACE:{//'{' <statement-seq> '}'
                auto err=analyseStatementSeq();
                if(err.has_value())
                    return err;
                next=nextToken();
                if(!next.has_value()||next.value().GetType()!=TokenType::RIGHT_BRACE)
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoRightBrace);
                break;
            }
            case IF:{//<condition-statement>::='if' '(' <condition> ')' <statement> ['else' <statement>]
                next=nextToken();
                if(!next.has_value()||next.value().GetType()!=TokenType::LEFT_BRACKET)
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoLeftBracket);
                auto err=analyseCondition();
                int index1=_funInstruction[_instructionIndex]._funins.size()-1;
                if(err.has_value())
                    return err;
                next=nextToken();
                if(!next.has_value()||next.value().GetType()!=TokenType::RIGHT_BRACKET)
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoRightBracket);
                err=analyseStatement();
                if(err.has_value())
                    return err;



                _funInstruction[_instructionIndex]._funins.emplace_back(JMP,0,0);
                int index2=_funInstruction[_instructionIndex]._funins.size()-1;
                _funInstruction[_instructionIndex]._funins[index1].SetX(index2+1);

                int off1=_funInstruction[_instructionIndex]._funins.size();
                _funInstruction[_instructionIndex]._funins[index1].SetX(off1);


                next=nextToken();
                if(!next.has_value())
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteStatement);
                if(next.value().GetType()==TokenType::ELSE){

                    err=analyseStatement();
                    if(err.has_value())
                        return err;
                    int off2=_funInstruction[_instructionIndex]._funins.size();
                    _funInstruction[_instructionIndex]._funins[index2].SetX(off2);
                }
                else{
                    _funInstruction[_instructionIndex]._funins[index2].SetX(index2+1);
                    unreadToken();
                }

                break;
            }
            case WHILE:{//<loop-statement>::='while' '(' <condition> ')' <statement>
                int off1=_funInstruction[_instructionIndex]._funins.size();

                next=nextToken();
                if(!next.has_value()||next.value().GetType()!=TokenType::LEFT_BRACKET)
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoLeftBracket);

                auto err=analyseCondition();
                if(err.has_value())
                    return err;
                int index1=_funInstruction[_instructionIndex]._funins.size()-1;
                next=nextToken();
//                std::cout<<next.value().GetValueString()<<std::endl;
                if(!next.has_value()||next.value().GetType()!=TokenType::RIGHT_BRACKET)
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoRightBracket);

                err=analyseStatement();
                if(err.has_value())
                    return err;

                _funInstruction[_instructionIndex]._funins.emplace_back(JMP,off1,0);
                int off2=_funInstruction[_instructionIndex]._funins.size();
                _funInstruction[_instructionIndex]._funins[index1].SetX(off2);
                break;
            }
            case RETURN:{//<jump-statement>::= 'return' [<expression>] ';'
                next=nextToken();
                if(!next.has_value())
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteStatement);
                if(next.value().GetType()==TokenType::SEMICOLON){
                    if(_fun[_instructionIndex]._haveReturnValue==1){//函数声明时有返回值
                        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedReturnValue);
                    }
                    else{
                        _funInstruction[_instructionIndex]._funins.emplace_back(RET,0,0);
                        break;
                    }
                }
                else
                    unreadToken();
                auto err=analyseExpression();
                if(err.has_value())
                    return err;
                if(_fun[_instructionIndex]._haveReturnValue==0){//函数声明时无返回值
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoNeedReturnValue);
                }
                _funInstruction[_instructionIndex]._funins.emplace_back(IRET,0,0);

                next=nextToken();
                if(next.value().GetType()!=TokenType::SEMICOLON)
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
                break;
            }
            case SCAN:{//<scan-statement>::= 'scan' '(' <identifier> ')' ';'
                next=nextToken();
                if(!next.has_value()||next.value().GetType()!=TokenType::LEFT_BRACKET)
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoLeftBracket);
                next=nextToken();
                if(!next.has_value()||next.value().GetType()!=TokenType::IDENTIFIER)
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);

                auto preTokenStr=next.value().GetValueString();
                int n=_var.size();
                int addr=-1;
                int index=-1;
                for(int i=n-1;i>=0;i--){
                    if(preTokenStr==_var[i].getName()){
                        index=i;
                        break;
                    }
                }
                if(index==-1)
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
                if(_var[index].getType()==0)
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrAssignToConstant);
                addr=_var[index].getAddress();
                int _offset=addr-_indexTable[_var[index].getLevel()];;
                int _level_diff=1-_var[index].getLevel();
                _funInstruction[_instructionIndex]._funins.emplace_back(LOADA,_level_diff,_offset);
                _funInstruction[_instructionIndex]._funins.emplace_back(ISCAN,0,0);
                _funInstruction[_instructionIndex]._funins.emplace_back(ISTORE,0,0);

                _var[index]._type=2;

                next=nextToken();
                if(!next.has_value()||next.value().GetType()!=TokenType::RIGHT_BRACKET)
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoRightBracket);
                next=nextToken();
                if(!next.has_value()||next.value().GetType()!=TokenType::SEMICOLON)
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrConstantNeedValue);

                break;
            }
            case PRINT:{//<print-statement>::= 'print' '(' [<printable-list>] ')' ';'
                // <printable-list> ::= <printable> {',' <printable>}
                // <printable> ::= <expression>
                next=nextToken();
                if(!next.has_value()||next.value().GetType()!=TokenType::LEFT_BRACKET)
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoLeftBracket);
                next=nextToken();
                if(!next.has_value())
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteStatement);
                else if(next.value().GetType()==TokenType::RIGHT_BRACKET){
                    _funInstruction[_instructionIndex]._funins.emplace_back(PRINTL,0,0);
                    next=nextToken();
                    if(!next.has_value()||next.value().GetType()!=TokenType::SEMICOLON)
                        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
                    break;
                }
                else{
                    unreadToken();
                }
                auto err=analyseExpression();
                if(err.has_value())
                    return err;

                _funInstruction[_instructionIndex]._funins.emplace_back(IPRINT,0,0);

                while (true){
                    next=nextToken();
                    if(!next.has_value())
                        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteStatement);
                    if(next.value().GetType()==TokenType::RIGHT_BRACKET){
                        unreadToken();
                        break;
                    }
                    if(next.value().GetType()!=TokenType::COMMA)
                        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteStatement);
                    _funInstruction[_instructionIndex]._funins.emplace_back(BIPUSH,32,0);
                    _funInstruction[_instructionIndex]._funins.emplace_back(CPRINT,0,0);
                    auto err=analyseExpression();
                    if(err.has_value())
                        return err;
                    _funInstruction[_instructionIndex]._funins.emplace_back(IPRINT,0,0);
                }
                _funInstruction[_instructionIndex]._funins.emplace_back(PRINTL,0,0);
                next=nextToken();
                if(!next.has_value()||next.value().GetType()!=TokenType::RIGHT_BRACKET)
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoRightBracket);
                next=nextToken();
                if(!next.has_value()||next.value().GetType()!=TokenType::SEMICOLON)
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
                break;
            }
            case IDENTIFIER:{//<assignment-expression>';'|<function-call>';'
                //<assignment-expression> ::=<identifier><assignment-operator><expression>
                //<function-call> ::=<identifier> '(' [<expression-list>] ')'
                auto preTokenStr=next.value().GetValueString();


                next=nextToken();
                if(!next.has_value())
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteStatement);
                if(next.value().GetType()==TokenType::ASSIGNMENT_SIGN){

                    int n=_var.size();
                    int addr=-1;
                    int index=-1;
                    for(int i=n-1;i>=0;i--){
                        if(preTokenStr==_var[i].getName()){
                            index=i;
                            break;
                        }
                    }
                    if(index==-1)
                        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
                    addr=_var[index].getAddress();
                    if(_var[index]._type==0){
                        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrAssignToConstant);
                    }
                    int _offset=0;
                    int _level_diff=0;
                    _offset=addr-_indexTable[_var[index].getLevel()];
                    _level_diff=1-_var[index].getLevel();
                    _funInstruction[_instructionIndex]._funins.emplace_back(LOADA,_level_diff,_offset);

                    auto err=analyseExpression();
                    if(err.has_value())
                        return err;
                    _funInstruction[_instructionIndex]._funins.emplace_back(ISTORE,0,0);
                    _var[index]._type=2;
                }
                else if(next.value().GetType()==TokenType::LEFT_BRACKET){
                    unreadToken();
                    unreadToken();
                    auto err=analyseFunctionCall();
                    if(err.has_value())
                        return err;
                    _funInstruction[_instructionIndex]._funins.emplace_back(POP,0,0);
                }
                else
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteStatement);

                next=nextToken();
//                std::cout<<next.value().GetValueString()<<std::endl;
                if(!next.has_value()||next.value().GetType()!=TokenType::SEMICOLON)
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
                break;
            }
            case SEMICOLON:{//';'
                break;
            }
            default:
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteStatement);
        }
        return {};
	}
	//<condition> ::= <expression>[<relational-operator><expression>]
	//<relational-operator> ::= '<' | '<=' | '>' | '>=' | '!=' | '=='
    std::optional<CompilationError> Analyser::analyseCondition(){//'(' <condition> ')'
	    auto err=analyseExpression();
	    if(err.has_value())
            return err;
	    auto next=nextToken();
	    if(!next.has_value())
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoRightBracket);
        switch (next.value().GetType()){
            case RIGHT_BRACKET:{
                _funInstruction[_instructionIndex]._funins.emplace_back(JE,0,0);
                unreadToken();
                break;
            }
            case LESS_SIGN:{
                err=analyseExpression();
                if(err.has_value())
                    return err;
                _funInstruction[_instructionIndex]._funins.emplace_back(ISUB,0,0);
                _funInstruction[_instructionIndex]._funins.emplace_back(JGE,0,0);//如果<则顺序zhixing,>=则跳出
//                _instructions.emplace_back(ISUB,0,0);
//                _instructions.emplace_back(JL,0,0);
                break;
            }
            case LESS_EQUAL_SIGN:{
                err=analyseExpression();
                if(err.has_value())
                    return err;
                _funInstruction[_instructionIndex]._funins.emplace_back(ISUB,0,0);
                _funInstruction[_instructionIndex]._funins.emplace_back(JG,0,0);
//                _instructions.emplace_back(ISUB,0,0);
//                _instructions.emplace_back(JLE,0,0);
                break;
            }
            case GREATER_SIGN:{
                err=analyseExpression();
                if(err.has_value())
                    return err;
                _funInstruction[_instructionIndex]._funins.emplace_back(ISUB,0,0);
                _funInstruction[_instructionIndex]._funins.emplace_back(JLE,0,0);
//                _instructions.emplace_back(ISUB,0,0);
//                _instructions.emplace_back(JG,0,0);
                break;
            }
            case GREATER_EQUAL_SIGN:{
                err=analyseExpression();
                if(err.has_value())
                    return err;
                _funInstruction[_instructionIndex]._funins.emplace_back(ISUB,0,0);
                _funInstruction[_instructionIndex]._funins.emplace_back(JL,0,0);
//                _instructions.emplace_back(ISUB,0,0);
//                _instructions.emplace_back(JGE,0,0);
                break;
            }
            case NOT_EQUAL_SIGN:{
                err=analyseExpression();
                if(err.has_value())
                    return err;
                _funInstruction[_instructionIndex]._funins.emplace_back(ISUB,0,0);
                _funInstruction[_instructionIndex]._funins.emplace_back(JE,0,0);
//                _instructions.emplace_back(ISUB,0,0);
//                _instructions.emplace_back(JNE,0,0);
                break;
            }
            case EQUAL_SIGN:{
                err=analyseExpression();
                if(err.has_value())
                    return err;
                _funInstruction[_instructionIndex]._funins.emplace_back(ISUB,0,0);
                _funInstruction[_instructionIndex]._funins.emplace_back(JNE,0,0);
//                _instructions.emplace_back(ISUB,0,0);
//                _instructions.emplace_back(JE,0,0);
                break;
            }
            default:
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteCondition);
        }

        return {};
	}


    //<expression> ::= <additive-expression>
    //<additive-expression> ::= <multiplicative-expression>{<additive-operator><multiplicative-expression>}
    //<multiplicative-expression> ::= <unary-expression>{<multiplicative-operator><unary-expression>}
    //<unary-expression> ::= [<unary-operator>]<primary-expression>
    //<primary-expression> ::= '('<expression>')' |<identifier> |<integer-literal> |<function-call>
    std::optional<CompilationError> Analyser::analyseExpression() {
        auto err=analyseMultiplicativeExpression();
        if(err.has_value())
            return err;
        while (true){
            auto next=nextToken();
            if(!next.has_value())
                return {};
            if(next.value().GetType()!=TokenType::PLUS_SIGN && next.value().GetType()!=TokenType::MINUS_SIGN){
                unreadToken();
                return {};
            }
            err=analyseMultiplicativeExpression();
            if(err.has_value())
                return err;

            if(_instructionIndex==-1){
                if (next.value().GetType() == TokenType::PLUS_SIGN)
                    _start.emplace_back(Operation::IADD, 0, 0);
                else if (next.value().GetType() == TokenType::MINUS_SIGN)
                    _start.emplace_back(Operation::ISUB, 0, 0);
            }
            else{
                if (next.value().GetType() == TokenType::PLUS_SIGN)
                    _funInstruction[_instructionIndex]._funins.emplace_back(IADD,0,0);
//                    _instructions.emplace_back(Operation::IADD, 0, 0);
                else if (next.value().GetType() == TokenType::MINUS_SIGN)
                    _funInstruction[_instructionIndex]._funins.emplace_back(ISUB,0,0);
//                    _instructions.emplace_back(Operation::ISUB, 0, 0);
            }
        }
        return {};
    }
    std::optional<CompilationError> Analyser::analyseMultiplicativeExpression() {
        auto err=analyseUnaryExpression();
        if(err.has_value())
            return err;
        while (true){
            auto next=nextToken();
            if(!next.has_value())
                return {};
            if(next.value().GetType()!=TokenType::MULTIPLICATION_SIGN && next.value().GetType()!=TokenType::DIVISION_SIGN){
                unreadToken();
                return {};
            }
            err=analyseUnaryExpression();
            if(err.has_value())
                return err;

            //根据结果生成指令
            if(_instructionIndex==-1){
                if (next.value().GetType() == TokenType::MULTIPLICATION_SIGN)
                    _start.emplace_back(Operation::IMUL, 0, 0);
                else if (next.value().GetType() == TokenType::DIVISION_SIGN)
                    _start.emplace_back(Operation::IDIV, 0, 0);
            }
            else{
                if (next.value().GetType() == TokenType::MULTIPLICATION_SIGN)
                    _funInstruction[_instructionIndex]._funins.emplace_back(IMUL,0,0);
//                    _instructions.emplace_back(Operation::IMUL, 0, 0);
                else if (next.value().GetType() == TokenType::DIVISION_SIGN)
                    _funInstruction[_instructionIndex]._funins.emplace_back(IDIV,0,0);
//                    _instructions.emplace_back(Operation::IDIV, 0, 0);
            }
        }
        return {};
    }
    //<unary-expression> ::= [<unary-operator>]'('<expression>')' |<identifier> |<integer-literal> |<function-call>
    std::optional<CompilationError> Analyser::analyseUnaryExpression() {
        auto next=nextToken();
        auto prefix=1;
        if(!next.has_value())
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
        if(next.value().GetType()==TokenType::PLUS_SIGN)
            prefix=1;
        else if (next.value().GetType() == TokenType::MINUS_SIGN) {
            prefix = -1;
            //_instructions.emplace_back(Operation::NOP, 0);          //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        }
        else
            unreadToken();
        next=nextToken();
        if (!next.has_value())
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
        switch (next.value().GetType()){
            case LEFT_BRACKET:{
                auto err=analyseExpression();
                if(err.has_value()){
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
                }
                next=nextToken();
                if(!next.has_value()||next.value().GetType()!=TokenType::RIGHT_BRACKET){
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
                }
                break;
            }
            case INT_LITERAL:{
                int32_t x=atoi(next.value().GetValueString().c_str());
                if(_instructionIndex==-1){
                    _start.emplace_back(IPUSH,x,0);
                }
                else{
                    _funInstruction[_instructionIndex]._funins.emplace_back(IPUSH,x,0);
//                    _instructions.emplace_back(IPUSH,x,0);
                }
                break;
            }
            case IDENTIFIER:{
                auto preToken=next;         //preToken是标识符identifer
                auto preTokenStr=preToken.value().GetValueString();
                next=nextToken();
                if(!next.has_value()||next.value().GetType()!=TokenType::LEFT_BRACKET){//<unary-expression>:=[<unary-operator>]<identifier>
                    unreadToken();
                    int n=_var.size();
                    int addr=-1;
                    int index=-1;
                    for(int i=n-1;i>=0;i--){
                        if(preTokenStr==_var[i].getName()){
                            index=i;
                            break;
                        }
                    }
                    if(index==-1)
                        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
                    if(_var[index].getType()==1)
                        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotInitialized);
                    if(_var[index].getType()==3)
                        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
                    addr=_var[index].getAddress();
                    int _offset=0;
                    int _level_diff=0;
                    if(_instructionIndex==-1){//现在在全局变量赋值
                        _offset=addr;
                        _level_diff=0;
                        _start.emplace_back(LOADA,_level_diff,_offset);
                        _start.emplace_back(ILOAD,0,0);
                    }
                    else{//局部变量赋值
                        _offset=addr-_indexTable[_var[index].getLevel()];
                        _level_diff=1-_var[index].getLevel();
//                        std::cout<<"indextable[1]:"<<_indexTable[1]<<std::endl;
//                        std::cout<<"level:"<<_var[index].getLevel()<<"  indextable[]:"<<_indexTable[_var[index].getLevel()]<<std::endl;
//                        std::cout<<"addr:"<<addr<<std::endl;
//                        std::cout<<"off:"<<_offset<<std::endl;

                        _funInstruction[_instructionIndex]._funins.emplace_back(LOADA,_level_diff,_offset);
                        _funInstruction[_instructionIndex]._funins.emplace_back(ILOAD,0,0);
//                        _instructions.emplace_back(LOADA,_level_diff,_offset);
//                        _instructions.emplace_back(ILOAD,0,0);
                    }
                }
                else{//<unary-expression>:=<function-call> ::= <identifier> '(' [<expression-list>] ')'
                    unreadToken();
                    unreadToken();
                    auto err=analyseFunctionCall();
                    if(err.has_value())
                        return err;
                }
                break;
            }
            default:
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
        }
        if (prefix == -1){
            if(_instructionIndex==-1){
                _start.emplace_back(Operation::INEG, 0,0);
            }
            else{
                _funInstruction[_instructionIndex]._funins.emplace_back(INEG,0,0);
//                _instructions.emplace_back(Operation::INEG, 0,0);
            }
        }
        return {};
    }

    //<function-call> ::= <identifier> '(' [<expression-list>] ')'
    //<expression-list> ::= <expression>{','<expression>}
    std::optional<CompilationError> Analyser::analyseFunctionCall() {
	    int params=0;
        auto next=nextToken();
        if(!next.has_value()||next.value().GetType()!=TokenType::IDENTIFIER)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
        auto funToken=next;

        int nf=_fun.size();
        bool haveFunction= false;
        for(int i=0;i<nf;i++){
            if(funToken.value().GetValueString()==_fun[i]._value)
                haveFunction=true;
        }

        if(!haveFunction)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteFunctionCall);

        next=nextToken();
        if(!next.has_value()||next.value().GetType()!=TokenType::LEFT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteFunctionCall);
        next=nextToken();
        if(!next.has_value()||next.value().GetType()!=TokenType::RIGHT_BRACKET){
            unreadToken();
            auto err=analyseExpression();
            if(err.has_value())
                return err;
            params++;
            while (true){
                next=nextToken();
                if(!next.has_value()||next.value().GetType()!=TokenType::COMMA){
                    unreadToken();
                    break;
                }
                err=analyseExpression();
                if(err.has_value())
                    return err;
                params++;
            }
        }
        next=nextToken();
        if(!next.has_value()||next.value().GetType()!=TokenType::RIGHT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteFunctionCall);

        nf=_fun.size();
        int tmpIndex=-1;
        for(int i=0;i<nf;i++){
            if(funToken.value().GetValueString()==_fun[i]._value)
                tmpIndex=i;
        }

        if(params==_fun[tmpIndex]._params_size)
            _funInstruction[_instructionIndex]._funins.emplace_back(CALL,tmpIndex,0);
        else
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteFunctionCall);
        return {};
    }


	std::optional<Token> Analyser::nextToken() {
		if (_offset == _tokens.size())
			return {};
		// 考虑到 _tokens[0..._offset-1] 已经被分析过了
		// 所以我们选择 _tokens[0..._offset-1] 的 EndPos 作为当前位置
		_current_pos = _tokens[_offset].GetEndPos();
		return _tokens[_offset++];
	}

	void Analyser::unreadToken() {
		if (_offset == 0)
			DieAndPrint("analyser unreads token from the begining.");
		_current_pos = _tokens[_offset - 1].GetEndPos();
		_offset--;
	}


    void Analyser::_add(const Token& tk, int32_t type, int32_t level ) {
        if (tk.GetType() != TokenType::IDENTIFIER)
            DieAndPrint("only identifier can be added to the table.");
        if(type==3){
            functionsTable f("S",0,1,tk.GetValueString());
            _fun.emplace_back(f);
            _instructionIndex++;
        }
        else{
            variableTable v(tk.GetValueString(),type,level,_nextVarAddress);
            _nextVarAddress++;
            _var.emplace_back(v);
        }
        _nextTokenIndex++;
    }

	void Analyser::addConstant(const Token& tk,int32_t level) {
		_add(tk, 0, level);
	}

	void Analyser::addUninitializedVariable(const Token& tk,int32_t level) {
		_add(tk, 1, level);
	}

    void Analyser::addVariable(const Token& tk,int32_t level) {
        _add(tk, 2, level);
    }
    void Analyser::addFunction(const Token& tk,int32_t level) {
        _add(tk, 3, level);
    }

	int32_t Analyser::getIndex(const std::string& s,int32_t level) {
        int n=_var.size();
        for(int i=0;i<n;i++){
            if(s==_var[i].getName() && _var[i].getLevel()==level)//各个变量的类型,0为常量,1为未赋值变量，2为已赋值变量,3为函数
                return i;
        }
        return -1;
	}

	bool Analyser::isDeclared(const std::string& s,int32_t level) {//level为0则是全局的
        int n=_var.size();
        for(int i=0;i<n;i++){
            if(s==_var[i].getName() && _var[i].getLevel()==level)//各个变量的类型,0为常量,1为未赋值变量，2为已赋值变量,3为函数
                return true;
        }
        if(isFunctionName(s)){
            return true;
        }
        return false;
	}
    bool Analyser::isConstant(const std::string&s,int32_t level) {
        int n=_var.size();
        for(int i=0;i<n;i++){
            if(s==_var[i].getName() && _var[i].getLevel()==level && _var[i].getType()==0)
                return true;
        }
        return false;
    }
	bool Analyser::isUninitializedVariable(const std::string& s,int32_t level) {
        int n=_var.size();
        for(int i=0;i<n;i++){
            if(s==_var[i].getName() && _var[i].getLevel()==level && _var[i].getType()==1)
                return true;
        }
        return false;
	}
	bool Analyser::isInitializedVariable(const std::string&s,int32_t level) {
        int n=_var.size();
        for(int i=0;i<n;i++){
            if(s==_var[i].getName() && _var[i].getLevel()==level && _var[i].getType()==2)
                return true;
        }
        return false;
	}
    bool Analyser::isFunctionName(const std::string&s) {
        int n=_fun.size();
        for(int i=0;i<n;i++){
            if(s==_fun[i]._value&&_instructionIndex==i)
                return true;
        }
        return false;
    }

    std::vector<Instruction> Analyser::getStartCode(){
        return _start;
	}
    std::vector<variableTable> Analyser::getVarTable(){
        return _var;
	}
    std::vector<functionsTable> Analyser::getFunctionTable(){
        return _fun;
	}

}