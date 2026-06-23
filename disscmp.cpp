// disscmp.c
// disassembler for National Semiconductor SC/MP

// hints usage:
//    XPPC: disable lfflag
//    LDPI and JS: disable peephole detection

static const char versionName[] = "SC/MP disassembler";

#include "discpu.h"

struct InstrRec {
    uint8_t     opcode; // 
    uint8_t     mask;   // 
    uint8_t     typ;    //
    const char  *op;    // mnemonic
    uint8_t     lfref;  // lfFlag/refFlag/codeRef
};
typedef const struct InstrRec *InstrPtr;


class DisSCMP : public CPU {
public:
    DisSCMP(const char *name, int subtype, int endian, int addrwid,
           char curAddrChr, char hexChr, const char *byteOp,
           const char *wordOp, const char *longOp);

    virtual int dis_line(addr_t addr, char *opcode, char *parms, int &lfref, addr_t &refaddr);
};


DisSCMP cpu_SCMP("SCMP", 0, BIG_END, ADDR_16, '$', 'H', "DB", "DW", "DL");


DisSCMP::DisSCMP(const char *name, int subtype, int endian, int addrwid,
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

    add_cpu();
}


// =====================================================


enum InstType {
    o_Invalid,      // 0
    o_Implied,      // - no operands
    o_Immed,        // - immediate byte
    o_PtrJmp,       // - JMP/JP/JZ/JNZ
    o_Ptr,          // - indexed ptr + disp
    o_MPtr,         // - indexed mptr + disp
    o_XP,           // - XPAL/XPAH/XPPC
};


// first byte in bits 15-8, second byte in bits 7-0
static const struct InstrRec *FindInstr(const struct InstrRec *table, uint16_t opcode)
{
    for ( ; table->mask; table++) {
        if (table->opcode == (opcode & table->mask)) {
            return table;
        }
    }

    return NULL;
}


static const struct InstrRec opcdTable_SCMP[] =
{
    // op   mask     typ      opcd   lfref
    { 0x00, 0xFF, o_Implied, "HALT", 0 },
    { 0x01, 0xFF, o_Implied, "XAE" , 0 },
    { 0x02, 0xFF, o_Implied, "CCL" , 0 },
    { 0x03, 0xFF, o_Implied, "SCL" , 0 },
    { 0x04, 0xFF, o_Implied, "DINT", 0 },
    { 0x05, 0xFF, o_Implied, "IEN" , 0 },
    { 0x06, 0xFF, o_Implied, "CSA" , 0 },
    { 0x07, 0xFF, o_Implied, "CAS" , 0 },
    { 0x08, 0xFF, o_Implied, "NOP" , 0 },

    { 0x19, 0xFF, o_Implied, "SIO" , 0 },

    { 0x1C, 0xFF, o_Implied, "SR"  , 0 },
    { 0x1D, 0xFF, o_Implied, "SRL" , 0 },
    { 0x1E, 0xFF, o_Implied, "RR"  , 0 },
    { 0x1F, 0xFF, o_Implied, "RRL" , 0 },

    { 0x30, 0xFC, o_XP,      "XPAL", 0 },
    { 0x34, 0xFC, o_XP,      "XPAH", 0 },
    { 0x3C, 0xFC, o_XP,      "XPPC", LFFLAG },

    { 0x40, 0xFF, o_Implied, "LDE" , 0 },
    { 0x50, 0xFF, o_Implied, "ANE" , 0 },
    { 0x58, 0xFF, o_Implied, "ORE" , 0 },
    { 0x60, 0xFF, o_Implied, "XRE" , 0 },
    { 0x68, 0xFF, o_Implied, "DAE" , 0 },
    { 0x70, 0xFF, o_Implied, "ADE" , 0 },
    { 0x78, 0xFF, o_Implied, "CAE" , 0 },

    { 0x8F, 0xFF, o_Immed,   "DLY" , 0 },

    { 0x90, 0xFC, o_PtrJmp,  "JMP" , REFFLAG | CODEREF | LFFLAG },
    { 0x94, 0xFC, o_PtrJmp,  "JP"  , REFFLAG | CODEREF },
    { 0x98, 0xFC, o_PtrJmp,  "JZ"  , REFFLAG | CODEREF },
    { 0x9C, 0xFC, o_PtrJmp,  "JNZ" , REFFLAG | CODEREF },

    { 0xA8, 0xFC, o_Ptr,     "ILD" , 0 },
    { 0xB8, 0xFC, o_Ptr,     "DLD" , 0 },

    { 0xC4, 0xFF, o_Immed,   "LDI" , 0 },
    { 0xCC, 0xFF, o_Invalid, ""    , 0 }, // there is no store immediate
    { 0xD4, 0xFF, o_Immed,   "ANI" , 0 },
    { 0xDC, 0xFF, o_Immed,   "ORI" , 0 },
    { 0xE4, 0xFF, o_Immed,   "XRI" , 0 },
    { 0xEC, 0xFF, o_Immed,   "DAI" , 0 },
    { 0xF4, 0xFF, o_Immed,   "ADI" , 0 },
    { 0xFC, 0xFF, o_Immed,   "CAI" , 0 },

    { 0xC0, 0xF8, o_MPtr,    "LD"  , 0 },
    { 0xC8, 0xF8, o_MPtr,    "ST"  , 0 },
    { 0xD0, 0xF8, o_MPtr,    "AND" , 0 },
    { 0xD8, 0xF8, o_MPtr,    "OR"  , 0 },
    { 0xE0, 0xF8, o_MPtr,    "XOR" , 0 },
    { 0xE8, 0xF8, o_MPtr,    "DAD" , 0 },
    { 0xF0, 0xF8, o_MPtr,    "ADD" , 0 },
    { 0xF8, 0xF8, o_MPtr,    "CAD" , 0 },

    { 0x00, 0x00, o_Invalid, ""    , 0 }
};


// =====================================================


int DisSCMP::dis_line(addr_t addr, char *opcode, char *parms, int &lfref, addr_t &refaddr)
{
    addr_t      ad;
    addr_t      ra;
    int         i;
    int         n;
    InstrPtr    instr;
    char        *p;
    char        s[256];

    bool invalid = false;
    int hint = rom.get_hint(addr);

    // get first byte of instruction
    ad = addr;
    int op = ReadByte(ad++);
    int len = 1;
    refaddr = 0;

    // get instruction information
    instr = FindInstr(opcdTable_SCMP, op);
    lfref = instr->lfref;
    strcpy(opcode, instr->op);

    switch (instr->typ) {
        case o_Implied:  // no operands
            break;

        case o_Immed:    // immediate byte
            n = ReadByte(ad++);
            len++;
            H2Str(n, parms);
            break;

        case o_Ptr:      // indexed ptr + disp
        case o_PtrJmp:   // JMP/JP/JZ/JNZ
            n = ReadByte(ad++);
            len++;
            if (op & 0x03) {
                p = s;
                if (n > 128) {
                    *p++ = '-';
                    n = 256 - n;
                }
                if (n < 16) {
                    sprintf(p, "%d", n);
                } else {
                    H2Str(n, p);
                }
                sprintf(parms + strlen(parms), "%s(P%d)", s, op & 0x03);
            } else {
                if (n >= 128) {
                    n = n - 256;
                }
                ra = ad + n;
                RefStr(ra, parms, lfref, refaddr);
            }
            break;

        case o_MPtr:     // indexed mptr + disp
            i = n = ReadByte(ad++);
            len++;
            p = s;
            if (n > 128) {
                *p++ = '-';
                n = 256 - n;
            }
            if (n < 16) {
                sprintf(p, "%d", n);
            } else {
                H2Str(n, p);
            }

            switch (op & 0x07) {
                case 0: // PC relative
                    if (i >= 128) {
                        i = i - 256;
                    }
                    // address is relative to the offset byte
                    ra = addr + 1 + i;
                    RefStr(ra, s, lfref, refaddr);
                    sprintf(parms, "%s", s);
                    break;
                case 1: case 2: case 3:
                    sprintf(parms, "%s(P%d)", s, op & 0x03);
                    break;
                case 4: // immediate (should be handled as o_Immed)
                    invalid = true;
                    break;
                case 5: case 6: case 7:
                    sprintf(parms, "@%s(P%d)", s, op & 0x03);
                    break;
                default: // should not get here
                    invalid = true;
                    break;
            }
            break;

        case o_XP:       // XPAL/XPAH/XPPC
            sprintf(parms, "P%d", op & 0x03);
            if (hint & 0x01) {
                // disable lfflag if odd hint
                lfref &= ~LFFLAG;
            }
            break;

        default:
            invalid = true;
            break;
    }

    // invalid instruction handler, including the case where it ran out of bytes
    if (invalid || opcode==NULL || opcode[0]==0 || rom.AddrOutRange(addr)) {
        strcpy(opcode, "???");
        H2Str(ReadByte(addr), parms);
        len      = 0; // illegal instruction
        lfref    = 0;
        refaddr  = 0;
    }

    // rip-stop checks and peephole special cases
    if (opcode[0]) {
        int op = ReadByte(addr);
        switch (op) {
#if 0 // might not be a problem on SC/MP?
            case 0x00: // repeated all zeros or all ones
            case 0xFF:
                if (ReadByte(addr+1) == op) {
                    lfref |= RIPSTOP;
                }
                break;
#endif
            // handle LDPI and JS
            case 0xC4:
                // LDPI if C4 -- 35-37 C4 -- 31-33
                if (ReadByte(addr+3) == op && !(hint & 1)) {
                    i = ReadByte(addr+2);
                    if (0x35 <= i && i <= 0x37 && i-4 == ReadByte(addr+5)) {
                         ra = (ReadByte(addr+1)<<8) + ReadByte(addr+4);
                         len = 6;
                         strcpy(opcode, "LDPI");
                         if (i+8 == ReadByte(addr+6)) {
                             // JS if LDPI + 3D-3F
                             lfref |= CODEREF;
                             strcpy(opcode, "JS");
                             len++;
                             ra++; // JS uses addr-1
                         }
                         RefStr(ra, s, lfref, refaddr);
                         sprintf(parms, "P%d,%s", i & 0x03, s);
                    }
                }
        }
    }

    return len;
}


