#include "tokenizer/tokenizer.h"

#include <cctype>
#include <sstream>

namespace miniplc0 {

    std::pair<std::optional<Token>, std::optional<CompilationError>> Tokenizer::NextToken() {
        if (!_initialized)
            readAll();
        if (_rdr.bad())
            return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrorCode::ErrStreamError));
        if (isEOF())
            return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrorCode::ErrEOF));
        auto p = nextToken();
        if (p.second.has_value())
            return std::make_pair(p.first, p.second);
        auto err = checkToken(p.first.value());
        if (err.has_value())
            return std::make_pair(p.first, err.value());
        return std::make_pair(p.first, std::optional<CompilationError>());
    }

    std::pair<std::vector<Token>, std::optional<CompilationError>> Tokenizer::AllTokens() {
        std::vector<Token> result;
        while (true) {
            auto p = NextToken();
            if (p.second.has_value()) {
                if (p.second.value().GetCode() == ErrorCode::ErrEOF)
                    return std::make_pair(result, std::optional<CompilationError>());
                else
                    return std::make_pair(std::vector<Token>(), p.second);
            }
            result.emplace_back(p.first.value());
        }
    }

    // 注意：这里的返回值中 Token 和 CompilationError 只能返回一个，不能同时返回。
    std::pair<std::optional<Token>, std::optional<CompilationError>> Tokenizer::nextToken() {
        // 用于存储已经读到的组成当前token字符
        std::stringstream ss;
        // 分析token的结果，作为此函数的返回值
        std::pair<std::optional<Token>, std::optional<CompilationError>> result;
        // <行号，列号>，表示当前token的第一个字符在源代码中的位置
        std::pair<int64_t, int64_t> pos;
        // 记录当前自动机的状态，进入此函数时是初始状态
        DFAState current_state = DFAState::INITIAL_STATE;
        // 这是一个死循环，除非主动跳出
        // 每一次执行while内的代码，都可能导致状态的变更
        while (true) {
            // 读一个字符，请注意auto推导得出的类型是std::optional<char>
            // 这里其实有两种写法
            // 1. 每次循环前立即读入一个 char
            // 2. 只有在可能会转移的状态读入一个 char
            // 因为我们实现了 unread，为了省事我们选择第一种
            auto current_char = nextChar();
            // 针对当前的状态进行不同的操作
            switch (current_state) {

                // 初始状态
                // 这个 case 我们给出了核心逻辑，但是后面的 case 不用照搬。
                case INITIAL_STATE: {
                    // 已经读到了文件尾
                    if (!current_char.has_value())
                        // 返回一个空的token，和编译错误ErrEOF：遇到了文件尾
                        return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrEOF));

                    // 获取读到的字符的值，注意auto推导出的类型是char
                    auto ch = current_char.value();
                    // 标记是否读到了不合法的字符，初始化为否
                    auto invalid = false;

                    // 使用了自己封装的判断字符类型的函数，定义于 tokenizer/utils.hpp
                    // see https://en.cppreference.com/w/cpp/string/byte/isblank
                    if (miniplc0::isspace(ch)) // 读到的字符是空白字符（空格、换行、制表符等）
                        current_state = DFAState::INITIAL_STATE; // 保留当前状态为初始状态，此处直接break也是可以的
                    else if (!miniplc0::isprint(ch)) // control codes and backspace
                        invalid = true;
                    else if (miniplc0::isdigit(ch)){
                        if(ch=='0'){
                            current_state = DFAState::ZERO_STATE; // 切换到首字母为0的整数的状态
                        }
                        else{
                            current_state = DFAState::INT_DECIMAL_STATE; // 切换到十进制整数的状态
                        }
                    } // 读到的字符是数字
                    else if (miniplc0::isalpha(ch)) // 读到的字符是英文字母
                        current_state = DFAState::IDENTIFIER_STATE; // 切换到标识符的状态
                    else {
                        switch (ch) {

                            case '=': // 如果读到的字符是`=`，则切换到等于号的状态
                                current_state = DFAState::ASSIGNMENT_SIGN_STATE;
                                break;
                            case '-':
                                current_state = DFAState::MINUS_SIGN_STATE;
                                break;
                                // 请填空：切换到减号的状态
                            case '+':
                                // 请填空：切换到加号的状态
                                current_state = DFAState::PLUS_SIGN_STATE;
                                break;
                            case '*':
                                // 请填空：切换状态
                                current_state = DFAState::MULTIPLICATION_SIGN_STATE;
                                break;
                            case '/':
                                // 请填空：切换状态
                                current_state = DFAState::DIVISION_SIGN_STATE;
                                break;
                            case ';':
                                current_state = DFAState::SEMICOLON_STATE;
                                break;
                            case ',':
                                current_state = DFAState::COMMA_STATE;
                                break;
                            case '(':
                                current_state = DFAState::LEFT_BRACKET_STATE;
                                break;
                            case ')':
                                current_state = DFAState::RIGHT_BRACKET_STATE;
                                break;
                            case '{':
                                current_state = DFAState::LEFT_BRACE_STATE;
                                break;
                            case '}':
                                current_state = DFAState::RIGHT_BRACE_STATE;
                                break;
                            case '!':
                                current_state = DFAState::EXCLAMATION_SIGN_STATE;
                                break;
                            case '<':
                                current_state = DFAState::LESS_SIGN_STATE;
                                break;
                            case '>':
                                current_state = DFAState::GREATER_SIGN_STATE;
                                break;

                                ///// 请填空：
                                ///// 对于其他的可接受字符
                                ///// 切换到对应的状态

                                // 不接受的字符导致的不合法的状态
                            default:
                                invalid = true;
                                break;
                        }
                    }
                    // 如果读到的字符导致了状态的转移，说明它是一个token的第一个字符
                    if (current_state != DFAState::INITIAL_STATE)
                        pos = previousPos(); // 记录该字符的的位置为token的开始位置
                    // 读到了不合法的字符
                    if (invalid) {
                        // 回退这个字符
                        unreadLast();
                        // 返回编译错误：非法的输入
                        pos=currentPos();
                        return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrInvalidInput));
                    }
                    // 如果读到的字符导致了状态的转移，说明它是一个token的第一个字符
                    if (current_state != DFAState::INITIAL_STATE) // ignore white spaces
                        ss << ch; // 存储读到的字符
                    break;
                }

                //当前状态是以0为首的整数
                case ZERO_STATE:{
                    if(!current_char.has_value()){
                        std::string token_string;
                        ss>>token_string;
                        return std::make_pair(std::make_optional<Token>(TokenType::INT_LITERAL,token_string,pos,currentPos()),std::optional<CompilationError>());
                    }
                    auto ch=current_char.value();
                    if(miniplc0::isdigit(ch)){
                        ss<<ch;
                        current_state=DFAState::INT_DECIMAL_STATE;
                    }
                    else if(ch=='x'||ch=='X'){
                        ss<<ch;
                        current_state=DFAState::INT_HEXADECIMAL_STATE;
                    }
                    else if(miniplc0::isalpha(ch)){
                        ss<<ch;
                        current_state=DFAState::IDENTIFIER_STATE;
                    }
                    else{
                        //返回整数'0'
                        unreadLast();
                        std::string token_string;
                        ss>>token_string;
                        return std::make_pair(std::make_optional<Token>(TokenType::INT_LITERAL,token_string,pos,currentPos()),std::optional<CompilationError>());
                    }
                    break;
                }

                //当前状态是十六进制整数
                case INT_HEXADECIMAL_STATE: {
                    // 已经读到了文件尾
                    if (!current_char.has_value()){
                        std::string token_string,token_string_dec;
                        int sum=0,len=0,index=0;
                        ss>>token_string;
                        token_string_dec=token_string;
                        auto ct=checkToken(Token(INT_HEXADECIMAL,token_string,pos,currentPos()));//检查十六进制整数是否合法
                        if(ct.has_value()){
                            return std::make_pair(std::optional<Token>(), ct);
                        }
                        len=token_string.length();
                        for(index=2;index<len;index++){
                            if(token_string.at(index)!='0')
                                break;
                        }
                        len=token_string.length()-index;
                        if(len>8||(len==8&&token_string[index]>'7'))//检查是否溢出:
                            return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos,ErrorCode::ErrIntegerOverflow));
                        len=token_string.length();
                        for(int i=index;i<len;i++){
                            sum=sum*16+Hex2Dec(token_string[i]);
                        }
                        token_string_dec=std::to_string(sum);

                        return std::make_pair(std::make_optional<Token>(TokenType::INT_LITERAL,token_string_dec,pos,currentPos()), std::optional<CompilationError>());
                    }
                    // 获取读到的字符的值，注意auto推导出的类型是char
                    auto ch = current_char.value();
                    if (miniplc0::isdigit(ch)) // 读到的字符是数字
                        ss<<ch;
                    else if (miniplc0::isalpha(ch)) {
                        ss<<ch;
                    }// 读到的字符是英文字母
                    else{ // 如果读到的字符不是上述情况之一
                        unreadLast();
                        std::string token_string,token_string_dec;
                        int sum=0,len=0,index=0;
                        ss>>token_string;
                        token_string_dec=token_string;
//                        std::cout<<"here,token!"<<token_string<<std::endl;

                        auto ct=checkToken(Token(INT_HEXADECIMAL,token_string,pos,currentPos()));//检查十六进制整数是否合法
                        if(ct.has_value()){
                            return std::make_pair(std::optional<Token>(), ct);
                        }
                        len=token_string.length();
                        for(index=2;index<len;index++){
                            if(token_string.at(index)!='0')
                                break;
                        }
                        len=token_string.length()-index;
                        if(len>8||(len==8&&token_string[index]>'7'))//检查是否溢出
                            return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos,ErrorCode::ErrIntegerOverflow));
                        len=token_string.length();
                        for(int i=index;i<len;i++){
                            sum=sum*16+Hex2Dec(token_string[i]);
                        }
                        token_string_dec=std::to_string(sum);

                        return std::make_pair(std::make_optional<Token>(TokenType::INT_LITERAL,token_string_dec,pos,currentPos()), std::optional<CompilationError>());
                    }

                    break;
                }

                    // 当前状态是十进制整数
                case INT_DECIMAL_STATE: {
                    // 如果当前已经读到了文件尾，则解析已经读到的字符串为整数,解析成功则返回无符号整数类型的token，否则返回编译错误
                    // 如果读到的字符是数字，则存储读到的字符;如果读到的是字母，则存储读到的字符，并切换状态到标识符
                    // 如果读到的字符不是上述情况之一，则回退读到的字符，并解析已经读到的字符串为整数,解析成功则返回无符号整数类型的token，否则返回编译错误

                    // 已经读到了文件尾
                    if (!current_char.has_value()){
                        std::string token_string;
                        ss>>token_string;
                        unsigned int len=token_string.length();
                        unsigned int index=0;
                        int32_t token_integer;
                        auto ct=checkToken(Token(INT_DECIMAL,token_string,pos,currentPos()));
                        if(ct.has_value()){
                            return std::make_pair(std::optional<Token>(), ct);
                        }
//                        for(index=0;index<len;index++){
//                            if(token_string.at(index)!='0')
//                                break;
//                        }
//                        len=len-index;
                        if(len>10||(len==10&&token_string.at(index)>'2'))//checkToken已经判断了前导为0的情况
                            return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos,ErrorCode::ErrIntegerOverflow));
                        else{
                            token_integer=atoi(token_string.c_str());
                            if(token_integer<0)
                                return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos,ErrorCode::ErrIntegerOverflow));
                            else
                                return std::make_pair(std::make_optional<Token>(TokenType::INT_LITERAL,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }

                        return std::make_pair(std::make_optional<Token>(TokenType::INT_DECIMAL,token_string,pos,currentPos()), std::optional<CompilationError>());
                    }
                    // 获取读到的字符的值，注意auto推导出的类型是char
                    auto ch = current_char.value();
                    if (miniplc0::isdigit(ch)) // 读到的字符是数字
                        ss<<ch;
                    else if (miniplc0::isalpha(ch)) {
                        ss<<ch;
                        current_state = DFAState::IDENTIFIER_STATE;
                    }// 读到的字符是英文字母
                    else{ // 如果读到的字符不是上述情况之一
                        unreadLast();
                        std::string token_string;
                        ss>>token_string;
                        unsigned int len=token_string.length();
                        unsigned int index=0;
                        int32_t token_integer;
                        auto ct=checkToken(Token(INT_DECIMAL,token_string,pos,currentPos()));
                        if(ct.has_value()){
                            return std::make_pair(std::optional<Token>(), ct);
                        }
                        if(len>10||(len==10&&token_string.at(index)>'2'))
                            return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos,ErrorCode::ErrIntegerOverflow));
                        else{
                            token_integer=atoi(token_string.c_str());
                            if(token_integer<0)
                                return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos,ErrorCode::ErrIntegerOverflow));
                            else
                                return std::make_pair(std::make_optional<Token>(TokenType::INT_LITERAL,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                    }

                    break;
                }
                case IDENTIFIER_STATE: {
                    // 请填空：
                    // 如果当前已经读到了文件尾，则解析已经读到的字符串
                    //     如果解析结果是关键字，那么返回对应关键字的token，否则返回标识符的token
                    // 如果读到的是字符或字母，则存储读到的字符
                    // 如果读到的字符不是上述情况之一，则回退读到的字符，并解析已经读到的字符串
                    //     如果解析结果是关键字，那么返回对应关键字的token，否则返回标识符的token

                    // 已经读到了文件尾
                    if (!current_char.has_value()){
                        std::string token_string;
                        ss>>token_string;

                        auto ct=checkToken(Token(IDENTIFIER,token_string,pos,currentPos()));
                        if(ct.has_value()){
                            return std::make_pair(std::optional<Token>(), ct);
                        }

                        if(token_string=="const"){
                            return std::make_pair(std::make_optional<Token>(TokenType::CONST,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="void"){
                            return std::make_pair(std::make_optional<Token>(TokenType::VOID,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="int"){
                            return std::make_pair(std::make_optional<Token>(TokenType::INT,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="char"){
                            return std::make_pair(std::make_optional<Token>(TokenType::CHAR,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="double"){
                            return std::make_pair(std::make_optional<Token>(TokenType::DOUBLE,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="struct"){
                            return std::make_pair(std::make_optional<Token>(TokenType::STRUCT,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="if"){
                            return std::make_pair(std::make_optional<Token>(TokenType::IF,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="else"){
                            return std::make_pair(std::make_optional<Token>(TokenType::ELSE,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="switch"){
                            return std::make_pair(std::make_optional<Token>(TokenType::SWITCH,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="case"){
                            return std::make_pair(std::make_optional<Token>(TokenType::CASE,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="default"){
                            return std::make_pair(std::make_optional<Token>(TokenType::DEFAULT,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="while"){
                            return std::make_pair(std::make_optional<Token>(TokenType::WHILE,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="for"){
                            return std::make_pair(std::make_optional<Token>(TokenType::FOR,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="do"){
                            return std::make_pair(std::make_optional<Token>(TokenType::DO,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="return"){
                            return std::make_pair(std::make_optional<Token>(TokenType::RETURN,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="break"){
                            return std::make_pair(std::make_optional<Token>(TokenType::BREAK,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="continue"){
                            return std::make_pair(std::make_optional<Token>(TokenType::CONTINUE,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="print"){
                            return std::make_pair(std::make_optional<Token>(TokenType::PRINT,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="scan"){
                            return std::make_pair(std::make_optional<Token>(TokenType::SCAN,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else{
                            return std::make_pair(std::make_optional<Token>(TokenType::IDENTIFIER,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                    }
                    // 获取读到的字符的值，注意auto推导出的类型是char
                    auto ch = current_char.value();
                    if (miniplc0::isdigit(ch)) // 读到的字符是数字
                        ss<<ch;
                    else if (miniplc0::isalpha(ch)) {
                        ss<<ch;
                    }// 读到的字符是英文字母

                    else{ // 如果读到的字符不是上述情况之一
                        unreadLast();
                        std::string token_string;
                        ss>>token_string;

                        auto ct=checkToken(Token(IDENTIFIER,token_string,pos,currentPos()));
                        if(ct.has_value()){
                            return std::make_pair(std::optional<Token>(), ct);;
                        }
                        if(token_string=="const"){
                            return std::make_pair(std::make_optional<Token>(TokenType::CONST,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="void"){
                            return std::make_pair(std::make_optional<Token>(TokenType::VOID,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="int"){
                            return std::make_pair(std::make_optional<Token>(TokenType::INT,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="char"){
                            return std::make_pair(std::make_optional<Token>(TokenType::CHAR,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="double"){
                            return std::make_pair(std::make_optional<Token>(TokenType::DOUBLE,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="struct"){
                            return std::make_pair(std::make_optional<Token>(TokenType::STRUCT,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="if"){
                            return std::make_pair(std::make_optional<Token>(TokenType::IF,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="else"){
                            return std::make_pair(std::make_optional<Token>(TokenType::ELSE,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="switch"){
                            return std::make_pair(std::make_optional<Token>(TokenType::SWITCH,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="case"){
                            return std::make_pair(std::make_optional<Token>(TokenType::CASE,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="default"){
                            return std::make_pair(std::make_optional<Token>(TokenType::DEFAULT,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="while"){
                            return std::make_pair(std::make_optional<Token>(TokenType::WHILE,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="for"){
                            return std::make_pair(std::make_optional<Token>(TokenType::FOR,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="do"){
                            return std::make_pair(std::make_optional<Token>(TokenType::DO,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="return"){
                            return std::make_pair(std::make_optional<Token>(TokenType::RETURN,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="break"){
                            return std::make_pair(std::make_optional<Token>(TokenType::BREAK,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="continue"){
                            return std::make_pair(std::make_optional<Token>(TokenType::CONTINUE,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="print"){
                            return std::make_pair(std::make_optional<Token>(TokenType::PRINT,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else if(token_string=="scan"){
                            return std::make_pair(std::make_optional<Token>(TokenType::SCAN,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                        else{
                            return std::make_pair(std::make_optional<Token>(TokenType::IDENTIFIER,token_string,pos,currentPos()), std::optional<CompilationError>());
                        }
                    }

                    break;
                }

                    // 如果当前状态是加号
                case PLUS_SIGN_STATE: {
                    unreadLast();
                    return std::make_pair(std::make_optional<Token>(TokenType::PLUS_SIGN, '+', pos, currentPos()), std::optional<CompilationError>());
                }
                    // 当前状态为减号的状态
                case MINUS_SIGN_STATE: {
                    unreadLast();
                    return std::make_pair(std::make_optional<Token>(TokenType::MINUS_SIGN,'-',pos,currentPos()),std::optional<CompilationError>());
                }

                    // 请填空：
                    // 对于其他的合法状态，进行合适的操作
                    // 比如进行解析、返回token、返回编译错误
                case DIVISION_SIGN_STATE:{
//                    unreadLast();
//                    return std::make_pair(std::make_optional<Token>(TokenType::DIVISION_SIGN,'/',pos,currentPos()),std::optional<CompilationError>());
                    if (!current_char.has_value()){
                        return std::make_pair(std::make_optional<Token>(TokenType::DIVISION_SIGN,'/',pos,currentPos()),std::optional<CompilationError>());
                    }
                    auto ch = current_char.value();
                    if(ch=='/'){

                        ss.str("");

                        current_state=DFAState::ANNOTATION_1_STATE;
                        break;
                    }
                    else if(ch=='*'){
                        ss.str("");
                        current_state=DFAState::ANNOTATION_2_STATE;
                        break;
                    }
                    else{
                        unreadLast();
                        return std::make_pair(std::make_optional<Token>(TokenType::DIVISION_SIGN,'/',pos,currentPos()),std::optional<CompilationError>());
                    }
                }
                case ANNOTATION_1_STATE:{//  //
                    if(!current_char.has_value()){
                        current_state=DFAState ::INITIAL_STATE;
                        break;
                    }
                    auto ch=current_char.value();
                    if(ch==10||ch==13){
                        unreadLast();
                        current_state=DFAState ::INITIAL_STATE;
                    }
                    break;
                }
                case ANNOTATION_2_STATE:{//  /*
                    if(!current_char.has_value()){
                        return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos,ErrorCode::ErrAnnotationUnmatched));
                    }
                    auto ch=current_char.value();
                    if(ch=='*'){
                        current_state=DFAState ::ANNOTATION_3_STATE;
                    }
                    break;
                }
                case ANNOTATION_3_STATE:{//  /*...*
                    if(!current_char.has_value()){
                        return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos,ErrorCode::ErrAnnotationUnmatched));
                    }
                    auto ch=current_char.value();
                    if(ch=='/'){
                        current_state=DFAState ::INITIAL_STATE;
                    }
                    else{
                        current_state=DFAState::ANNOTATION_2_STATE;
                    }
                    break;
                }

                case MULTIPLICATION_SIGN_STATE:{
                    unreadLast();
                    return std::make_pair(std::make_optional<Token>(TokenType::MULTIPLICATION_SIGN,'*',pos,currentPos()),std::optional<CompilationError>());
                }

                case ASSIGNMENT_SIGN_STATE:{
                    if (!current_char.has_value()){
                        return std::make_pair(std::make_optional<Token>(TokenType::ASSIGNMENT_SIGN,'=',pos,currentPos()),std::optional<CompilationError>());
                    }
                    auto ch = current_char.value();
                    if(ch=='='){
                        std::string string_token;
                        ss<<ch;
                        ss>>string_token;
                        return std::make_pair(std::make_optional<Token>(TokenType::EQUAL_SIGN,string_token,pos,currentPos()),std::optional<CompilationError>());
                    }
                    else{
                        unreadLast();
                        return std::make_pair(std::make_optional<Token>(TokenType::ASSIGNMENT_SIGN,'=',pos,currentPos()),std::optional<CompilationError>());
                    }
                }

                case SEMICOLON_STATE:{
                    unreadLast();
                    return std::make_pair(std::make_optional<Token>(TokenType::SEMICOLON,';',pos,currentPos()),std::optional<CompilationError>());
                }
                case COMMA_STATE:{
                    unreadLast();
                    return std::make_pair(std::make_optional<Token>(TokenType::COMMA,',',pos,currentPos()),std::optional<CompilationError>());
                }
                case LEFT_BRACKET_STATE:{
                    unreadLast();
                    return std::make_pair(std::make_optional<Token>(TokenType::LEFT_BRACKET,'(',pos,currentPos()),std::optional<CompilationError>());
                }
                case RIGHT_BRACKET_STATE:{
                    unreadLast();
                    return std::make_pair(std::make_optional<Token>(TokenType::RIGHT_BRACKET,')',pos,currentPos()),std::optional<CompilationError>());
                }
                case LEFT_BRACE_STATE:{
                    unreadLast();
                    return std::make_pair(std::make_optional<Token>(TokenType::LEFT_BRACE,'{',pos,currentPos()),std::optional<CompilationError>());
                }
                case RIGHT_BRACE_STATE:{
                    unreadLast();
                    return std::make_pair(std::make_optional<Token>(TokenType::RIGHT_BRACE,'}',pos,currentPos()),std::optional<CompilationError>());
                }
                case LESS_SIGN_STATE:{
                    if (!current_char.has_value()){
                        return std::make_pair(std::make_optional<Token>(TokenType::LESS_SIGN,'<',pos,currentPos()),std::optional<CompilationError>());
                    }
                    auto ch = current_char.value();
                    if(ch=='='){
                        std::string string_token;
                        ss<<ch;
                        ss>>string_token;
                        return std::make_pair(std::make_optional<Token>(TokenType::LESS_EQUAL_SIGN,string_token,pos,currentPos()),std::optional<CompilationError>());
                    }
                    else{
                        unreadLast();
                        return std::make_pair(std::make_optional<Token>(TokenType::LESS_SIGN,'<',pos,currentPos()),std::optional<CompilationError>());
                    }
                }
                case GREATER_SIGN_STATE:{
                    if (!current_char.has_value()){
                        return std::make_pair(std::make_optional<Token>(TokenType::GREATER_SIGN,'>',pos,currentPos()),std::optional<CompilationError>());
                    }
                    auto ch = current_char.value();
                    if(ch=='='){
                        std::string string_token;
                        ss<<ch;
                        ss>>string_token;
                        return std::make_pair(std::make_optional<Token>(TokenType::GREATER_EQUAL_SIGN,string_token,pos,currentPos()),std::optional<CompilationError>());
                    }
                    else{
                        unreadLast();
                        return std::make_pair(std::make_optional<Token>(TokenType::GREATER_SIGN,'>',pos,currentPos()),std::optional<CompilationError>());
                    }
                }
                case EXCLAMATION_SIGN_STATE:{
                    if (!current_char.has_value()){
                        //读到'!'时已经到文件结尾
                        return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos,ErrorCode::ErrEOF));
                    }
                    auto ch = current_char.value();
                    if(ch=='='){
                        std::string string_token;
                        ss<<ch;
                        ss>>string_token;
                        return std::make_pair(std::make_optional<Token>(TokenType::NOT_EQUAL_SIGN,string_token,pos,currentPos()),std::optional<CompilationError>());
                    }
                    else{
                        unreadLast();
                        return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos,ErrorCode::ErrInvalidOperator));
                    }
                }

                    // 预料之外的状态，如果执行到了这里，说明程序异常
                default:
                    DieAndPrint("unhandled state.");
                    break;
            }
        }
        // 预料之外的状态，如果执行到了这里，说明程序异常
        return std::make_pair(std::optional<Token>(), std::optional<CompilationError>());
    }

    std::optional<CompilationError> Tokenizer::checkToken(const Token& t) {
        auto val = t.GetValueString();
//        std::cout<<"here,error!val=="<<val<<std::endl;
        switch (t.GetType()) {
            case IDENTIFIER: {
                if (miniplc0::isdigit(val[0])){
//                    std::cout<<"here,error1!"<<std::endl;
                    return std::make_optional<CompilationError>(t.GetStartPos().first, t.GetStartPos().second, ErrorCode::ErrInvalidIdentifier);
                }
                break;
            }
            case INT_DECIMAL:{
                if (val[0] == '0' && val.length() > 1){
//                    std::cout<<"here,error2!"<<std::endl;
                    return std::make_optional<CompilationError>(t.GetStartPos().first, t.GetStartPos().second, ErrorCode::ErrInvalidIntegerLiteral);
                }
                break;
            }
            case INT_HEXADECIMAL:{
                if(val.length()==2&&(val[1]=='X'||val[1]=='x')){
//                    std::cout<<"here,error3!"<<std::endl;
                    return std::make_optional<CompilationError>(t.GetStartPos().first, t.GetStartPos().second, ErrorCode::ErrInvalidIntegerLiteral);
                }
                else{
                    for(unsigned int i=2;i<val.length();i++){
                        if(miniplc0::isdigit(val[i])||(val[i]>='a'&&val[i]<='f')||(val[i]>='A'&&val[i]<='F')){
                            continue;
                        }
                        else{
                            return std::make_optional<CompilationError>(t.GetStartPos().first, t.GetStartPos().second, ErrorCode::ErrInvalidIntegerLiteral);
                        }
                    }
                }
                break;
            }

            default:
                break;
        }
        return {};
    }

    void Tokenizer::readAll() {
        if (_initialized)
            return;
        for (std::string tp; std::getline(_rdr, tp);)
            _lines_buffer.emplace_back(std::move(tp + "\n"));
        _initialized = true;
        _ptr = std::make_pair<int64_t, int64_t>(0, 0);
        return;
    }

    // Note: We allow this function to return a postion which is out of bound according to the design like std::vector::end().
    std::pair<uint64_t, uint64_t> Tokenizer::nextPos() {
        if (_ptr.first >= _lines_buffer.size())
            DieAndPrint("advance after EOF");
        if (_ptr.second == _lines_buffer[_ptr.first].size() - 1)
            return std::make_pair(_ptr.first + 1, 0);
        else
            return std::make_pair(_ptr.first, _ptr.second + 1);
    }

    std::pair<uint64_t, uint64_t> Tokenizer::currentPos() {
        return _ptr;
    }

    std::pair<uint64_t, uint64_t> Tokenizer::previousPos() {
        if (_ptr.first == 0 && _ptr.second == 0)
            DieAndPrint("previous position from beginning");
        if (_ptr.second == 0)
            return std::make_pair(_ptr.first - 1, _lines_buffer[_ptr.first - 1].size() - 1);
        else
            return std::make_pair(_ptr.first, _ptr.second - 1);
    }

    std::optional<char> Tokenizer::nextChar() {
        if (isEOF())
            return {}; // EOF
        auto result = _lines_buffer[_ptr.first][_ptr.second];
        _ptr = nextPos();
        return result;
    }

    int Tokenizer::Hex2Dec(char ch){
        int ans=0;
        if(ch>='0'&&ch<='9'){
            ans=ch-'0';
        }
        else if(ch>='a'&&ch<='f'){
            ans=ch-'a'+10;
        }
        else if(ch>='A'&&ch<='F'){
            ans=ch-'A'+10;
        }
        return ans;
    }

    bool Tokenizer::isEOF() {
        return _ptr.first >= _lines_buffer.size();
    }

    // Note: Is it evil to unread a buffer?
    void Tokenizer::unreadLast() {
        _ptr = previousPos();
    }
}
