/*
 * Lyra2RE kernel implementation.
 *
 * ==========================(LICENSE BEGIN)============================
 * Copyright (c) 2014 djm34
 * Copyright (c) 2014 James Lovejoy
 * Copyright (c) 2017 djm34
 * Copyright (c) 2018 KL0nLutiy
 * Copyright (c) 2018 fancyIX
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * ===========================(LICENSE END)=============================
 *
 * @author   djm34
 * @author   fancyIX 2018
 */
// typedef unsigned int uint;
//#pragma OPENCL EXTENSION cl_amd_printf : enable

#ifndef LYRA2Z_CL
#define LYRA2Z_CL


#if __ENDIAN_LITTLE__
#define SPH_LITTLE_ENDIAN 1
#else
#define SPH_BIG_ENDIAN 1
#endif

#define SPH_UPTR sph_u64

typedef unsigned int sph_u32;
typedef int sph_s32;
#ifndef __OPENCL_VERSION__
typedef unsigned long sph_u64;
typedef long  sph_s64;
#else
typedef unsigned long sph_u64;
typedef long sph_s64;
#endif


#define SPH_64 1
#define SPH_64_TRUE 1

#define SPH_C32(x)    ((sph_u32)(x ## U))
#define SPH_T32(x)    ((x) & SPH_C32(0xFFFFFFFF))

#define SPH_C64(x)    ((sph_u64)(x ## UL))
#define SPH_T64(x)    ((x) & SPH_C64(0xFFFFFFFFFFFFFFFF))

#define SPH_ROTL32(x,n) rotate(x,(uint)n)     //faster with driver 14.6
#define SPH_ROTR32(x,n) rotate(x,(uint)(32-n))
#define SPH_ROTL64(x,n) rotate(x,(ulong)n)
#define SPH_ROTR64(x,n) rotate(x,(ulong)(64-n))


#define SWAP4(x) as_uint(as_uchar4(x).wzyx)
#define SWAP8(x) as_ulong(as_uchar8(x).s76543210)
#define SWAP32(x) as_ulong(as_uint2(x).s10)

#include "lyra2mdzf2.cl"

typedef union {
    uint h4[8];
    ulong h8[4];
    uint4 h16[2];
    ulong2 hl16[2];
    ulong4 h32;
} hash2_t;

typedef union {
    uint h4[32];
    ulong h8[16];
    uint4 h16[8];
    ulong2 hl16[8];
    ulong4 h32[4];
} lyraState_t;


/// lyra2 algo p2 

__attribute__((reqd_work_group_size(4, 5, 1)))
__kernel void search2(__global uchar* sharedDataBuf)
{
  uint gid = get_global_id(1);
  __global lyraState_t *lyraState = (__global lyraState_t *)(sharedDataBuf + ((8 * 4  * 4) * (gid-get_global_offset(1))));

  //__global ulong *notepad = buffer + get_local_id(0) + 4 * SLOT;
  uint2 notepad[192];
  const int player = get_local_id(0);

  uint2 state[4];
  uint2 si[4], sII[4];

  //-------------------------------------
  // Load Lyra state
  state[0] = as_uint2((ulong)(lyraState->h8[player]));
  state[1] = as_uint2((ulong)(lyraState->h8[player+4]));
  state[2] = as_uint2((ulong)(lyraState->h8[player+8]));
  state[3] = as_uint2((ulong)(lyraState->h8[player+12]));
  
  for (int loop = 0; loop < LYRA_ROUNDS; loop++) { // write columns and rows 'in order'
    for(int cp = 0; cp < 3; cp++) notepad[HYPERMATRIX_COUNT - STATE_BLOCK_COUNT * (loop + 1) + cp * REG_ROW_COUNT] = state[cp];
    round_lyra_4way_sw(state);
  }

  make_hyper_one_macro(state, notepad);
  
  make_next_hyper_macro(1, 0, 2, state, notepad);
  make_next_hyper_macro(2, 1, 3, state, notepad);
  make_next_hyper_macro(3, 0, 4, state, notepad);
  make_next_hyper_macro(4, 3, 5, state, notepad);
  make_next_hyper_macro(5, 2, 6, state, notepad);
  make_next_hyper_macro(6, 1, 7, state, notepad);
  
  uint modify;
  uint prev = 7;
  uint iterator = 0;
  for (uint j = 0; j < LYRA_ROUNDS / 2; j++) {
    for (uint i = 0; i<LYRA_ROUNDS; i++) {
      modify = broadcast_zero((uint)(as_ulong(state[0]) % 8));
      hyper_xor_dpp_macro(prev, modify, iterator, state, notepad);
      prev = iterator;
      iterator = (iterator + 3) & 7;
    }
    for (uint i = 0; i<LYRA_ROUNDS; i++) {
      modify = broadcast_zero((uint)(as_ulong(state[0]) % 8));
      hyper_xor_dpp_macro(prev, modify, iterator, state, notepad);
      prev = iterator;
      iterator = (iterator - 1) & 7;
    }
  }

  for(int loop = 0; loop < 3; loop++) state[loop] ^= notepad[HYPERMATRIX_COUNT * modify + loop * REG_ROW_COUNT];

  //-------------------------------------
  // save lyra state    
  lyraState->h8[player] = as_ulong(state[0]);
  lyraState->h8[player+4] = as_ulong(state[1]);
  lyraState->h8[player+8] = as_ulong(state[2]);
  lyraState->h8[player+12] = as_ulong(state[3]);

  barrier(CLK_GLOBAL_MEM_FENCE);
}



#endif // LYRA2Z_CL