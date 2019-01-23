/******************************************************************************
* Copyright 2017-2022 liuweiqiang@leelen.cn
* FileName: 	 crypto1.h 
* Desc:
* 
* 
* Author: 	 liuwq
* Date: 	 2017/05/23
* Notes: 
* 
* ----------------------------------------------------------------------------
* Histroy: v1.0   2017/05/23, liuwq create this file
* 
******************************************************************************/
#ifndef _CRYPTO1_H_     
#define _CRYPTO1_H_    
 
 
/*------------------------------- Includes ----------------------------------*/
 #include <stdint.h>
// #include "crapto1.h"
/*----------------------------- Global Typedefs -----------------------------*/
 

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

  struct Crypto1State {uint32_t odd, even;};
  struct Crypto1State *crypto1_create(uint64_t);
	
  void crypto1_destroy(struct Crypto1State *);
  void crypto1_get_lfsr(struct Crypto1State *, uint64_t *);
  uint8_t crypto1_bit(struct Crypto1State *, uint8_t, int);
  uint8_t crypto1_byte(struct Crypto1State *, uint8_t, int);
  uint32_t crypto1_word(struct Crypto1State *, uint32_t, int);
  uint32_t prng_successor(uint32_t x, uint32_t n);

  struct Crypto1State *lfsr_recovery32(uint32_t ks2, uint32_t in);
  struct Crypto1State *lfsr_recovery64(uint32_t ks2, uint32_t ks3);

  void lfsr_rollback(struct Crypto1State *s, uint32_t in, int fb);
  uint32_t lfsr_rollback_word(struct Crypto1State *s, uint32_t in, int fb);
  int nonce_distance(uint32_t from, uint32_t to);
#define FOREACH_VALID_NONCE(N, FILTER, FSIZE)\
  uint32_t __n = 0,__M = 0, N = 0;\
  int __i;\
  for(; __n < 1 << 16; N = prng_successor(__M = ++__n, 16))\
    for(__i = FSIZE - 1; __i >= 0; __i--)\
      if(BIT(FILTER, __i) ^ parity(__M & 0xFF01))\
        break;\
      else if(__i)\
        __M = prng_successor(__M, (__i == 7) ? 48 : 8);\
      else

#define LF_POLY_ODD (0x29CE5C)
#define LF_POLY_EVEN (0x870804)
#define BIT(x, n) ((x) >> (n) & 1)
#define BEBIT(x, n) BIT(x, (n) ^ 24)
  static __inline int parity(uint32_t x)
  {
#if !defined __i386__ || !defined __GNUC__
    x ^= x >> 16;
    x ^= x >> 8;
    x ^= x >> 4;
    return BIT(0x6996, x & 0xf);
#else
    __asm__("movl %1, %%eax\n"
            "mov %%ax, %%cx\n"
            "shrl $0x10, %%eax\n"
            "xor %%ax, %%cx\n"
            "xor %%ch, %%cl\n"
            "setpo %%al\n"
            "movzx %%al, %0\n": "=r"(x) : "r"(x): "eax", "ecx");
    return x;
#endif
  }
  static __inline int filter(uint32_t const x)
  {
    uint32_t f;

    f  = 0xf22c0 >> (x       & 0xf) & 16;
    f |= 0x6c9c0 >> (x >>  4 & 0xf) &  8;
    f |= 0x3c8b0 >> (x >>  8 & 0xf) &  4;
    f |= 0x1e458 >> (x >> 12 & 0xf) &  2;
    f |= 0x0d938 >> (x >> 16 & 0xf) &  1;
    return BIT(0xEC57E80A, f);
  }
	
	
#ifdef __cplusplus
}
#endif
 
/*----------------------------- Global Defines ------------------------------*/
 
 
/*----------------------------- External Variables --------------------------*/
 
 
/*------------------------ Global Function Prototypes -----------------------*/
 struct Crypto1State *crypto1_create(uint64_t key);
 void crypto1_destroy(struct Crypto1State *state);
 uint8_t crypto1_byte(struct Crypto1State *s, uint8_t in, int is_encrypted);
 

#endif //_CRYPTO1_H_
