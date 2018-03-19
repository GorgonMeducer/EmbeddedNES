#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifndef COMMON_H
#define COMMON_H

typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;
typedef uint64_t qword;

#if false
#define common_bit_set(__VALUE, __POS)      ((__VALUE) & (1L << (__POS)))

#define __common_set_bit(__ADDR, __POS)    do { *(__ADDR) |= 1L << (__POS);    } while(0)
#define __common_unset_bit(__ADDR, __POS)  do { *(__ADDR) &= ~(1L << (__POS)); } while(0)
#define __common_toggle_bit(__ADDR, __POS) do { *(__ADDR) ^= 1L << (__POS);    } while(0)

#define __common_modify_bit(__ADDR, __POS, __SET)                               \
    do {                                                                        \
        if (__SET) {                                                            \
            __common_set_bit((__ADDR), (__POS));                                \
        } else {                                                                \
            __common_unset_bit((__ADDR), (__POS));                              \
        }                                                                       \
    } while(0) 

#define common_set_bitb(__ADDR, __POS)              __common_set_bit((__ADDR), (__POS))
#define common_unset_bitb(__ADDR, __POS)            __common_unset_bit((__ADDR), (__POS))
#define common_toggle_bitb(__ADDR, __POS)           __common_toggle_bit((__ADDR), (__POS))
#define common_modify_bitb(__ADDR, __POS, __SET)    __common_modify_bit((__ADDR), (__POS), (__SET))

#define common_set_bitw(__ADDR, __POS)              __common_set_bit((__ADDR), (__POS))
#define common_unset_bitw(__ADDR, __POS)            __common_unset_bit((__ADDR), (__POS))
#define common_toggle_bitw(__ADDR, __POS)           __common_toggle_bit((__ADDR), (__POS))
#define common_modify_bitw(__ADDR, __POS, __SET)    __common_modify_bit((__ADDR), (__POS), (__SET))

#else


// Binary Operations
inline bool common_bit_set(uint_fast32_t value, uint_fast8_t position)
{
    return value & (1L << position);
}




#define M_common(SUFFIX, TYPE) \
    inline void common_set_bit##SUFFIX(TYPE *variable, uint_fast8_t position)    { *variable |= 1L << position;    } \
    inline void common_unset_bit##SUFFIX(TYPE *variable, uint_fast8_t position)  { *variable &= ~(1L << position); } \
    inline void common_toggle_bit##SUFFIX(TYPE *variable, uint_fast8_t position) { *variable ^= 1L << position;    } \
    inline void common_modify_bit##SUFFIX(TYPE *variable, uint_fast8_t position, bool set) \
        { set ? common_set_bit##SUFFIX(variable, position) : common_unset_bit##SUFFIX(variable, position); } 

M_common(b, byte)
M_common(w, word)
M_common(d, dword)
M_common(q, qword)


#endif



#if false
// Byte Bit Operations
void common_set_bitb(byte *variable, byte position);
void common_unset_bitb(byte *variable, byte position);
void common_toggle_bitb(byte *variable, byte position);
void common_modify_bitb(byte *variable, byte position, bool set);

// Word Bit Operations
void common_set_bitw(word *variable, byte position);
void common_unset_bitw(word *variable, byte position);
void common_toggle_bitw(word *variable, byte position);
void common_modify_bitw(word *variable, byte position, bool set);

// Double Word Bit Operations
void common_set_bitd(dword *variable, byte position);
void common_unset_bitd(dword *variable, byte position);
void common_toggle_bitd(dword *variable, byte position);
void common_modify_bitd(dword *variable, byte position, bool set);

// Quad Word Bit Operations
void common_set_bitq(qword *variable, byte position);
void common_unset_bitq(qword *variable, byte position);
void common_toggle_bitq(qword *variable, byte position);
void common_modify_bitq(qword *variable, byte position, bool set);
#endif
#endif
