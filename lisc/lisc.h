#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int uint;

typedef struct Bits Bits;
typedef struct Ref Ref;
typedef struct OpDesc OpDesc;
typedef struct Ins Ins;
typedef struct Phi Phi;
typedef struct Blk Blk;
typedef struct Sym Sym;
typedef struct Cons Cons;
typedef struct Fn Fn;

typedef enum { U, F, T } B3;

enum {
	RAX = 1, /* caller-save */
	RCX,
	RDX,
	RSI,
	RDI,
	R8,
	R9,
	R10,
	R11,

	RBX, /* callee-save */
	R12,
	R13,
	R14,
	R15,

	RBP, /* reserved */
	RSP,

	// NReg = R15 - RAX + 1
	NReg = 3 /* for test purposes */
};

enum {
	Tmp0    = 33,

	NString = 32,
	NPred   = 15,
	NBlk    = 128,
	NIns    = 256,

	BITS    = 4,
	NBit    = 64,
};

struct Bits {
	uint64_t t[BITS];
};

#define BGET(b, n) (1&((b).t[n/NBit]>>(n%NBit)))
#define BSET(b, n) ((b).t[n/NBit] |= 1ll<<(n%NBit))
#define BCLR(b, n) ((b).t[n/NBit] &= ~(1ll<<(n%NBit)))

struct Ref {
	uint16_t type:2;
	uint16_t val:14;
};

enum {
	RSym,
	RCons,
	RSlot,
	NRef = (1<<14) - 1
};

#define R        (Ref){0, 0}
#define SYM(x)   (Ref){RSym, x}
#define CONS(x)  (Ref){RCons, x}
#define SLOT(x)  (Ref){RSlot, x}

static inline int req(Ref a, Ref b)
{ return a.type == b.type && a.val == b.val; }
static inline int rtype(Ref r)
{ return req(r, R) ? -1 : r.type; }

enum {
	OXXX = 0,
	/* public instruction */
	OAdd,
	OSub,
	ODiv,
	ORem,
	OStore,
	OLoad,
	/* reserved instructions */
	ONop,
	OCopy,
	OSwap,
	OSign,
	OXDiv,
	OLast
};

enum {
	CXXX,
	CWord,
	CLong,
};

enum {
	JXXX,
	JRet,
	JJmp,
	JJez,
};

struct OpDesc {
	char *name;
	int arity;
	B3 comm;
};

struct Ins {
	short op;
	Ref to;
	Ref arg[2];
};

struct Phi {
	Ref to;
	Ref arg[NPred];
	Blk *blk[NPred];
	uint narg;
	Phi *link;
};

struct Blk {
	Phi *phi;
	Ins *ins;
	uint nins;
	struct {
		short type;
		Ref arg;
	} jmp;
	Blk *s1;
	Blk *s2;
	Blk *link;

	int id;
	int visit;
	Blk **pred;
	uint npred;
	Bits in, out, gen;
	int nlive;
	int loop;
	char name[NString];
};

struct Sym {
	enum {
		SUndef,
		SReg,
		STmp,
	} type;
	char name[NString];
	int class;
	uint ndef, nuse;
	uint cost;
	uint spill;
	int hint;
};

struct Cons {
	enum {
		CUndef,
		CNum,
		CAddr,
	} type;
	char label[NString];
	int64_t val;
};

struct Fn {
	Blk *start;
	Sym *sym;
	Cons *cons;
	int ntmp;
	int nblk;
	Blk **rpo;
	uint nspill;
};


/* main.c */
extern char debug['Z'+1];
void dumpss(Bits *, Sym *, FILE *);

/* parse.c */
extern OpDesc opdesc[];
void diag(char *);
void *alloc(size_t);
Blk *blocka(void);
Fn *parsefn(FILE *);
void printfn(Fn *, FILE *);

/* ssa.c */
void fillpreds(Fn *);
void fillrpo(Fn *);
void ssafix(Fn *, int);

/* live.c */
void filllive(Fn *);

/* isel.c */
void isel(Fn *);

/* spill.c */
int bcnt(Bits *);
void fillcost(Fn *);
void spill(Fn *);

/* rega.c */
void rega(Fn *);

/* emit.c */
void emitfn(Fn *, FILE *);
