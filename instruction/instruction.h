#pragma once

#include <cstdint>
#include <utility>

namespace miniplc0 {

	enum Operation {
		NOP=0,
        BIPUSH,
        IPUSH,
        POP=0x04,
        POP2,
        POPN,
        DUP,
        DUP2,
        LOADC,
        LOADA=0x0a,
        NEW,
        SNEW,
        ILOAD=0x10,
        DLOAD,
        ALOAD,
        IALOAD=0x18,
        DALOAD,
        AALOAD,
        ISTORE=0x20,
        DSTORE,
        ASTORE,
        IASTORE=0x28,
        DASTORE,
        AASTORE,
        IADD=0x30,
        DADD,
        ISUB=0x34,
        DSUB,
        IMUL=0x38,
        DMUL,
        IDIV=0x3c,
        DDIV,
        INEG=0x40,
        DNEG,
        ICMP=0x44,
        DCMP,
        I2D=0x60,
        D2I,
        I2C,
        JMP=0x70,
        JE,
        JNE,
        JL,
        JGE,
        JG,
        JLE,
        CALL=0x80,
        RET=0x88,
        IRET,
        DRET,
        ARET,
        IPRINT=0xa0,
        DPRINT,
        CPRINT,
        SPRINT,
        PRINTL=0xaf,
        ISCAN=0xb0,
        DSCAN,
        CSCAN

	};
	
	class Instruction final {
	private:
		using int32_t = std::int32_t;
	public:
		friend void swap(Instruction& lhs, Instruction& rhs);
	public:
		Instruction(Operation opr, int32_t x,int32_t y) : _opr(opr), _x(x), _y(y) {}
		
		Instruction() : Instruction(Operation::NOP, 0,0){}
		Instruction(const Instruction& i) { _opr = i._opr; _x = i._x; _y=i._y;}
		Instruction(Instruction&& i) :Instruction() { swap(*this, i); }
		Instruction& operator=(Instruction i) { swap(*this, i); return *this; }
		bool operator==(const Instruction& i) const { return _opr == i._opr && _x == i._x && _y==i._y; }

		Operation GetOperation() const { return _opr; }
		int32_t GetX() const { return _x; }
        int32_t GetY() const { return _y; }

        void SetX(int32_t x) { _x=x; }
        void SetY(int32_t y) { _y=y; }
	private:
		Operation _opr;
		int32_t _x;
		int32_t _y;
	};

	inline void swap(Instruction& lhs, Instruction& rhs) {
		using std::swap;
		swap(lhs._opr, rhs._opr);
		swap(lhs._x, rhs._x);
        swap(lhs._y, rhs._y);
	}
}