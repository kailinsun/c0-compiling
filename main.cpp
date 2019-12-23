#include "argparse.hpp"
#include "fmt/core.h"

#include "tokenizer/tokenizer.h"
#include "analyser/analyser.h"
#include "fmts.hpp"

#include <iostream>
#include <fstream>

unsigned int u2ChangeToBigEnd(unsigned int n){//uint16_t
    auto *p=(unsigned char*)&n;
    return (((unsigned int)*p<<8)+((unsigned int)*(p+1)));
}
unsigned int u4ChangeToBigEnd(unsigned int n){//uint16_t
    auto *p=(unsigned char*)&n;
    return (((unsigned int)*p<<24) + ((unsigned int)*(p+1)<<16) + ((unsigned int)*(p+2)<<8) + ((unsigned int)*(p+3)));
}
unsigned int opCount(miniplc0::Instruction &p){
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
            return 0;
        case miniplc0::BIPUSH:
            return 0x10;
        case miniplc0::IPUSH:
        case miniplc0::POPN:
            return 0x40;
        case miniplc0::LOADC:
            return 0x20;
        case miniplc0::SNEW:
            return 0x40;
        case miniplc0::JMP:
        case miniplc0::JE:
        case miniplc0::JNE:
        case miniplc0::JL:
        case miniplc0::JGE:
        case miniplc0::JG:
        case miniplc0::JLE:
        case miniplc0::CALL:
            return 0x20;
        case miniplc0::LOADA:
            return 0x24;
    }
    return -1;
}

std::vector<miniplc0::Token> _tokenize(std::istream& input) {
	miniplc0::Tokenizer tkz(input);
	auto p = tkz.AllTokens();
	if (p.second.has_value()) {
		fmt::print(stderr, "Tokenization error: {}\n", p.second.value());
		exit(2);
	}
	return p.first;
}

void Tokenize(std::istream& input, std::ostream& output) {
	auto v = _tokenize(input);
	for (auto& it : v)
		output << fmt::format("{}\n", it);
	return;
}

void Analyse(std::istream& input, std::ostream& output){
	auto tks = _tokenize(input);
	miniplc0::Analyser analyser(tks);
	auto p = analyser.Analyse();
	if (p.second.has_value()) {
		fmt::print(stderr, "Syntactic analysis error: {}\n", p.second.value());
		exit(2);
	}

    output<<".constants:"<<std::endl;
    auto _fu=analyser.getFunctionTable();
    int nfu=_fu.size();
    for(int i=0;i<nfu;i++){
        output<<i<<"  ";
        output<<_fu[i]._type<<"  ";
        output<<"\""<<_fu[i]._value<<"\""<<std::endl;
    }

    output<<".start:"<<std::endl;
	auto _st=analyser.getStartCode();
	unsigned int ti=0;
    for (auto& it : _st){
        output <<ti<<" "<< fmt::format("{}", it)<<std::endl;
        ti++;
    }

    output<<".functions:"<<std::endl;
    _fu=analyser.getFunctionTable();
    nfu=_fu.size();
    for(int i=0;i<nfu;i++){
        output<<i<<"  ";
        output<<i<<"  ";
        output<<_fu[i]._params_size<<"  ";
        output<<_fu[i]._level<<std::endl;
    }

    auto v = p.first;
    int nv=v.size();
    for(int i=0;i<nv;i++){
        output << ".F" <<i<<":"<<std::endl;
        int j=0;
        for (auto& it : v[i]._funins){
            output <<j<<" "<<fmt::format("{}", it)<<std::endl;
            j++;
        }
    }

//    auto _va=analyser.getVarTable();
//    int nva=_va.size();
//    for(int i=0;i<nva;i++){
//        std::cout<<"name:"<<_va[i]._name<<"  ";
//        std::cout<<"type:"<<_va[i]._type<<"  ";
//        std::cout<<"level:"<<_va[i]._level<<"  ";
//        std::cout<<"address:"<<_va[i]._address<<"  "<<std::endl;
//    }

	return;
}

void AnalyseBinary(std::istream& input, std::ostream& output){
    auto tks = _tokenize(input);
    miniplc0::Analyser analyser(tks);
    auto p = analyser.Analyse();
    if (p.second.has_value()) {
        fmt::print(stderr, "Syntactic analysis error: {}\n", p.second.value());
        exit(2);
    }


    auto _fun=analyser.getFunctionTable();
    auto _st=analyser.getStartCode();
    //u4 magic;             must be 0x43303A29
    unsigned int magic=0x43303A29;
    magic=u4ChangeToBigEnd(magic);
    output.write((char*)&magic,sizeof(int32_t));
    //u4 version;
    unsigned int version=1;
    version=u4ChangeToBigEnd(version);
    output.write((char*)&version,sizeof(int32_t));
    //u2 constants_count;
    unsigned int constants_count=_fun.size();
    constants_count=u2ChangeToBigEnd(constants_count);
    output.write((char*)&constants_count,sizeof(int16_t));
    output<<std::flush;
    //Constant_info constants[constants_count];
    for(unsigned int i=0;i<_fun.size();i++){
        //u1 type;u2 length;u1 value[length];
        unsigned int type=0;
        output.write((char *)&type, sizeof(int8_t));
        unsigned int length=_fun[i]._value.length();
        length=u2ChangeToBigEnd(length);
        output.write((char*)&length,sizeof(int16_t));
        output<<_fun[i]._value<<std::flush;
    }
    //Start_code_info start_code;
    //u2 instructions_count;
    unsigned int instructions_count_start=_st.size();
    instructions_count_start=u2ChangeToBigEnd(instructions_count_start);
    output.write((char *)&instructions_count_start, sizeof(int16_t));
    output<<std::flush;
    //Instruction instructions[instructions_count];
    for(unsigned int i=0;i<_st.size();i++){
        unsigned opcode=_st[i].GetOperation();
        output.write((char *)&opcode, sizeof(int8_t));
        if(opCount(_st[i])==0){
        }
        else if(opCount(_st[i])==0x10){
            unsigned oprands=_st[i].GetX();
            output.write((char *)&oprands, sizeof(int8_t));
        }
        else if(opCount(_st[i])==0x20){
            unsigned oprands=_st[i].GetX();
            oprands=u2ChangeToBigEnd(oprands);
            output.write((char *)&oprands, sizeof(int16_t));
        }
        else if(opCount(_st[i])==0x40){
            unsigned oprands=_st[i].GetX();
            oprands=u4ChangeToBigEnd(oprands);
            output.write((char *)&oprands, sizeof(int32_t));
        }
        else if(opCount(_st[i])==0x24){
            unsigned oprands=_st[i].GetX();
            oprands=u2ChangeToBigEnd(oprands);
            output.write((char *)&oprands, sizeof(int16_t));
            oprands=_st[i].GetY();
            oprands=u4ChangeToBigEnd(oprands);
            output.write((char *)&oprands, sizeof(int32_t));
        }
        output<<std::flush;
    }
    //u2 functions_count;
    unsigned int functions_count=_fun.size();
    functions_count=u2ChangeToBigEnd(functions_count);
    output.write((char *)&functions_count, sizeof(int16_t));
    output<<std::flush;


    auto _fun_body = p.first;
    //Function_info functions[functions_count];
    //u2 name_index; // name: CO_binary_file.strings[name_index]
    //u2 params_size;
    //u2 level;
    //u2 instructions_count;
    //Instruction instructions[instructions_count];
    for(unsigned int i=0;i<_fun.size();i++){
        unsigned int name_index=i;
        name_index=u2ChangeToBigEnd(name_index);
        output.write((char *)&name_index, sizeof(int16_t));
        unsigned int param_size=_fun[i]._params_size;
        param_size=u2ChangeToBigEnd(param_size);
        output.write((char *)&param_size, sizeof(int16_t));
        unsigned int level=_fun[i]._level;
        level=u2ChangeToBigEnd(level);
        output.write((char *)&level, sizeof(int16_t));
        unsigned int instructions_count=_fun_body[i]._funins.size();
        instructions_count=u2ChangeToBigEnd(instructions_count);
        output.write((char *)&instructions_count, sizeof(int16_t));
        for(unsigned int j=0;j<_fun_body[i]._funins.size();j++){
            unsigned opcode=_fun_body[i]._funins[j].GetOperation();
            output.write((char *)&opcode, sizeof(int8_t));
            if(opCount(_fun_body[i]._funins[j])==0){
            }
            else if(opCount(_fun_body[i]._funins[j])==0x10){
                unsigned oprands=_fun_body[i]._funins[j].GetX();
                output.write((char *)&oprands, sizeof(int8_t));
            }
            else if(opCount(_fun_body[i]._funins[j])==0x20){
                unsigned oprands=_fun_body[i]._funins[j].GetX();
                oprands=u2ChangeToBigEnd(oprands);
                output.write((char *)&oprands, sizeof(int16_t));
            }
            else if(opCount(_fun_body[i]._funins[j])==0x40){
                unsigned oprands=_fun_body[i]._funins[j].GetX();
                oprands=u4ChangeToBigEnd(oprands);
                output.write((char *)&oprands, sizeof(int32_t));
            }
            else if(opCount(_fun_body[i]._funins[j])==0x24){
                unsigned oprands=_fun_body[i]._funins[j].GetX();
                output.write((char *)&oprands, sizeof(int16_t));
                oprands=u2ChangeToBigEnd(oprands);
                oprands=_fun_body[i]._funins[j].GetY();
                oprands=u4ChangeToBigEnd(oprands);
                output.write((char *)&oprands, sizeof(int32_t));
            }
            output<<std::flush;
        }
    }

    return;
}

int main(int argc, char** argv) {
	argparse::ArgumentParser program("cc0");
	program.add_argument("input")
		.help("speicify the file to be compiled.");
//	program.add_argument("-t")
//		.default_value(false)
//		.implicit_value(true)
//		.help("perform tokenization for the input file.");
//	program.add_argument("-l")
//		.default_value(false)
//		.implicit_value(true)
//		.help("perform syntactic analysis for the input file.");
    program.add_argument("-s")
            .default_value(false)
            .implicit_value(true)
            .help("Translate the input C0 source code into a text assembly file.");
    program.add_argument("-c")
            .default_value(false)
            .implicit_value(true)
            .help("Translate the input C0 source code into binary target file.");
	program.add_argument("-o", "--output")
		.required()
		.default_value(std::string("-"))
		.help("specify the output file.");

	try {
		program.parse_args(argc, argv);
	}
	catch (const std::runtime_error& err) {
		fmt::print(stderr, "{}\n\n", err.what());
		program.print_help();
		exit(2);
	}

	auto input_file = program.get<std::string>("input");
	auto output_file = program.get<std::string>("--output");
	std::istream* input;
	std::ostream* output;
	std::ifstream inf;
	std::ofstream outf;
	if (input_file != "-") {
		inf.open(input_file, std::ios::in);
		if (!inf) {
			fmt::print(stderr, "Fail to open {} for reading.\n", input_file);
			exit(2);
		}
		input = &inf;
	}
	else
		input = &std::cin;
//	if (output_file != "-") {
//		outf.open(output_file, std::ios::out | std::ios::trunc);
//		if (!outf) {
//			fmt::print(stderr, "Fail to open {} for writing.\n", output_file);
//			exit(2);
//		}
//		output = &outf;
//	}
//	else
//		output = &std::cout;
//
//	if (program["-t"] == true && program["-l"] == true) {
//		fmt::print(stderr, "You can only perform tokenization or syntactic analysis at one time.");
//		exit(2);
//	}
//	if (program["-t"] == true) {
//		Tokenize(*input, *output);
//	}
//	else if (program["-l"] == true) {
//		Analyse(*input, *output);
//	}


    if (program["-s"] == true && program["-c"] == true) {
        fmt::print(stderr, "You can only perform -s or -c at one time.");
        exit(2);
    }
    if (program["-s"] == true) {
        if(output_file!="-"){
            outf.open(output_file, std::ios::out | std::ios::trunc);
            if (!outf) {
                fmt::print(stderr, "Fail to open {} for writing.\n", output_file);
                exit(2);
            }
            output = &outf;
        }
        else{
            outf.open("out", std::ios::out | std::ios::trunc);
            if (!outf) {
                fmt::print(stderr, "Fail to open {} for writing.\n", output_file);
                exit(2);
            }
            output = &outf;
        }
        Analyse(*input, *output);
    }
    else if (program["-c"] == true) {
        if(output_file!="-"){
            outf.open(output_file, std::ios::out | std::ios::trunc| std::ios::binary);
            if (!outf) {
                fmt::print(stderr, "Fail to open {} for writing.\n", output_file);
                exit(2);
            }
            output = &outf;
        }
        else{
            outf.open("out", std::ios::out | std::ios::trunc| std::ios::binary);
            if (!outf) {
                fmt::print(stderr, "Fail to open {} for writing.\n", output_file);
                exit(2);
            }
            output = &outf;
        }
        AnalyseBinary(*input, *output);
    }
	else {
		fmt::print(stderr, "You must choose tokenization or syntactic analysis.");
		exit(2);
	}
	exit(0);
}