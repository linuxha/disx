// dis68HC16.cpp

// hints usage:
//    16-bit immediate: enables reference
//    branches: BCC/BCS change to BHS/BLO

static const char versionName[] = "Motorola 68HC16 disassembler";

#include "discpu.h"

class Dis68HC16 : public CPU {
public:
    Dis68HC16(const char *name, int subtype, int endian, int addrwid,
              char curAddrChr, char hexChr, const char *byteOp,
              const char *wordOp, const char *longOp);

    virtual int dis_line(addr_t addr, char *opcode, char *parms, int &lfref, addr_t &refaddr);

private:
    void hint_opcode(int hint, addr_t addr, char *opcode);
};


Dis68HC16 cpu_68HC16("68HC16", 0, BIG_END, ADDR_24, '*', '$', "FCB", "FDB", "DL");


enum InstType {
    iInvalid = 0,
    iInherent,
    i8Immediate,
    i16Immediate,
    iMAC,
    iPSH,
    iPUL,
    iMOV,
    iMOVEXT,
    iExtended,
    i20Extended,
    i8Relative,
    i16Relative,
    ibExtRelative,
    ib8XRelative,
    ib8YRelative,
    ib8ZRelative,
    ib16XRelative,
    ib16YRelative,
    ib16ZRelative,
    ibExtended,
    ib8XIndexed,
    ib8YIndexed,
    ib8ZIndexed,
    ibWExtended,
    ib16XIndexed,
    ib16YIndexed,
    ib16ZIndexed,
    i8XIndexed,
    i8YIndexed,
    i8ZIndexed,
    i16XIndexed,
    i16YIndexed,
    i16ZIndexed,
    i20XIndexed,
    i20YIndexed,
    i20ZIndexed,
    iEXIndexed,
    iEYIndexed,
    iEZIndexed,
};


Dis68HC16::Dis68HC16(const char *name, int subtype, int endian, int addrwid,
                     char curAddrChr, char hexChr, const char *byteOp,
                     const char *wordOp, const char *longOp)
{
    _name    = name;
    _version = versionName;
    _subtype = subtype;
    _dbopcd  = byteOp;
    _dwopcd  = wordOp;
    _dlopcd  = longOp;
    _curpc   = curAddrChr;
    _endian  = endian;
    _hexchr  = hexChr;
    _addrwid = addrwid;
    _usefcc  = true;

    add_cpu();
}


// =====================================================


struct InstrRec {
    const char      *op;    // mnemonic
    enum InstType   typ;    // typ
    uint8_t         lfref;  // lfFlag/refFlag/codeRef
};
typedef const struct InstrRec *InstrPtr;

static const struct InstrRec M68HC16_opcdTable[] =
{
        // op        typ            lfref
/*00*/  {"COM"  , i8XIndexed  , 0 },
/*01*/  {"DEC"  , i8XIndexed  , 0 },
/*02*/  {"NEG"  , i8XIndexed  , 0 },
/*03*/  {"INC"  , i8XIndexed  , 0 },
/*04*/  {"ASL"  , i8XIndexed  , 0 },
/*05*/  {"CLR"  , i8XIndexed  , 0 },
/*06*/  {"TST"  , i8XIndexed  , 0 },
/*07*/  {""     , iInvalid    , 0 },
/*08*/  {"BCLR" , ib16XIndexed, 0 },
/*09*/  {"BSET" , ib16XIndexed, 0 },
/*0A*/  {"BRCLR", ib16XRelative,0 },
/*0B*/  {"BRSET", ib16XRelative,0 },
/*0C*/  {"ROL"  , i8XIndexed  , 0 },
/*0D*/  {"ASR"  , i8XIndexed  , 0 },
/*0E*/  {"ROR"  , i8XIndexed  , 0 },
/*0F*/  {"LSR"  , i8XIndexed  , 0 },

/*10*/  {"COM"  , i8YIndexed  , 0 },
/*11*/  {"DEC"  , i8YIndexed  , 0 },
/*12*/  {"NEG"  , i8YIndexed  , 0 },
/*13*/  {"INC"  , i8YIndexed  , 0 },
/*14*/  {"ASL"  , i8YIndexed  , 0 },
/*15*/  {"CLR"  , i8YIndexed  , 0 },
/*16*/  {"TST"  , i8YIndexed  , 0 },
/*17*/  {""     , iInvalid    , 0 }, // 17 pre-byte
/*18*/  {"BCLR" , ib16YIndexed, 0 },
/*19*/  {"BSET" , ib16YIndexed, 0 },
/*1A*/  {"BRCLR", ib16YRelative,0 },
/*1B*/  {"BRSET", ib16YRelative,0 },
/*1C*/  {"ROL"  , i8YIndexed  , 0 },
/*1D*/  {"ASR"  , i8YIndexed  , 0 },
/*1E*/  {"ROR"  , i8YIndexed  , 0 },
/*1F*/  {"LSR"  , i8YIndexed  , 0 },

/*20*/  {"COM"  , i8ZIndexed  , 0 },
/*21*/  {"DEC"  , i8ZIndexed  , 0 },
/*22*/  {"NEG"  , i8ZIndexed  , 0 },
/*23*/  {"INC"  , i8ZIndexed  , 0 },
/*24*/  {"ASL"  , i8ZIndexed  , 0 },
/*25*/  {"CLR"  , i8ZIndexed  , 0 },
/*26*/  {"TST"  , i8ZIndexed  , 0 },
/*27*/  {""     , iInvalid    , 0 }, // 27 pre-byte
/*28*/  {"BCLR" , ib16ZIndexed, 0 },
/*29*/  {"BSET" , ib16ZIndexed, 0 },
/*2A*/  {"BRCLR", ib16ZRelative,0 },
/*2B*/  {"BRSET", ib16ZRelative,0 },
/*2C*/  {"ROL"  , i8ZIndexed  , 0 },
/*2D*/  {"ASR"  , i8ZIndexed  , 0 },
/*2E*/  {"ROR"  , i8ZIndexed  , 0 },
/*2F*/  {"LSR"  , i8ZIndexed  , 0 },

/*30*/  {"MOVB" , iMOV        , 0 },
/*31*/  {"MOVW" , iMOV        , 0 },
/*32*/  {"MOVB" , iMOV        , 0 },
/*33*/  {"MOVW" , iMOV        , 0 },
/*34*/  {"PSHM" , iPSH        , 0 },
/*35*/  {"PULM" , iPUL        , 0 },
/*36*/  {"BSR"  , i8Relative  , REFFLAG | CODEREF },
/*37*/  {""     , iInvalid    , 0 }, // 37 pre-byte
/*38*/  {"BCLR" , ibExtended  , 0 },
/*39*/  {"BSET" , ibExtended  , 0 },
/*3A*/  {"BRCLR", ibExtRelative,0 },
/*3B*/  {"BRSET", ibExtRelative,0 },
/*3C*/  {"AIX"  , i8Immediate , 0 },
/*3D*/  {"AIY"  , i8Immediate , 0 },
/*3E*/  {"AIZ"  , i8Immediate , 0 },
/*3F*/  {"AIS"  , i8Immediate , 0 },

/*40*/  {"SUBA" , i8XIndexed  , 0 },
/*41*/  {"ADDA" , i8XIndexed  , 0 },
/*42*/  {"SBCA" , i8XIndexed  , 0 },
/*43*/  {"ADCA" , i8XIndexed  , 0 },
/*44*/  {"EORA" , i8XIndexed  , 0 },
/*45*/  {"LDAA" , i8XIndexed  , 0 },
/*46*/  {"ANDA" , i8XIndexed  , 0 },
/*47*/  {"ORAA" , i8XIndexed  , 0 },
/*48*/  {"CMPA" , i8XIndexed  , 0 },
/*49*/  {"BITA" , i8XIndexed  , 0 },
/*4A*/  {"STAA" , i8XIndexed  , 0 },
/*4B*/  {"JMP"  , i20XIndexed , LFFLAG },
/*4C*/  {"CPX"  , i8XIndexed  , 0 },
/*4D*/  {"CPY"  , i8XIndexed  , 0 },
/*4E*/  {"CPZ"  , i8XIndexed  , 0 },
/*4F*/  {"CPS"  , i8XIndexed  , 0 },

/*50*/  {"SUBA" , i8YIndexed  , 0 },
/*51*/  {"ADDA" , i8YIndexed  , 0 },
/*52*/  {"SBCA" , i8YIndexed  , 0 },
/*53*/  {"ADCA" , i8YIndexed  , 0 },
/*54*/  {"EORA" , i8YIndexed  , 0 },
/*55*/  {"LDAA" , i8YIndexed  , 0 },
/*56*/  {"ANDA" , i8YIndexed  , 0 },
/*57*/  {"ORAA" , i8YIndexed  , 0 },
/*58*/  {"CMPA" , i8YIndexed  , 0 },
/*59*/  {"BITA" , i8YIndexed  , 0 },
/*5A*/  {"STAA" , i8YIndexed  , 0 },
/*5B*/  {"JMP"  , i20YIndexed , LFFLAG },
/*5C*/  {"CPX"  , i8YIndexed  , 0 },
/*5D*/  {"CPY"  , i8YIndexed  , 0 },
/*5E*/  {"CPZ"  , i8YIndexed  , 0 },
/*5F*/  {"CPS"  , i8YIndexed  , 0 },

/*60*/  {"SUBA" , i8ZIndexed  , 0 },
/*61*/  {"ADDA" , i8ZIndexed  , 0 },
/*62*/  {"SBCA" , i8ZIndexed  , 0 },
/*63*/  {"ADCA" , i8ZIndexed  , 0 },
/*64*/  {"EORA" , i8ZIndexed  , 0 },
/*65*/  {"LDAA" , i8ZIndexed  , 0 },
/*66*/  {"ANDA" , i8ZIndexed  , 0 },
/*67*/  {"ORAA" , i8ZIndexed  , 0 },
/*68*/  {"CMPA" , i8ZIndexed  , 0 },
/*69*/  {"BITA" , i8ZIndexed  , 0 },
/*6A*/  {"STAA" , i8ZIndexed  , 0 },
/*6B*/  {"JMP"  , i20ZIndexed , LFFLAG },
/*6C*/  {"CPX"  , i8ZIndexed  , 0 },
/*6D*/  {"CPY"  , i8ZIndexed  , 0 },
/*6E*/  {"CPZ"  , i8ZIndexed  , 0 },
/*6F*/  {"CPS"  , i8ZIndexed  , 0 },

/*70*/  {"SUBA" , i8Immediate , 0 },
/*71*/  {"ADDA" , i8Immediate , 0 },
/*72*/  {"SBCA" , i8Immediate , 0 },
/*73*/  {"ADCA" , i8Immediate , 0 },
/*74*/  {"EORA" , i8Immediate , 0 },
/*75*/  {"LDAA" , i8Immediate , 0 },
/*76*/  {"ANDA" , i8Immediate , 0 },
/*77*/  {"ORAA" , i8Immediate , 0 },
/*78*/  {"CMPA" , i8Immediate , 0 },
/*79*/  {"BITA" , i8Immediate , 0 },
/*7A*/  {"JMP"  , i20Extended , REFFLAG | CODEREF | LFFLAG },
/*7B*/  {"MAC"  , iMAC        , 0 },
/*7C*/  {"ADDE" , i8Immediate , 0 },
/*7D*/  {""     , iInvalid    , 0 },
/*7E*/  {""     , iInvalid    , 0 },
/*7F*/  {""     , iInvalid    , 0 },

/*80*/  {"SUBD" , i8XIndexed  , 0 },
/*81*/  {"ADDD" , i8XIndexed  , 0 },
/*82*/  {"SBCD" , i8XIndexed  , 0 },
/*83*/  {"ADCD" , i8XIndexed  , 0 },
/*84*/  {"EORD" , i8XIndexed  , 0 },
/*85*/  {"LDD"  , i8XIndexed  , 0 },
/*86*/  {"ANDD" , i8XIndexed  , 0 },
/*87*/  {"ORD"  , i8XIndexed  , 0 },
/*88*/  {"CPD"  , i8XIndexed  , 0 },
/*89*/  {"JSR"  , i20XIndexed , 0 },
/*8A*/  {"STD"  , i8XIndexed  , 0 },
/*8B*/  {"BRSET", ib8XRelative, 0 },
/*8C*/  {"STX"  , i8XIndexed  , 0 },
/*8D*/  {"STY"  , i8XIndexed  , 0 },
/*8E*/  {"STZ"  , i8XIndexed  , 0 },
/*8F*/  {"STS"  , i8XIndexed  , 0 },

/*90*/  {"SUBD" , i8YIndexed  , 0 },
/*91*/  {"ADDD" , i8YIndexed  , 0 },
/*92*/  {"SBCD" , i8YIndexed  , 0 },
/*93*/  {"ADCD" , i8YIndexed  , 0 },
/*94*/  {"EORD" , i8YIndexed  , 0 },
/*95*/  {"LDD"  , i8YIndexed  , 0 },
/*96*/  {"ANDD" , i8YIndexed  , 0 },
/*97*/  {"ORD"  , i8YIndexed  , 0 },
/*98*/  {"CPD"  , i8YIndexed  , 0 },
/*99*/  {"JSR"  , i20YIndexed , 0 },
/*9A*/  {"STD"  , i8YIndexed  , 0 },
/*9B*/  {"BRSET", ib8YRelative, 0 },
/*9C*/  {"STX"  , i8YIndexed  , 0 },
/*9D*/  {"STY"  , i8YIndexed  , 0 },
/*9E*/  {"STZ"  , i8YIndexed  , 0 },
/*9F*/  {"STS"  , i8YIndexed  , 0 },

/*A0*/  {"SUBD" , i8ZIndexed  , 0 },
/*A1*/  {"ADDD" , i8ZIndexed  , 0 },
/*A2*/  {"SBCD" , i8ZIndexed  , 0 },
/*A3*/  {"ADCD" , i8ZIndexed  , 0 },
/*A4*/  {"EORD" , i8ZIndexed  , 0 },
/*A5*/  {"LDD"  , i8ZIndexed  , 0 },
/*A6*/  {"ANDD" , i8ZIndexed  , 0 },
/*A7*/  {"ORD"  , i8ZIndexed  , 0 },
/*A8*/  {"CPD"  , i8ZIndexed  , 0 },
/*A9*/  {"JSR"  , i20ZIndexed , 0 },
/*AA*/  {"STD"  , i8ZIndexed  , 0 },
/*AB*/  {"BRSET", ib8ZRelative, 0 },
/*AC*/  {"STX"  , i8ZIndexed  , 0 },
/*AD*/  {"STY"  , i8ZIndexed  , 0 },
/*AE*/  {"STZ"  , i8ZIndexed  , 0 },
/*AF*/  {"STS"  , i8ZIndexed  , 0 },

/*B0*/  {"BRA"  , i8Relative  , REFFLAG | CODEREF | LFFLAG },
/*B1*/  {"BRN"  , i8Relative  , REFFLAG | CODEREF },
/*B2*/  {"BHI"  , i8Relative  , REFFLAG | CODEREF },
/*B3*/  {"BLS"  , i8Relative  , REFFLAG | CODEREF },
/*B4*/  {"BCC"  , i8Relative  , REFFLAG | CODEREF }, // aka BHS
/*B5*/  {"BCS"  , i8Relative  , REFFLAG | CODEREF }, // aka BLO
/*B6*/  {"BNE"  , i8Relative  , REFFLAG | CODEREF },
/*B7*/  {"BEQ"  , i8Relative  , REFFLAG | CODEREF },
/*B8*/  {"BVC"  , i8Relative  , REFFLAG | CODEREF },
/*B9*/  {"BVS"  , i8Relative  , REFFLAG | CODEREF },
/*BA*/  {"BPL"  , i8Relative  , REFFLAG | CODEREF },
/*BB*/  {"BMI"  , i8Relative  , REFFLAG | CODEREF },
/*BC*/  {"BGE"  , i8Relative  , REFFLAG | CODEREF },
/*BD*/  {"BLT"  , i8Relative  , REFFLAG | CODEREF },
/*BE*/  {"BGT"  , i8Relative  , REFFLAG | CODEREF },
/*BF*/  {"BLE"  , i8Relative  , REFFLAG | CODEREF },

/*C0*/  {"SUBB" , i8XIndexed  , 0 },
/*C1*/  {"ADDB" , i8XIndexed  , 0 },
/*C2*/  {"SBCB" , i8XIndexed  , 0 },
/*C3*/  {"ADCB" , i8XIndexed  , 0 },
/*C4*/  {"EORB" , i8XIndexed  , 0 },
/*C5*/  {"LDAB" , i8XIndexed  , 0 },
/*C6*/  {"ANDB" , i8XIndexed  , 0 },
/*C7*/  {"ORAB" , i8XIndexed  , 0 },
/*C8*/  {"CMPB" , i8XIndexed  , 0 },
/*C9*/  {"BITB" , i8XIndexed  , 0 },
/*CA*/  {"STAB" , i8XIndexed  , 0 },
/*CB*/  {"BRCLR", ib8XRelative, 0 },
/*CC*/  {"LDX"  , i8XIndexed  , 0 },
/*CD*/  {"LDY"  , i8XIndexed  , 0 },
/*CE*/  {"LDZ"  , i8XIndexed  , 0 },
/*CF*/  {"LDS"  , i8XIndexed  , 0 },

/*D0*/  {"SUBB" , i8YIndexed  , 0 },
/*D1*/  {"ADDB" , i8YIndexed  , 0 },
/*D2*/  {"SBCB" , i8YIndexed  , 0 },
/*D3*/  {"ADCB" , i8YIndexed  , 0 },
/*D4*/  {"EORB" , i8YIndexed  , 0 },
/*D5*/  {"LDAB" , i8YIndexed  , 0 },
/*D6*/  {"ANDB" , i8YIndexed  , 0 },
/*D7*/  {"ORAB" , i8YIndexed  , 0 },
/*D8*/  {"CMPB" , i8YIndexed  , 0 },
/*D9*/  {"BITB" , i8YIndexed  , 0 },
/*DA*/  {"STAB" , i8YIndexed  , 0 },
/*DB*/  {"BRCLR", ib8YRelative, 0 },
/*DC*/  {"LDX"  , i8YIndexed  , 0 },
/*DD*/  {"LDY"  , i8YIndexed  , 0 },
/*DE*/  {"LDZ"  , i8YIndexed  , 0 },
/*DF*/  {"LDS"  , i8YIndexed  , 0 },

/*E0*/  {"SUBB" , i8ZIndexed  , 0 },
/*E1*/  {"ADDB" , i8ZIndexed  , 0 },
/*E2*/  {"SBCB" , i8ZIndexed  , 0 },
/*E3*/  {"ADCB" , i8ZIndexed  , 0 },
/*E4*/  {"EORB" , i8ZIndexed  , 0 },
/*E5*/  {"LDAB" , i8ZIndexed  , 0 },
/*E6*/  {"ANDB" , i8ZIndexed  , 0 },
/*E7*/  {"ORAB" , i8ZIndexed  , 0 },
/*E8*/  {"CMPB" , i8ZIndexed  , 0 },
/*E9*/  {"BITB" , i8ZIndexed  , 0 },
/*EA*/  {"STAB" , i8ZIndexed  , 0 },
/*EB*/  {"BRCLR", ib8ZRelative, 0 },
/*EC*/  {"LDX"  , i8ZIndexed  , 0 },
/*ED*/  {"LDY"  , i8ZIndexed  , 0 },
/*EE*/  {"LDZ"  , i8ZIndexed  , 0 },
/*EF*/  {"LDS"  , i8ZIndexed  , 0 },

/*F0*/  {"SUBB" , i8Immediate , 0 },
/*F1*/  {"ADDB" , i8Immediate , 0 },
/*F2*/  {"SBCB" , i8Immediate , 0 },
/*F3*/  {"ADCB" , i8Immediate , 0 },
/*F4*/  {"EORB" , i8Immediate , 0 },
/*F5*/  {"LDAB" , i8Immediate , 0 },
/*F6*/  {"ANDB" , i8Immediate , 0 },
/*F7*/  {"ORAB" , i8Immediate , 0 },
/*F8*/  {"CMPB" , i8Immediate , 0 },
/*F9*/  {"BITB" , i8Immediate , 0 },
/*FA*/  {"JSR"  , i20Extended , REFFLAG | CODEREF },
/*FB*/  {"RMAC" , iMAC        , 0 },
/*FC*/  {"ADDD" , i8Immediate , 0 },
/*FD*/  {""     , iInvalid    , 0 },
/*FE*/  {""     , iInvalid    , 0 },
/*FF*/  {""     , iInvalid    , 0 }
};

static const struct InstrRec M68HC16_opcdTable17[] =
{
            // op        typ        lfref
/*1700*/    {"COM"  , i16XIndexed , 0 },
/*1701*/    {"DEC"  , i16XIndexed , 0 },
/*1702*/    {"NEG"  , i16XIndexed , 0 },
/*1703*/    {"INC"  , i16XIndexed , 0 },
/*1704*/    {"ASL"  , i16XIndexed , 0 },
/*1705*/    {"CLR"  , i16XIndexed , 0 },
/*1706*/    {"TST"  , i16XIndexed , 0 },
/*1707*/    {""     , i16XIndexed , 0 },
/*1708*/    {"BCLR" , ib8XIndexed , 0 },
/*1709*/    {"BSET" , ib8XIndexed , 0 },
/*170A*/    {""     , i16XIndexed , 0 },
/*170B*/    {""     , i16XIndexed , 0 },
/*170C*/    {"ROL"  , i16XIndexed , 0 },
/*170D*/    {"ASR"  , i16XIndexed , 0 },
/*170E*/    {"ROR"  , i16XIndexed , 0 },
/*170F*/    {"LSR"  , i16XIndexed , 0 },

/*1710*/    {"COM"  , i16YIndexed , 0 },
/*1711*/    {"DEC"  , i16YIndexed , 0 },
/*1712*/    {"NEG"  , i16YIndexed , 0 },
/*1713*/    {"INC"  , i16YIndexed , 0 },
/*1714*/    {"ASL"  , i16YIndexed , 0 },
/*1715*/    {"CLR"  , i16YIndexed , 0 },
/*1716*/    {"TST"  , i16YIndexed , 0 },
/*1717*/    {""     , i16YIndexed , 0 },
/*1718*/    {"BCLR" , ib8YIndexed , 0 },
/*1719*/    {"BSET" , ib8YIndexed , 0 },
/*171A*/    {""     , i16YIndexed , 0 },
/*171B*/    {""     , i16YIndexed , 0 },
/*171C*/    {"ROL"  , i16YIndexed , 0 },
/*171D*/    {"ASR"  , i16YIndexed , 0 },
/*171E*/    {"ROR"  , i16YIndexed , 0 },
/*171F*/    {"LSR"  , i16YIndexed , 0 },

/*1720*/    {"COM"  , i16ZIndexed , 0},
/*1721*/    {"DEC"  , i16ZIndexed , 0},
/*1722*/    {"NEG"  , i16ZIndexed , 0},
/*1723*/    {"INC"  , i16ZIndexed , 0},
/*1724*/    {"ASL"  , i16ZIndexed , 0},
/*1725*/    {"CLR"  , i16ZIndexed , 0},
/*1726*/    {"TST"  , i16ZIndexed , 0},
/*1727*/    {""     , i16ZIndexed , 0},
/*1728*/    {"BCLR" , ib8ZIndexed , 0},
/*1729*/    {"BSET" , ib8ZIndexed , 0},
/*172A*/    {""     , i16ZIndexed , 0},
/*172B*/    {""     , i16ZIndexed , 0},
/*172C*/    {"ROL"  , i16ZIndexed , 0},
/*172D*/    {"ASR"  , i16ZIndexed , 0},
/*172E*/    {"ROR"  , i16ZIndexed , 0},
/*172F*/    {"LSR"  , i16ZIndexed , 0},

/*1730*/    {"COM"  , iExtended   , 0 },
/*1731*/    {"DEC"  , iExtended   , 0 },
/*1732*/    {"NEG"  , iExtended   , 0 },
/*1733*/    {"INC"  , iExtended   , 0 },
/*1734*/    {"ASL"  , iExtended   , 0 },
/*1735*/    {"CLR"  , iExtended   , 0 },
/*1736*/    {"TST"  , iExtended   , 0 },
/*1737*/    {""     , iInvalid    , 0 },
/*1738*/    {""     , iInvalid    , 0 },
/*1739*/    {""     , iInvalid    , 0 },
/*173A*/    {""     , iInvalid    , 0 },
/*173B*/    {""     , iInvalid    , 0 },
/*173C*/    {"ROL"  , iExtended   , 0 },
/*173D*/    {"ASR"  , iExtended   , 0 },
/*173E*/    {"ROR"  , iExtended   , 0 },
/*173F*/    {"LSR"  , iExtended   , 0 },

/*1740*/    {"SUBA" , i16XIndexed , 0 },
/*1741*/    {"ADDA" , i16XIndexed , 0 },
/*1742*/    {"SBCA" , i16XIndexed , 0 },
/*1743*/    {"ADCA" , i16XIndexed , 0 },
/*1744*/    {"EORA" , i16XIndexed , 0 },
/*1745*/    {"LDAA" , i16XIndexed , 0 },
/*1746*/    {"ANDA" , i16XIndexed , 0 },
/*1747*/    {"ORAA" , i16XIndexed , 0 },
/*1748*/    {"CMPA" , i16XIndexed , 0 },
/*1749*/    {"BITA" , i16XIndexed , 0 },
/*174A*/    {"STAA" , i16XIndexed , 0 },
/*174B*/    {""     , iInvalid    , 0 },
/*174C*/    {"CPX"  , i16XIndexed , 0 },
/*174D*/    {"CPY"  , i16XIndexed , 0 },
/*174E*/    {"CPZ"  , i16XIndexed , 0 },
/*174F*/    {"CPS"  , i16XIndexed , 0 },

/*1750*/    {"SUBA" , i16YIndexed , 0 },
/*1751*/    {"ADDA" , i16YIndexed , 0 },
/*1752*/    {"SBCA" , i16YIndexed , 0 },
/*1753*/    {"ADCA" , i16YIndexed , 0 },
/*1754*/    {"EORA" , i16YIndexed , 0 },
/*1755*/    {"LDAA" , i16YIndexed , 0 },
/*1756*/    {"ANDA" , i16YIndexed , 0 },
/*1757*/    {"ORAA" , i16YIndexed , 0 },
/*1758*/    {"CMPA" , i16YIndexed , 0 },
/*1759*/    {"BITA" , i16YIndexed , 0 },
/*175A*/    {"STAA" , i16YIndexed , 0 },
/*175B*/    {""     , iInvalid    , 0 },
/*175C*/    {"CPX"  , i16YIndexed , 0 },
/*175D*/    {"CPY"  , i16YIndexed , 0 },
/*175E*/    {"CPZ"  , i16YIndexed , 0 },
/*175F*/    {"CPS"  , i16YIndexed , 0 },

/*1760*/    {"SUBA" , i16ZIndexed , 0 },
/*1761*/    {"ADDA" , i16ZIndexed , 0 },
/*1762*/    {"SBCA" , i16ZIndexed , 0 },
/*1763*/    {"ADCA" , i16ZIndexed , 0 },
/*1764*/    {"EORA" , i16ZIndexed , 0 },
/*1765*/    {"LDAA" , i16ZIndexed , 0 },
/*1766*/    {"ANDA" , i16ZIndexed , 0 },
/*1767*/    {"ORAA" , i16ZIndexed , 0 },
/*1768*/    {"CMPA" , i16ZIndexed , 0 },
/*1769*/    {"BITA" , i16ZIndexed , 0 },
/*176A*/    {"STAA" , i16ZIndexed , 0 },
/*176B*/    {""     , iInvalid    , 0 },
/*176C*/    {"CPX"  , i16ZIndexed , 0 },
/*176D*/    {"CPY"  , i16ZIndexed , 0 },
/*176E*/    {"CPZ"  , i16ZIndexed , 0 },
/*176F*/    {"CPS"  , i16ZIndexed , 0 },

/*1770*/    {"SUBA" , iExtended   , 0 },
/*1771*/    {"ADDA" , iExtended   , 0 },
/*1772*/    {"SBCA" , iExtended   , 0 },
/*1773*/    {"ADCA" , iExtended   , 0 },
/*1774*/    {"EORA" , iExtended   , 0 },
/*1775*/    {"LDAA" , iExtended   , 0 },
/*1776*/    {"ANDA" , iExtended   , 0 },
/*1777*/    {"ORAA" , iExtended   , 0 },
/*1778*/    {"CMPA" , iExtended   , 0 },
/*1779*/    {"BITA" , iExtended   , 0 },
/*177A*/    {"STAA" , iExtended   , 0 },
/*177B*/    {""     , iInvalid    , 0 },
/*177C*/    {"CPX"  , iExtended   , 0 },
/*177D*/    {"CPY"  , iExtended   , 0 },
/*177E*/    {"CPZ"  , iExtended   , 0 },
/*177F*/    {"CPS"  , iExtended   , 0 },

/*1780*/    {""    , iInvalid     , 0 },
/*1781*/    {""    , iInvalid     , 0 },
/*1782*/    {""    , iInvalid     , 0 },
/*1783*/    {""    , iInvalid     , 0 },
/*1784*/    {""    , iInvalid     , 0 },
/*1785*/    {""    , iInvalid     , 0 },
/*1786*/    {""    , iInvalid     , 0 },
/*1787*/    {""    , iInvalid     , 0 },
/*1788*/    {""    , iInvalid     , 0 },
/*1789*/    {""    , iInvalid     , 0 },
/*178A*/    {""    , iInvalid     , 0 },
/*178B*/    {""    , iInvalid     , 0 },
/*178C*/    {"STX" , i16XIndexed  , 0 },
/*178D*/    {"STY" , i16XIndexed  , 0 },
/*178E*/    {"STZ" , i16XIndexed  , 0 },
/*178F*/    {"STS" , i16XIndexed  , 0 },

/*1790*/    {""    , iInvalid     , 0 },
/*1791*/    {""    , iInvalid     , 0 },
/*1792*/    {""    , iInvalid     , 0 },
/*1793*/    {""    , iInvalid     , 0 },
/*1794*/    {""    , iInvalid     , 0 },
/*1795*/    {""    , iInvalid     , 0 },
/*1796*/    {""    , iInvalid     , 0 },
/*1797*/    {""    , iInvalid     , 0 },
/*1798*/    {""    , iInvalid     , 0 },
/*1799*/    {""    , iInvalid     , 0 },
/*179A*/    {""    , iInvalid     , 0 },
/*179B*/    {""    , iInvalid     , 0 },
/*179C*/    {"STX" , i16YIndexed  , 0 },
/*179D*/    {"STY" , i16YIndexed  , 0 },
/*179E*/    {"STZ" , i16YIndexed  , 0 },
/*179F*/    {"STS" , i16YIndexed  , 0 },

/*17A0*/    {""    , iInvalid     , 0 },
/*17A1*/    {""    , iInvalid     , 0 },
/*17A2*/    {""    , iInvalid     , 0 },
/*17A3*/    {""    , iInvalid     , 0 },
/*17A4*/    {""    , iInvalid     , 0 },
/*17A5*/    {""    , iInvalid     , 0 },
/*17A6*/    {""    , iInvalid     , 0 },
/*17A7*/    {""    , iInvalid     , 0 },
/*17A8*/    {""    , iInvalid     , 0 },
/*17A9*/    {""    , iInvalid     , 0 },
/*17AA*/    {""    , iInvalid     , 0 },
/*17AB*/    {""    , iInvalid     , 0 },
/*17AC*/    {"STX" , i16ZIndexed  , 0 },
/*17AD*/    {"STY" , i16ZIndexed  , 0 },
/*17AE*/    {"STZ" , i16ZIndexed  , 0 },
/*17AF*/    {"STS" , i16ZIndexed  , 0 },

/*17B0*/    {""    , iInvalid     , 0 },
/*17B1*/    {""    , iInvalid     , 0 },
/*17B2*/    {""    , iInvalid     , 0 },
/*17B3*/    {""    , iInvalid     , 0 },
/*17B4*/    {""    , iInvalid     , 0 },
/*17B5*/    {""    , iInvalid     , 0 },
/*17B6*/    {""    , iInvalid     , 0 },
/*17B7*/    {""    , iInvalid     , 0 },
/*17B8*/    {""    , iInvalid     , 0 },
/*17B9*/    {""    , iInvalid     , 0 },
/*17BA*/    {""    , iInvalid     , 0 },
/*17BB*/    {""    , iInvalid     , 0 },
/*17BC*/    {"STX" , iExtended    , 0 },
/*17BD*/    {"STY" , iExtended    , 0 },
/*17BE*/    {"STZ" , iExtended    , 0 },
/*17BF*/    {"STS" , iExtended    , 0 },

/*17C0*/    {"SUBB" , i16XIndexed , 0 },
/*17C1*/    {"ADDB" , i16XIndexed , 0 },
/*17C2*/    {"SBCB" , i16XIndexed , 0 },
/*17C3*/    {"ADCB" , i16XIndexed , 0 },
/*17C4*/    {"EORB" , i16XIndexed , 0 },
/*17C5*/    {"LDAB" , i16XIndexed , 0 },
/*17C6*/    {"ANDB" , i16XIndexed , 0 },
/*17C7*/    {"ORAB" , i16XIndexed , 0 },
/*17C8*/    {"CMPB" , i16XIndexed , 0 },
/*17C9*/    {"BITB" , i16XIndexed , 0 },
/*17CA*/    {"STAB" , i16XIndexed , 0 },
/*17CB*/    {""     , iInvalid    , 0 },
/*17CC*/    {"LDX"  , i16XIndexed , 0 },
/*17CD*/    {"LDY"  , i16XIndexed , 0 },
/*17CE*/    {"LDZ"  , i16XIndexed , 0 },
/*17CF*/    {"LDS"  , i16XIndexed , 0 },

/*17D0*/    {"SUBB" , i16YIndexed , 0 },
/*17D1*/    {"ADDB" , i16YIndexed , 0 },
/*17D2*/    {"SBCB" , i16YIndexed , 0 },
/*17D3*/    {"ADCB" , i16YIndexed , 0 },
/*17D4*/    {"EORB" , i16YIndexed , 0 },
/*17D5*/    {"LDAB" , i16YIndexed , 0 },
/*17D6*/    {"ANDB" , i16YIndexed , 0 },
/*17D7*/    {"ORAB" , i16YIndexed , 0 },
/*17D8*/    {"CMPB" , i16YIndexed , 0 },
/*17D9*/    {"BITB" , i16YIndexed , 0 },
/*17DA*/    {"STAB" , i16YIndexed , 0 },
/*17DB*/    {""     , iInvalid    , 0 },
/*17DC*/    {"LDX"  , i16YIndexed , 0 },
/*17DD*/    {"LDY"  , i16YIndexed , 0 },
/*17DE*/    {"LDZ"  , i16YIndexed , 0 },
/*17DF*/    {"LDS"  , i16YIndexed , 0 },

/*17E0*/    {"SUBB" , i16ZIndexed , 0 },
/*17E1*/    {"ADDB" , i16ZIndexed , 0 },
/*17E2*/    {"SBCB" , i16ZIndexed , 0 },
/*17E3*/    {"ADCB" , i16ZIndexed , 0 },
/*17E4*/    {"EORB" , i16ZIndexed , 0 },
/*17E5*/    {"LDAB" , i16ZIndexed , 0 },
/*17E6*/    {"ANDB" , i16ZIndexed , 0 },
/*17E7*/    {"ORAB" , i16ZIndexed , 0 },
/*17E8*/    {"CMPB" , i16ZIndexed , 0 },
/*17E9*/    {"BITB" , i16ZIndexed , 0 },
/*17EA*/    {"STAB" , i16ZIndexed , 0 },
/*17EB*/    {""     , iInvalid    , 0 },
/*17EC*/    {"LDX"  , i16ZIndexed , 0 },
/*17ED*/    {"LDY"  , i16ZIndexed , 0 },
/*17EE*/    {"LDZ"  , i16ZIndexed , 0 },
/*17EF*/    {"LDS"  , i16ZIndexed , 0 },

/*17F0*/    {"SUBB" , iExtended   , 0 },
/*17F1*/    {"ADDB" , iExtended   , 0 },
/*17F2*/    {"SBCB" , iExtended   , 0 },
/*17F3*/    {"ADCB" , iExtended   , 0 },
/*17F4*/    {"EORB" , iExtended   , 0 },
/*17F5*/    {"LDAB" , iExtended   , 0 },
/*17F6*/    {"ANDB" , iExtended   , 0 },
/*17F7*/    {"ORAB" , iExtended   , 0 },
/*17F8*/    {"CMPB" , iExtended   , 0 },
/*17F9*/    {"BITB" , iExtended   , 0 },
/*17FA*/    {"STAB" , iExtended   , 0 },
/*17FB*/    {""     , iInvalid    , 0 },
/*17FC*/    {"LDX"  , iExtended   , 0 },
/*17FD*/    {"LDY"  , iExtended   , 0 },
/*17FE*/    {"LDZ"  , iExtended   , 0 },
/*17FF*/    {"LDS"  , iExtended   , 0 },
};

static const struct InstrRec M68HC16_opcdTable27[] =
{
            // op        typ        lfref
/*2700*/    {"COMW" , i16XIndexed , 0 },
/*2701*/    {"DECW" , i16XIndexed , 0 },
/*2702*/    {"NEGW" , i16XIndexed , 0 },
/*2703*/    {"INCW" , i16XIndexed , 0 },
/*2704*/    {"ASLW" , i16XIndexed , 0 },
/*2705*/    {"CLRW" , i16XIndexed , 0 },
/*2706*/    {"TSTW" , i16XIndexed , 0 },
/*2707*/    {""     , iInvalid    , 0 },
/*2708*/    {"BCLRW", ib16XIndexed, 0 },
/*2709*/    {"BSETW", ib16XIndexed, 0 },
/*270A*/    {""     , iInvalid    , 0 },
/*270B*/    {""     , iInvalid    , 0 },
/*270C*/    {"ROLW" , i16XIndexed , 0 },
/*270D*/    {"ASRW" , i16XIndexed , 0 },
/*270E*/    {"RORW" , i16XIndexed , 0 },
/*270F*/    {"LSRW" , i16XIndexed , 0 },

/*2710*/    {"COMW" , i16YIndexed , 0 },
/*2711*/    {"DECW" , i16YIndexed , 0 },
/*2712*/    {"NEGW" , i16YIndexed , 0 },
/*2713*/    {"INCW" , i16YIndexed , 0 },
/*2714*/    {"ASLW" , i16YIndexed , 0 },
/*2715*/    {"CLRW" , i16YIndexed , 0 },
/*2716*/    {"TSTW" , i16YIndexed , 0 },
/*2717*/    {""     , iInvalid    , 0 },
/*2718*/    {"BCLRW", ib16YIndexed, 0 },
/*2719*/    {"BSETW", ib16YIndexed, 0 },
/*271A*/    {""     , iInvalid    , 0 },
/*271B*/    {""     , iInvalid    , 0 },
/*271C*/    {"ROLW" , i16YIndexed , 0 },
/*271D*/    {"ASRW" , i16YIndexed , 0 },
/*271E*/    {"RORW" , i16YIndexed , 0 },
/*271F*/    {"LSRW" , i16YIndexed , 0 },

/*2720*/    {"COMW" , i16ZIndexed , 0 },
/*2721*/    {"DECW" , i16ZIndexed , 0 },
/*2722*/    {"NEGW" , i16ZIndexed , 0 },
/*2723*/    {"INCW" , i16ZIndexed , 0 },
/*2724*/    {"ASLW" , i16ZIndexed , 0 },
/*2725*/    {"CLRW" , i16ZIndexed , 0 },
/*2726*/    {"TSTW" , i16ZIndexed , 0 },
/*2727*/    {""     , iInvalid    , 0 },
/*2728*/    {"BCLRW", ib16ZIndexed, 0 },
/*2729*/    {"BSETW", ib16ZIndexed, 0 },
/*272A*/    {""     , iInvalid    , 0 },
/*272B*/    {""     , iInvalid    , 0 },
/*272C*/    {"ROLW" , i16ZIndexed , 0 },
/*272D*/    {"ASRW" , i16ZIndexed , 0 },
/*272E*/    {"RORW" , i16ZIndexed , 0 },
/*272F*/    {"LSRW" , i16ZIndexed , 0 },

/*2730*/    {"COMW" , iExtended   , 0 },
/*2731*/    {"DECW" , iExtended   , 0 },
/*2732*/    {"NEGW" , iExtended   , 0 },
/*2733*/    {"INCW" , iExtended   , 0 },
/*2734*/    {"ASLW" , iExtended   , 0 },
/*2735*/    {"CLRW" , iExtended   , 0 },
/*2736*/    {"TSTW" , iExtended   , 0 },
/*2737*/    {""     , iInvalid    , 0 },
/*2738*/    {"BCLRW", ibWExtended , 0 },
/*2739*/    {"BSETW", ibWExtended , 0 },
/*273A*/    {""     , iInvalid    , 0 },
/*273B*/    {""     , iInvalid    , 0 },
/*273C*/    {"ROLW" , iExtended   , 0 },
/*273D*/    {"ASRW" , iExtended   , 0 },
/*273E*/    {"RORW" , iExtended   , 0 },
/*273F*/    {"LSRW" , iExtended   , 0 },

/*2740*/    {"SUBA" , iEXIndexed  , 0 },
/*2741*/    {"ADDA" , iEXIndexed  , 0 },
/*2742*/    {"SBCA" , iEXIndexed  , 0 },
/*2743*/    {"ADCA" , iEXIndexed  , 0 },
/*2744*/    {"EORA" , iEXIndexed  , 0 },
/*2745*/    {"LDAA" , iEXIndexed  , 0 },
/*2746*/    {"ANDA" , iEXIndexed  , 0 },
/*2747*/    {"ORAA" , iEXIndexed  , 0 },
/*2748*/    {"CMPA" , iEXIndexed  , 0 },
/*2749*/    {"BITA" , iEXIndexed  , 0 },
/*274A*/    {"STAA" , iEXIndexed  , 0 },
/*274B*/    {""     , iInvalid    , 0 },
/*274C*/    {"NOP"  , iInherent   , 0 },
/*274D*/    {"TYX"  , iInherent   , 0 },
/*274E*/    {"TZX"  , iInherent   , 0 },
/*274F*/    {"TSX"  , iInherent   , 0 },

/*2750*/    {"SUBA" , iEYIndexed  , 0 },
/*2751*/    {"ADDA" , iEYIndexed  , 0 },
/*2752*/    {"SBCA" , iEYIndexed  , 0 },
/*2753*/    {"ADCA" , iEYIndexed  , 0 },
/*2754*/    {"EORA" , iEYIndexed  , 0 },
/*2755*/    {"LDAA" , iEYIndexed  , 0 },
/*2756*/    {"ANDA" , iEYIndexed  , 0 },
/*2757*/    {"ORAA" , iEYIndexed  , 0 },
/*2758*/    {"CMPA" , iEYIndexed  , 0 },
/*2759*/    {"BITA" , iEYIndexed  , 0 },
/*275A*/    {"STAA" , iEYIndexed  , 0 },
/*275B*/    {""     , iInvalid    , 0 },
/*275C*/    {"TXY"  , iInherent   , 0 },
/*275D*/    {""     , iInvalid    , 0 },
/*275E*/    {"TZY"  , iInherent   , 0 },
/*275F*/    {"TSY"  , iInherent   , 0 },

/*2760*/    {"SUBA" , iEZIndexed  , 0 },
/*2761*/    {"ADDA" , iEZIndexed  , 0 },
/*2762*/    {"SBCA" , iEZIndexed  , 0 },
/*2763*/    {"ADCA" , iEZIndexed  , 0 },
/*2764*/    {"EORA" , iEZIndexed  , 0 },
/*2765*/    {"LDAA" , iEZIndexed  , 0 },
/*2766*/    {"ANDA" , iEZIndexed  , 0 },
/*2767*/    {"ORAA" , iEZIndexed  , 0 },
/*2768*/    {"CMPA" , iEZIndexed  , 0 },
/*2769*/    {"BITA" , iEZIndexed  , 0 },
/*276A*/    {"STAA" , iEZIndexed  , 0 },
/*276B*/    {""     , iInvalid    , 0 },
/*276C*/    {"TXZ"  , iInherent   , 0 },
/*276D*/    {"TYZ"  , iInherent   , 0 },
/*276E*/    {""     , iInvalid    , 0 },
/*276F*/    {"TSZ"  , iInherent   , 0 },

/*2770*/    {"COME" , iInherent   , 0 },
/*2771*/    {"LDED" , iExtended   , 0 },
/*2772*/    {"NEGE" , iInherent   , 0 },
/*2773*/    {"STED" , iExtended   , 0 },
/*2774*/    {"ASLE" , iInherent   , 0 },
/*2775*/    {"CLRE" , iInherent   , 0 },
/*2776*/    {"TSTE" , iInherent   , 0 },
/*2777*/    {"RTI"  , iInherent   , LFFLAG },
/*2778*/    {"ADE"  , iInherent   , 0 },
/*2779*/    {"SDE"  , iInherent   , 0 },
/*277A*/    {"XGDE" , iInherent   , 0 },
/*277B*/    {"TDE"  , iInherent   , 0 },
/*277C*/    {"ROLE" , iInherent   , 0 },
/*277D*/    {"ASRE" , iInherent   , 0 },
/*277E*/    {"RORE" , iInherent   , 0 },
/*277F*/    {"LSRE" , iInherent   , 0 },

/*2780*/    {"SUBD" , iEXIndexed  , 0 },
/*2781*/    {"ADDD" , iEXIndexed  , 0 },
/*2782*/    {"SBCD" , iEXIndexed  , 0 },
/*2783*/    {"ADCD" , iEXIndexed  , 0 },
/*2784*/    {"EORD" , iEXIndexed  , 0 },
/*2785*/    {"LDD"  , iEXIndexed  , 0 },
/*2786*/    {"ANDD" , iEXIndexed  , 0 },
/*2787*/    {"ORD"  , iEXIndexed  , 0 },
/*2788*/    {"CPD"  , iEXIndexed  , 0 },
/*2789*/    {""     , iInvalid    , 0 },
/*278A*/    {"STD"  , iEXIndexed  , 0 },
/*278B*/    {""     , iInvalid    , 0 },
/*278C*/    {""     , iInvalid    , 0 },
/*278D*/    {""     , iInvalid    , 0 },
/*278E*/    {""     , iInvalid    , 0 },
/*278F*/    {""     , iInvalid    , 0 },

/*2790*/    {"SUBD" , iEYIndexed  , 0 },
/*2791*/    {"ADDD" , iEYIndexed  , 0 },
/*2792*/    {"SBCD" , iEYIndexed  , 0 },
/*2793*/    {"ADCD" , iEYIndexed  , 0 },
/*2794*/    {"EORD" , iEYIndexed  , 0 },
/*2795*/    {"LDD"  , iEYIndexed  , 0 },
/*2796*/    {"ANDD" , iEYIndexed  , 0 },
/*2797*/    {"ORD"  , iEYIndexed  , 0 },
/*2798*/    {"CPD"  , iEYIndexed  , 0 },
/*2799*/    {""     , iInvalid    , 0 },
/*279A*/    {"STD"  , iEYIndexed  , 0 },
/*279B*/    {""     , iInvalid    , 0 },
/*279C*/    {""     , iInvalid    , 0 },
/*279D*/    {""     , iInvalid    , 0 },
/*279E*/    {""     , iInvalid    , 0 },
/*279F*/    {""     , iInvalid    , 0 },

/*27A0*/    {"SUBD" , iEZIndexed  , 0 },
/*27A1*/    {"ADDD" , iEZIndexed  , 0 },
/*27A2*/    {"SBCD" , iEZIndexed  , 0 },
/*27A3*/    {"ADCD" , iEZIndexed  , 0 },
/*27A4*/    {"EORD" , iEZIndexed  , 0 },
/*27A5*/    {"LDD"  , iEZIndexed  , 0 },
/*27A6*/    {"ANDD" , iEZIndexed  , 0 },
/*27A7*/    {"ORD"  , iEZIndexed  , 0 },
/*27A8*/    {"CPD"  , iEZIndexed  , 0 },
/*27A9*/    {""     , iInvalid    , 0 },
/*27AA*/    {"STD"  , iEZIndexed  , 0 },
/*27AB*/    {""     , iInvalid    , 0 },
/*27AC*/    {""     , iInvalid    , 0 },
/*27AD*/    {""     , iInvalid    , 0 },
/*27AE*/    {""     , iInvalid    , 0 },
/*27AF*/    {""     , iInvalid    , 0 },

/*27B0*/    {"LDHI" , iInherent   , 0 },
/*27B1*/    {"TEDM" , iInherent   , 0 },
/*27B2*/    {"TEM"  , iInherent   , 0 },
/*27B3*/    {"TMXED", iInherent   , 0 },
/*27B4*/    {"TMER" , iInherent   , 0 },
/*27B5*/    {"TMET" , iInherent   , 0 },
/*27B6*/    {"ASLM" , iInherent   , 0 },
/*27B7*/    {"CLRM" , iInherent   , 0 },
/*27B8*/   {"PSHMAC", iInherent   , 0 },
/*27B9*/   {"PULMAC", iInherent   , 0 },
/*27BA*/    {"ASRM" , iInherent   , 0 },
/*27BB*/    {"TEKB" , iInherent   , 0 },
/*27BC*/    {""     , iInvalid    , 0 },
/*27BD*/    {""     , iInvalid    , 0 },
/*27BE*/    {""     , iInvalid    , 0 },
/*27BF*/    {""     , iInvalid    , 0 },

/*27C0*/    {"SUBB" , iEXIndexed  , 0 },
/*27C1*/    {"ADDB" , iEXIndexed  , 0 },
/*27C2*/    {"SBCB" , iEXIndexed  , 0 },
/*27C3*/    {"ADCB" , iEXIndexed  , 0 },
/*27C4*/    {"EORB" , iEXIndexed  , 0 },
/*27C5*/    {"LDAB" , iEXIndexed  , 0 },
/*27C6*/    {"ANDB" , iEXIndexed  , 0 },
/*27C7*/    {"ORAB" , iEXIndexed  , 0 },
/*27C8*/    {"CMPB" , iEXIndexed  , 0 },
/*27C9*/    {"BITB" , iEXIndexed  , 0 },
/*27CA*/    {"STAB" , iEXIndexed  , 0 },
/*27CB*/    {""     , iInvalid    , 0 },
/*27CC*/    {""     , iInvalid    , 0 },
/*27CD*/    {""     , iInvalid    , 0 },
/*27CE*/    {""     , iInvalid    , 0 },
/*27CF*/    {""     , iInvalid    , 0 },

/*27D0*/    {"SUBB" , iEYIndexed  , 0 },
/*27D1*/    {"ADDB" , iEYIndexed  , 0 },
/*27D2*/    {"SBCB" , iEYIndexed  , 0 },
/*27D3*/    {"ADCB" , iEYIndexed  , 0 },
/*27D4*/    {"EORB" , iEYIndexed  , 0 },
/*27D5*/    {"LDAB" , iEYIndexed  , 0 },
/*27D6*/    {"ANDB" , iEYIndexed  , 0 },
/*27D7*/    {"ORAB" , iEYIndexed  , 0 },
/*27D8*/    {"CMPB" , iEYIndexed  , 0 },
/*27D9*/    {"BITB" , iEYIndexed  , 0 },
/*27DA*/    {"STAB" , iEYIndexed  , 0 },
/*27DB*/    {""     , iInvalid    , 0 },
/*27DC*/    {""     , iInvalid    , 0 },
/*27DD*/    {""     , iInvalid    , 0 },
/*27DE*/    {""     , iInvalid    , 0 },
/*27DF*/    {""     , iInvalid    , 0 },

/*27E0*/    {"SUBB" , iEZIndexed  , 0 },
/*27E1*/    {"ADDB" , iEZIndexed  , 0 },
/*27E2*/    {"SBCB" , iEZIndexed  , 0 },
/*27E3*/    {"ADCB" , iEZIndexed  , 0 },
/*27E4*/    {"EORB" , iEZIndexed  , 0 },
/*27E5*/    {"LDAB" , iEZIndexed  , 0 },
/*27E6*/    {"ANDB" , iEZIndexed  , 0 },
/*27E7*/    {"ORAB" , iEZIndexed  , 0 },
/*27E8*/    {"CMPB" , iEZIndexed  , 0 },
/*27E9*/    {"BITB" , iEZIndexed  , 0 },
/*27EA*/    {"STAB" , iEZIndexed  , 0 },
/*27EB*/    {""     , iInvalid    , 0 },
/*27EC*/    {""     , iInvalid    , 0 },
/*27ED*/    {""     , iInvalid    , 0 },
/*27EE*/    {""     , iInvalid    , 0 },
/*27EF*/    {""     , iInvalid    , 0 },

/*27F0*/    {"COMD" , iInherent   , 0 },
/*27F1*/   {"LPSTOP", iInherent   , 0 },
/*27F2*/    {"NEGD" , iInherent   , 0 },
/*27F3*/    {"WAI"  , iInherent   , 0 },
/*27F4*/    {"ASLD" , iInherent   , 0 },
/*27F5*/    {"CLRD" , iInherent   , 0 },
/*27F6*/    {"TSTD" , iInherent   , 0 },
/*27F7*/    {"RTS"  , iInherent   , LFFLAG },
/*27F8*/    {"SXT"  , iInherent   , 0 },
/*27F9*/    {"LBSR" , i16Relative , CODEREF },
/*27FA*/    {"TBEK" , iInherent   , 0 },
/*27FB*/    {"TED"  , iInherent   , 0 },
/*27FC*/    {"ROLD" , iInherent   , 0 },
/*27FD*/    {"ASRD" , iInherent   , 0 },
/*27FE*/    {"RORD" , iInherent   , 0 },
/*27FF*/    {"LSRD" , iInherent   , 0 },
};

static const struct InstrRec M68HC16_opcdTable37[] =
{
            // op        typ        lfref
/*3700*/    {"COMA" , iInherent   , 0 },
/*3701*/    {"DECA" , iInherent   , 0 },
/*3702*/    {"NEGA" , iInherent   , 0 },
/*3703*/    {"INCA" , iInherent   , 0 },
/*3704*/    {"ASLA" , iInherent   , 0 },
/*3705*/    {"CLRA" , iInherent   , 0 },
/*3706*/    {"TSTA" , iInherent   , 0 },
/*3707*/    {"TBA"  , iInherent   , 0 },
/*3708*/    {"PSHA" , iInherent   , 0 },
/*3709*/    {"PULA" , iInherent   , 0 },
/*370A*/    {"SBA"  , iInherent   , 0 },
/*370B*/    {"ABA"  , iInherent   , 0 },
/*370C*/    {"ROLA" , iInherent   , 0 },
/*370D*/    {"ASRA" , iInherent   , 0 },
/*370E*/    {"RORA" , iInherent   , 0 },
/*370F*/    {"LSRA" , iInherent   , 0 },

/*3710*/    {"COMB" , iInherent   , 0 },
/*3711*/    {"DECB" , iInherent   , 0 },
/*3712*/    {"NEGB" , iInherent   , 0 },
/*3713*/    {"INCB" , iInherent   , 0 },
/*3714*/    {"ASLB" , iInherent   , 0 },
/*3715*/    {"CLRB" , iInherent   , 0 },
/*3716*/    {"TSTB" , iInherent   , 0 },
/*3717*/    {"TAB"  , iInherent   , 0 },
/*3718*/    {"PSHB" , iInherent   , 0 },
/*3719*/    {"PULB" , iInherent   , 0 },
/*371A*/    {"XGAB" , iInherent   , 0 },
/*371B*/    {"CBA"  , iInherent   , 0 },
/*371C*/    {"ROLB" , iInherent   , 0 },
/*371D*/    {"ASRB" , iInherent   , 0 },
/*371E*/    {"RORB" , iInherent   , 0 },
/*371F*/    {"LSRB" , iInherent   , 0 },

/*3720*/    {"SWI"  , iInherent   , 0 },
/*3721*/    {"DAA"  , iInherent   , 0 },
/*3722*/    {"ACE"  , iInherent   , 0 },
/*3723*/    {"ACED" , iInherent   , 0 },
/*3724*/    {"MUL"  , iInherent   , 0 },
/*3725*/    {"EMUL" , iInherent   , 0 },
/*3726*/    {"EMULS", iInherent   , 0 },
/*3727*/    {"FMULS", iInherent   , 0 },
/*3728*/    {"EDIV" , iInherent   , 0 },
/*3729*/    {"EDIVS", iInherent   , 0 },
/*372A*/    {"IDIV" , iInherent   , 0 },
/*372B*/    {"FDIV" , iInherent   , 0 },
/*372C*/    {"TPD"  , iInherent   , 0 },
/*372D*/    {"TDP"  , iInherent   , 0 },
/*372E*/    {""     , iInvalid    , 0 },
/*372F*/    {"TDMSK", iInherent   , 0 },

/*3730*/    {"SUBE" , i16Immediate, 0 },
/*3731*/    {"ADDE" , i16Immediate, 0 },
/*3732*/    {"SBCE" , i16Immediate, 0 },
/*3733*/    {"ADCE" , i16Immediate, 0 },
/*3734*/    {"EORE" , i16Immediate, 0 },
/*3735*/    {"LDE"  , i16Immediate, 0 },
/*3736*/    {"ANDE" , i16Immediate, 0 },
/*3737*/    {"ORE"  , i16Immediate, 0 },
/*3738*/    {"CPE"  , i16Immediate, 0 },
/*3739*/    {""     , iInvalid    , 0 },
/*373A*/    {"ANDP" , i16Immediate, 0 },
/*373B*/    {"ORP"  , i16Immediate, 0 },
/*373C*/    {"AIX"  , i16Immediate, 0 },
/*373D*/    {"AIY"  , i16Immediate, 0 },
/*373E*/    {"AIZ"  , i16Immediate, 0 },
/*373F*/    {"AIS"  , i16Immediate, 0 },

/*3740*/    {"SUBE" , i16XIndexed , 0 },
/*3741*/    {"ADDE" , i16XIndexed , 0 },
/*3742*/    {"SBCE" , i16XIndexed , 0 },
/*3743*/    {"ADCE" , i16XIndexed , 0 },
/*3744*/    {"EORE" , i16XIndexed , 0 },
/*3745*/    {"LDE"  , i16XIndexed , 0 },
/*3746*/    {"ANDE" , i16XIndexed , 0 },
/*3747*/    {"ORE"  , i16XIndexed , 0 },
/*3748*/    {"CPE"  , i16XIndexed , 0 },
/*3749*/    {""     , iInvalid    , 0 },
/*374A*/    {"STE"  , i16XIndexed , 0 },
/*374B*/    {""     , iInvalid    , 0 },
/*374C*/    {"XGEX" , iInherent   , 0 },
/*374D*/    {"AEX"  , iInherent   , 0 },
/*374E*/    {"TXS"  , iInherent   , 0 },
/*374F*/    {"ABX"  , iInherent   , 0 },

/*3750*/    {"SUBE" , i16YIndexed , 0 },
/*3751*/    {"ADDE" , i16YIndexed , 0 },
/*3752*/    {"SBCE" , i16YIndexed , 0 },
/*3753*/    {"ADCE" , i16YIndexed , 0 },
/*3754*/    {"EORE" , i16YIndexed , 0 },
/*3755*/    {"LDE"  , i16YIndexed , 0 },
/*3756*/    {"ANDE" , i16YIndexed , 0 },
/*3757*/    {"ORE"  , i16YIndexed , 0 },
/*3758*/    {"CPE"  , i16YIndexed , 0 },
/*3759*/    {""     , iInvalid    , 0 },
/*375A*/    {"STE"  , i16YIndexed , 0 },
/*375B*/    {""     , iInvalid    , 0 },
/*375C*/    {"XGEY" , iInherent   , 0 },
/*375D*/    {"AEY"  , iInherent   , 0 },
/*375E*/    {"TYS"  , iInherent   , 0 },
/*375F*/    {"ABY"  , iInherent   , 0 },

/*3760*/    {"SUBE" , i16ZIndexed , 0 },
/*3761*/    {"ADDE" , i16ZIndexed , 0 },
/*3762*/    {"SBCE" , i16ZIndexed , 0 },
/*3763*/    {"ADCE" , i16ZIndexed , 0 },
/*3764*/    {"EORE" , i16ZIndexed , 0 },
/*3765*/    {"LDE"  , i16ZIndexed , 0 },
/*3766*/    {"ANDE" , i16ZIndexed , 0 },
/*3767*/    {"ORE"  , i16ZIndexed , 0 },
/*3768*/    {"CPE"  , i16ZIndexed , 0 },
/*3769*/    {""     , iInvalid    , 0 },
/*376A*/    {"STE"  , i16ZIndexed , 0 },
/*376B*/    {""     , iInvalid    , 0 },
/*376C*/    {"XGEZ" , iInherent   , 0 },
/*376D*/    {"AEZ"  , iInherent   , 0 },
/*376E*/    {"TZS"  , iInherent   , 0 },
/*376F*/    {"ABZ"  , iInherent   , 0 },

/*3770*/    {"SUBE" , iExtended   , 0 },
/*3771*/    {"ADDE" , iExtended   , 0 },
/*3772*/    {"SBCE" , iExtended   , 0 },
/*3773*/    {"ADCE" , iExtended   , 0 },
/*3774*/    {"EORE" , iExtended   , 0 },
/*3775*/    {"LDE"  , iExtended   , 0 },
/*3776*/    {"ANDE" , iExtended   , 0 },
/*3777*/    {"ORE"  , iExtended   , 0 },
/*3778*/    {"CPE"  , iExtended   , 0 },
/*3779*/    {""     , iInvalid    , 0 },
/*377A*/    {"STE"  , iExtended   , 0 },
/*377B*/    {""     , iInvalid    , 0 },
/*377C*/    {"CPX"  , i16Immediate, 0 },
/*377D*/    {"CPY"  , i16Immediate, 0 },
/*377E*/    {"CPZ"  , i16Immediate, 0 },
/*377F*/    {"CPS"  , i16Immediate, 0 },

/*3780*/    {"LBRA" , i16Relative , REFFLAG | CODEREF | LFFLAG },
/*3781*/    {"LBRN" , i16Relative , REFFLAG | CODEREF },
/*3782*/    {"LBHI" , i16Relative , REFFLAG | CODEREF },
/*3783*/    {"LBLS" , i16Relative , REFFLAG | CODEREF },
/*3784*/    {"LBCC" , i16Relative , REFFLAG | CODEREF },    // also LBHS
/*3785*/    {"LBCS" , i16Relative , REFFLAG | CODEREF },    // also LBLO
/*3786*/    {"LBNE" , i16Relative , REFFLAG | CODEREF },
/*3787*/    {"LBEQ" , i16Relative , REFFLAG | CODEREF },
/*3788*/    {"LBVC" , i16Relative , REFFLAG | CODEREF },
/*3789*/    {"LBVS" , i16Relative , REFFLAG | CODEREF },
/*378A*/    {"LBPL" , i16Relative , REFFLAG | CODEREF },
/*378B*/    {"LBMI" , i16Relative , REFFLAG | CODEREF },
/*378C*/    {"LBGE" , i16Relative , REFFLAG | CODEREF },
/*378D*/    {"LBLT" , i16Relative , REFFLAG | CODEREF },
/*378E*/    {"LBGT" , i16Relative , REFFLAG | CODEREF },
/*378F*/    {"LBLE" , i16Relative , REFFLAG | CODEREF },

/*3790*/    {"LBMV" , i16Relative , REFFLAG | CODEREF },
/*3791*/    {"LBEV" , i16Relative , REFFLAG | CODEREF },
/*3792*/    {""     , iInvalid    , 0 },
/*3793*/    {""     , iInvalid    , 0 },
/*3794*/    {""     , iInvalid    , 0 },
/*3795*/    {""     , iInvalid    , 0 },
/*3796*/    {""     , iInvalid    , 0 },
/*3797*/    {""     , iInvalid    , 0 },
/*3798*/    {""     , iInvalid    , 0 },
/*3799*/    {""     , iInvalid    , 0 },
/*379A*/    {""     , iInvalid    , 0 },
/*379B*/    {""     , iInvalid    , 0 },
/*379C*/    {"TBXK" , iInherent   , 0 },
/*379D*/    {"TBYK" , iInherent   , 0 },
/*379E*/    {"TBZK" , iInherent   , 0 },
/*379F*/    {"TBSK" , iInherent   , 0 },

/*37A0*/    {""     , iInvalid    , 0 },
/*37A1*/    {""     , iInvalid    , 0 },
/*37A2*/    {""     , iInvalid    , 0 },
/*37A3*/    {""     , iInvalid    , 0 },
/*37A4*/    {""     , iInvalid    , 0 },
/*37A5*/    {""     , iInvalid    , 0 },
/*37A6*/    {"BGND" , iInherent   , 0 },
/*37A7*/    {""     , iInvalid    , 0 },
/*37A8*/    {""     , iInvalid    , 0 },
/*37A9*/    {""     , iInvalid    , 0 },
/*37AA*/    {""     , iInvalid    , 0 },
/*37AB*/    {""     , iInvalid    , 0 },
/*37AC*/    {"TXKB" , iInherent   , 0 },
/*37AD*/    {"TYKB" , iInherent   , 0 },
/*37AE*/    {"TZKB" , iInherent   , 0 },
/*37AF*/    {"TSKB" , iInherent   , 0 },

/*37B0*/    {"SUBD" , i16Immediate, 0 },
/*37B1*/    {"ADDD" , i16Immediate, 0 },
/*37B2*/    {"SBCD" , i16Immediate, 0 },
/*37B3*/    {"ADCD" , i16Immediate, 0 },
/*37B4*/    {"EORD" , i16Immediate, 0 },
/*37B5*/    {"LDD"  , i16Immediate, 0 },
/*37B6*/    {"ANDD" , i16Immediate, 0 },
/*37B7*/    {"ORD"  , i16Immediate, 0 },
/*37B8*/    {"CPD"  , i16Immediate, 0 },
/*37B9*/    {""     , iInvalid    , 0 },
/*37BA*/    {""     , iInvalid    , 0 },
/*37BB*/    {""     , iInvalid    , 0 },
/*37BC*/    {"LDX"  , i16Immediate, 0 },
/*37BD*/    {"LDY"  , i16Immediate, 0 },
/*37BE*/    {"LDZ"  , i16Immediate, 0 },
/*37BF*/    {"LDS"  , i16Immediate, 0 },

/*37C0*/    {"SUBD" , i16XIndexed , 0 },
/*37C1*/    {"ADDD" , i16XIndexed , 0 },
/*37C2*/    {"SBCD" , i16XIndexed , 0 },
/*37C3*/    {"ADCD" , i16XIndexed , 0 },
/*37C4*/    {"EORD" , i16XIndexed , 0 },
/*37C5*/    {"LDD"  , i16XIndexed , 0 },
/*37C6*/    {"ANDD" , i16XIndexed , 0 },
/*37C7*/    {"ORD"  , i16XIndexed , 0 },
/*37C8*/    {"CPD"  , i16XIndexed , 0 },
/*37C9*/    {""     , iInvalid    , 0 },
/*37CA*/    {"STD"  , i16XIndexed , 0 },
/*37CB*/    {""     , iInvalid    , 0 },
/*37CC*/    {"XGDX" , iInherent   , 0 },
/*37CD*/    {"ADX"  , iInherent   , 0 },
/*37CE*/    {""     , iInvalid    , 0 },
/*37CF*/    {""     , iInvalid    , 0 },

/*37D0*/    {"SUBD" , i16YIndexed , 0 },
/*37D1*/    {"ADDD" , i16YIndexed , 0 },
/*37D2*/    {"SBCD" , i16YIndexed , 0 },
/*37D3*/    {"ADCD" , i16YIndexed , 0 },
/*37D4*/    {"EORD" , i16YIndexed , 0 },
/*37D5*/    {"LDD"  , i16YIndexed , 0 },
/*37D6*/    {"ANDD" , i16YIndexed , 0 },
/*37D7*/    {"ORD"  , i16YIndexed , 0 },
/*37D8*/    {"CPD"  , i16YIndexed , 0 },
/*37D9*/    {""     , iInvalid    , 0 },
/*37DA*/    {"STD"  , i16YIndexed , 0 },
/*37DB*/    {""     , iInvalid    , 0 },
/*37DC*/    {"XGDY" , iInherent   , 0 },
/*37DD*/    {"ADY"  , iInherent   , 0 },
/*37DE*/    {""     , iInvalid    , 0 },
/*37DF*/    {""     , iInvalid    , 0 },

/*37E0*/    {"SUBD" , i16ZIndexed , 0 },
/*37E1*/    {"ADDD" , i16ZIndexed , 0 },
/*37E2*/    {"SBCD" , i16ZIndexed , 0 },
/*37E3*/    {"ADCD" , i16ZIndexed , 0 },
/*37E4*/    {"EORD" , i16ZIndexed , 0 },
/*37E5*/    {"LDD"  , i16ZIndexed , 0 },
/*37E6*/    {"ANDD" , i16ZIndexed , 0 },
/*37E7*/    {"ORD"  , i16ZIndexed , 0 },
/*37E8*/    {"CPD"  , i16ZIndexed , 0 },
/*37E9*/    {""     , iInvalid    , 0 },
/*37EA*/    {"STD"  , i16ZIndexed , 0 },
/*37EB*/    {""     , iInvalid    , 0 },
/*37EC*/    {"XGDZ" , iInherent   , 0 },
/*37ED*/    {"ADZ"  , iInherent   , 0 },
/*37EE*/    {""     , iInvalid    , 0 },
/*37EF*/    {""     , iInvalid    , 0 },

/*37F0*/    {"SUBD" , iExtended   , 0 },
/*37F1*/    {"ADDD" , iExtended   , 0 },
/*37F2*/    {"SBCD" , iExtended   , 0 },
/*37F3*/    {"ADCD" , iExtended   , 0 },
/*37F4*/    {"EORD" , iExtended   , 0 },
/*37F5*/    {"LDD"  , iExtended   , 0 },
/*37F6*/    {"ANDD" , iExtended   , 0 },
/*37F7*/    {"ORD"  , iExtended   , 0 },
/*37F8*/    {"CPD"  , iExtended   , 0 },
/*37F9*/    {""     , iInvalid    , 0 },
/*37FA*/    {"STD"  , iExtended   , 0 },
/*37FB*/    {""     , iInvalid    , 0 },
/*37FC*/    {"TPA"  , iInherent   , 0 },
/*37FD*/    {"TAP"  , iInherent   , 0 },
/*37FE*/    {"MOVB" , iMOVEXT     , 0 },
/*37FF*/    {"MOVW" , iMOVEXT     , 0 },
};


static const char xyz[] = "XYZ";
static const char *pushregs[] = { "CCR", "K", "Z", "Y", "X", "E", "D" };


void Dis68HC16::hint_opcode(int hint, addr_t addr, char *opcode)
{
    struct ops {
        uint16_t op;
        const char *newop;
    } ops[] = {
        {   0xB4, "BHS"  },
        {   0xB5, "BLO"  },
        { 0x3784, "LBHS" },
        { 0x3785, "LBLO" },
        {   0   ,  NULL  }
    };

    int opcd = ReadByte(addr);
    if (opcd == 0x37) {
        opcd = opcd*256 + ReadByte(addr+1);
    }

    if (hint & 1) {
        for (struct ops *p = ops; p->op; p++) {
            if (opcd == p->op) {
                strcpy(opcode, p->newop);
                return;
            }
        }
    }
}


int Dis68HC16::dis_line(addr_t addr, char *opcode, char *parms, int &lfref, addr_t &refaddr)
{
    unsigned int    ad;
    int             op;
    int             i;
    InstrPtr        instr;
    char            s[256];
    int             r1, r2;
    addr_t          ra;
    char            *p;

    ad    = addr;
    op    = ReadByte(ad++);
    int len = 1;
    int hint = rom.get_hint(addr);

    if (addr & 0x01) { // require even alignment!
        opcode[0] = 0;
        goto invalid;
    }

    switch (op) {
        default:
            instr = &M68HC16_opcdTable[op];
            break;

        case 0x17:
            i     = ReadByte(ad++);
            op    = (op << 8) | i;
            len   = 2;
            instr = &M68HC16_opcdTable17[i];
            break;

        case 0x27:
            i     = ReadByte(ad++);
            op    = (op << 8) | i;
            len   = 2;
            instr = &M68HC16_opcdTable27[i];
            break;

        case 0x37:
            i     = ReadByte(ad++);
            op    = (op << 8) | i;
            len   = 2;
            instr = &M68HC16_opcdTable37[i];
            break;
    }

    strcpy(opcode, instr->op);
    lfref  = instr->lfref;
    parms[0]  = 0;
    refaddr = 0;

    hint_opcode(hint, addr, opcode);
    switch (instr->typ) {
        case iInherent:
            /* nothing */;
            break;

        case i8Immediate:
            H2Str(ReadByte(ad++), s);
            len++;
            sprintf(parms, "#%s", s);
            break;

        case i16Immediate:
            ra = ReadWord(ad);
            len += 2;
            i = ReadByte(addr);
#if 0
            r1 = 1; // default to no reference
#else
            // Only CPX/Y/Z/S and LDX/Y/Z/D should generate references by default
            r1 = !((op & 0xFFFC) == 0x377C || (op & 0xFFFC) == 0x37BC);
#endif
            // override reference / no-reference with an odd hint
            if (r1 ^ !!(hint & 1)) {
                H4Str(ra, s);
                sprintf(parms, "#%s", s);
            } else {
                RefStr4(ra, s, lfref, refaddr);
                sprintf(parms, "#%s", s);
            }
            break;

        case iMAC:
            r1 = ReadByte(ad++);
            len++;
            sprintf(parms, "%d,%d", r1 >> 4, r1 & 0x0F);
            break;

        case iPSH:
        case iPUL:
            r1 = ReadByte(ad++);
            len++;
            p = parms;
            for (i = 0; i <= 6; i++) {
                if (r1 & (1 << i)) {
                     if (p != parms) {
                         *p++ = ',';
                     }
                     if (instr->typ == iPSH) {
                         strcat(p, pushregs[6-i]);
                     } else {
                         strcat(p, pushregs[i]);
                     }
                     p += strlen(p);
                }
            }
            break;

        case iMOV: // op dd addr
            r2 = ReadByte(ad++);
            r1 = ReadWord(ad);
            len += 3;
            p = parms;
            if (op & 2) {
                H4Str(r1, parms);
                strcat(p, ",");
                p += strlen(p);
            }
            H2Str(r2, p);
            strcat(p, ",X");
            p += strlen(p);
            p += strlen(p);
            if (!(op & 2)) {
                strcat(p, ",");
                p += strlen(p);
                H4Str(r1, p);
            }
            break;

        case iMOVEXT: // opcd addr addr
            r1 = ReadWord(ad);
            ad += 2;
            r2 = ReadWord(ad);
            ad += 2;
            len += 4;
            H4Str(r1, parms);
            strcat(parms, ",");
            H4Str(r2, parms + strlen(parms));
            break;

        case iExtended:
            ra = ReadWord(ad);
            ad += 2;
            len += 2;
#if 0
            H4Str(ra, s);
#else
            RefStr4(ra, s, lfref, refaddr);
#endif
            strcpy(parms, s);
            break;

        case i20Extended:
            r2 = ReadByte(ad++) & 0x0F;
            ra = ReadWord(ad);
            ad += 2;
            len += 3;
            i = (r2 << 16) + ReadWord(ad++);
            RefStr(ra, s, lfref, refaddr);
            strcpy(parms, s);
            break;

        case i8Relative:
            i = ReadByte(ad++) & 0xFE;
            if (i == 0xFF && (lfref & REFFLAG)) {
                // offset FF is suspicious
                lfref |= RIPSTOP;
            }
            len++;
            if (i >= 128) {
                i = i - 256;
            }
            // note that due to 68HC16 pipelining, all branches
            // are relative to the first address + 6
            ra = (addr + 6 + i) & 0xFFFFF;
            i = lfref; // ignore flag changes from RefStr
            RefStr(ra, parms, lfref, refaddr);
            // parms += strlen(parms);
            lfref = i;
            break;

        case i16Relative:
            i = ReadWord(ad) & 0xFFFE;
            ad += 2;
            len += 2;
            if (i >= 32767) {
                i = i - 65536;
            }
            ra = (addr + 6 + i) & 0xFFFFF;
            i = lfref; // ignore flag changes from RefStr
            RefStr(ra, parms, lfref, refaddr);
            // parms += strlen(parms);
            lfref = i;
            break;

        case iEXIndexed:
        case iEYIndexed:
        case iEZIndexed:
            r1 = xyz[instr->typ - iEXIndexed];
            sprintf(parms, "E,%c", r1);
            break;

        case i8XIndexed:
        case i8YIndexed:
        case i8ZIndexed:
            r1 = xyz[instr->typ - i8XIndexed];
            i = ReadByte(ad++);
            len++;
            H2Str(i, s);
            sprintf(parms,"%s,%c", s, r1);
            break;

        case i16XIndexed:
        case i16YIndexed:
        case i16ZIndexed:
            r1 = xyz[instr->typ - i16XIndexed];
            i = ReadWord(ad++);
            len += 2;
            H4Str(i, s);
            sprintf(parms,"%s,%c", s, r1);
            break;

        case ibExtended:
            r2 = ReadByte(ad++);
            ra = ReadWord(ad);
            ad += 2;
            len += 3;
            RefStr4(ra, s, lfref, refaddr);
            sprintf(parms,"%s,#$%.2X", s, r2);
            break;

        case ib8XIndexed:
        case ib8YIndexed:
        case ib8ZIndexed:
            r1 = xyz[instr->typ - ib8XIndexed];
            r2 = ReadByte(ad++);
            i = ReadByte(ad++);
            len += 2;
            H2Str(i, s);
            sprintf(parms,"%s,%c,#$%.2X", s, r1, r2);
            break;

        case ibWExtended:
            r2 = ReadWord(ad);
            ad += 2;
            i = ReadWord(ad);
            ad += 2;
            len += 4;
            RefStr4(i, s, lfref, refaddr);
            sprintf(parms,"%s,#$%.4X", s, r2);
            break;

        case ib16XIndexed:
        case ib16YIndexed:
        case ib16ZIndexed:
            r1 = xyz[instr->typ - ib16XIndexed];
            r2 = ReadByte(ad++);
            i = ReadWord(ad);
            ad += 2;
            len += 3;
            H4Str(i, s);
            sprintf(parms,"%s,%c,#$%.2X", s, r1, r2);
            break;

        case i20XIndexed:
        case i20YIndexed:
        case i20ZIndexed:
            r1 = xyz[instr->typ - i20XIndexed];
            r2 = ReadByte(ad++) & 0x0F;
            i = (r2 << 16) + ReadWord(ad++);
            len += 3;
            H6Str(i, s);
            sprintf(parms,"%s,%c", s, r1);
            break;

        case ib8XRelative:
        case ib8YRelative:
        case ib8ZRelative:
            r1 = xyz[instr->typ - ib8XRelative];
            r2 = ReadByte(ad++); // mask
            i = ReadByte(ad++); // X index
            len += 3;
            H2Str(i, s);
            sprintf(parms, "%s,%c,#", s, r1);
            H2Str(r2, s);
            p = parms + strlen(parms);
            sprintf(p, "%s,", s);
            p += strlen(p);
            i = ReadByte(ad++);
            if (i > 127) {
                i = i - 256;
            }
            ra = (addr + 6 + i) & 0xFFFFF;
            RefStr(ra, p, lfref, refaddr);
            break;

        case ibExtRelative:
        case ib16XRelative:
        case ib16YRelative:
        case ib16ZRelative:
            r1 = xyz[instr->typ - ib16XRelative];
            r2 = ReadByte(ad++); // mask
            i = ReadWord(ad); // index offset or ext address
            ad += 2;
            len += 5;
            H4Str(i, s);
            if (instr->typ == ibExtRelative) {
                sprintf(parms, "%s,#", s);
            } else {
                sprintf(parms, "%s,%c,#", s, r1);
            }
            H2Str(r2, s);
            p = parms + strlen(parms);
            sprintf(p, "%s,", s);
            p += strlen(p);
            i = ReadWord(ad);
            ad += 2;
            if (i > 32767) {
                i = i - 65536;
            }
            ra = (addr + 6 + i) & 0xFFFFF;
            RefStr(ra, p, lfref, refaddr);
            break;

        case iInvalid:
            len = 0;
            break;
    }

invalid:
    if (opcode[0] == 0 || len == 0 || rom.AddrOutRange(addr)) {
        strcpy(opcode, "???");
        H2Str(ReadByte(addr), parms);
        len     = 0;
        lfref   = 0;
        refaddr = 0;
    }

    return len;
}

