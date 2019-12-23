
#ifndef CC0_SYSTABLE_H
#define CC0_SYSTABLE_H

#include <cstdint>
#include <string>
#include <vector>
#include "instruction/instruction.h"

namespace miniplc0 {
    using int32_t = std::int32_t;
    using string = std::string;

    class variableTable{//符号表

    public:
        variableTable(string name,int32_t type,int32_t level,int32_t address):
            _name(name),_type(type),_level(level),_address(address){}
        variableTable():variableTable("",-1,-1,-1){}

        string getName(){ return _name;}
        int32_t getType(){ return _type;}
//        string getValue(){ return _value;}
        int32_t getLevel(){ return _level;}
        int32_t getAddress(){ return _address;}

        void setType(int32_t type){ _type=type;}
//        void setValue(string value){ _value=value;}
        void setAddress(int32_t address){ _address=address;}
    public:
        string _name;
        int32_t _type;       //各个变量的类型,0为常量,1为未赋值变量，2为已赋值变量,3为函数
//        string _value;      //常量变量的值，
        int32_t _level;     //层级,0为全局，1为局部
        int32_t _address;   //在栈中的位置
    };


    class functionsTable{//函数表和常量表合二为一
    public:
        functionsTable(string type,int32_t params_size,int32_t level,string value):
            _type(type),_params_size(params_size),_level(level),_value(value),_haveReturnValue(-1){}
    public:
//        int32_t name_index;     // 函数名在.constants中的下标
        string _type;
        int32_t _params_size;    //参数占用的slot数
        int32_t _level;          //函数嵌套的层级
        string _value;
        int32_t _haveReturnValue;
    };





    class functionBodyTable{
    public:
        std::vector<Instruction> _funins;
    };


}

#endif //CC0_SYSTABLE_H
