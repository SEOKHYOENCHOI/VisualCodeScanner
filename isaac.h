#pragma once
typedef struct isaac_ctx isaac_ctx;



#define ISAAC_SZ_LOG      (8)
#define ISAAC_SZ          (1<<ISAAC_SZ_LOG)
#define ISAAC_SEED_SZ_MAX (ISAAC_SZ<<2)

struct isaac_ctx {
	unsigned n;
	unsigned r[ISAAC_SZ];
	unsigned m[ISAAC_SZ];
	unsigned a;
	unsigned b;
	unsigned c;
};


void isaac_init(isaac_ctx *_ctx, const void *_seed, int _nseed);

unsigned isaac_next_uint32(isaac_ctx *_ctx);
unsigned isaac_next_uint(isaac_ctx *_ctx, unsigned _n);