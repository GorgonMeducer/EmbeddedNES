/*****************************************************************************
 *   Copyright(C)2009-2019 by GorgonMeducer<embedded_zhuoran@hotmail.com>    *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *     http://www.apache.org/licenses/LICENSE-2.0                            *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 ****************************************************************************/
 


//#ifndef __PLOOC_CLASS_STRICT_H__          /* deliberately comment this out! */
//#define __PLOOC_CLASS_STRICT_H__          /* deliberately comment this out! */

/******************************************************************************
 * HOW TO USE                                                                 *
 ******************************************************************************/
//!Add following content to your module header file, e.g. xxxxxx.h
//#include "plooc_class.h"       


/*============================ INCLUDES ======================================*/
//#include <stdint.h>

/*! \NOTE the uint_fast8_t used in this header file is defined in stdint.h 
          if you don't have stdint.h supported in your toolchain, you should
          define uint_fast8_t all by yourself with following rule:
          a. if the target processor is 8 bits, define it as uint8_t
          b. if the target processor is 16 bits, define it as uint16_t 
          c. if the target processor is 32 bits, define it as uint32_t
          d. if the target processor is 64 bits, define it as either uint32_t or 
             uint64_t
 */


#ifdef __cplusplus
extern "C" {
#endif
/*============================ MACROS ========================================*/
#undef dcl_class
#undef declare_class
#undef def_class
#undef define_class
#undef __def_class2
#undef __def_class3
#undef __def_class4
#undef __def_class5
#undef __def_class6
#undef __def_class7
#undef __def_class8
#undef __def_class
#undef end_def_class
#undef end_define_class
#undef __end_def_class
#undef extern_class
#undef __extern_class
#undef end_extern_class
#undef __end_extern_class

/*============================ MACROFIED FUNCTIONS ===========================*/


#if   defined(__PLOOC_CLASS_IMPLEMENT__) || defined(__PLOOC_CLASS_IMPLEMENT)

#   define __def_class2(__name, _1)                                             \
    typedef struct __name __name;                                               \
    typedef struct __##__name __##__name;                                       \
    struct __##__name {                                                         \
        __PLOOC_CONNECT2(__PLOOC_PRI_,_1)                                       \
    };                                                                          \
    struct __name {                                                             \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_1)                                       \
    };        
    
#   define __def_class3(__name, _1, _2)                                         \
    typedef struct __name __name;                                               \
    typedef struct __##__name __##__name;                                       \
    struct __##__name {                                                         \
        __PLOOC_CONNECT2(__PLOOC_PRI_,_1)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRI_,_2)                                       \
    };                                                                          \
    struct __name {                                                             \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_1)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_2)                                       \
    };
    
#   define __def_class4(__name, _1, _2, _3)                                     \
    typedef struct __name __name;                                               \
    typedef struct __##__name __##__name;                                       \
    struct __##__name {                                                         \
        __PLOOC_CONNECT2(__PLOOC_PRI_,_1)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRI_,_2)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRI_,_3)                                       \
    };                                                                          \
    struct __name {                                                             \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_1)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_2)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_3)                                       \
    };
    
#   define __def_class5(__name, _1, _2, _3, _4)                                 \
    typedef struct __name __name;                                               \
    typedef struct __##__name __##__name;                                       \
    struct __##__name {                                                         \
        __PLOOC_CONNECT2(__PLOOC_PRI_,_1)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRI_,_2)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRI_,_3)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRI_,_4)                                       \
    };                                                                          \
    struct __name {                                                             \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_1)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_2)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_3)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_4)                                       \
    };
    
#   define __def_class6(__name, _1, _2, _3, _4, _5)                             \
    typedef struct __name __name;                                               \
    typedef struct __##__name __##__name;                                       \
    struct __##__name {                                                         \
        __PLOOC_CONNECT2(__PLOOC_PRI_,_1)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRI_,_2)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRI_,_3)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRI_,_4)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRI_,_5)                                       \
    };                                                                          \
    struct __name {                                                             \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_1)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_2)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_3)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_4)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_5)                                       \
    };
    
#   define __def_class7(__name, _1, _2, _3, _4, _5, _6)                         \
    typedef struct __name __name;                                               \
    typedef struct __##__name __##__name;                                       \
    struct __##__name {                                                         \
        __PLOOC_CONNECT2(__PLOOC_PRI_,_1)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRI_,_2)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRI_,_3)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRI_,_4)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRI_,_5)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRI_,_6)                                       \
    };                                                                          \
    struct __name {                                                             \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_1)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_2)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_3)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_4)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_5)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_6)                                       \
    };
    
#   define __def_class8(__name, _1, _2, _3, _4, _5, _6, _7)                     \
    typedef struct __name __name;                                               \
    typedef struct __##__name __##__name;                                       \
    struct __##__name {                                                         \
        __PLOOC_CONNECT2(__PLOOC_PRI_,_1)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRI_,_2)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRI_,_3)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRI_,_4)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRI_,_5)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRI_,_6)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRI_,_7)                                       \
    };                                                                          \
    struct __name {                                                             \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_1)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_2)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_3)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_4)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_5)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_6)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_7)                                       \
    };
          
#   define __end_def_class(...)

#   define __extern_class(...) 
#   define __end_extern_class(...)


#   undef  __class
#   define __class(__name)                  __##__name

#   undef  class
#   define class(__name)                    __class(__name)


#   undef __with_class
#   define __with_class(__type, __src, ...)                                     \
        {                                                                       \
            class(__type)*_ =(class(__type) *)(__src);                          \
            PLOOC_UNUSED_PARAM(_);                                              \
            __VA_ARGS__;                                                        \
        }                                                                       \
        for (class(__type)*_ =(class(__type) *)(__src);NULL != _; _ = NULL)

#   undef with_class
#   define with_class(__type, __src, ...)                                       \
            __with_class(__type, __src, __VA_ARGS__)
            
#   undef __class_internal
#   define __class_internal(__src, __des, __type, ...)                          \
            class(__type) *(__des) = (class(__type) *)(__src);                  \
            PLOOC_UNUSED_PARAM(__des);                                          \
            __with_class(__type, (__src), __VA_ARGS__)

          
#   undef class_internal            
#   define class_internal(__src, __des, __type,...)                             \
            __class_internal(__src, __des, __type, __VA_ARGS__)


#elif   defined(__PLOOC_CLASS_INHERIT__) || defined(__PLOOC_CLASS_INHERIT)

#   define __def_class2(__name, _1)                                             \
    typedef struct __name __name;                                               \
    struct __protected_##__name {                                               \
        __PLOOC_CONNECT2(__PLOOC_PRO_,_1)                                       \
    };                                                                          \
    struct __name {                                                             \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_1)                                       \
    };        
    
#   define __def_class3(__name, _1, _2)                                         \
    typedef struct __name __name;                                               \
    struct __protected_##__name {                                               \
        __PLOOC_CONNECT2(__PLOOC_PRO_,_1)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRO_,_2)                                       \
    };                                                                          \
    struct __name {                                                             \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_1)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_2)                                       \
    };
    
#   define __def_class4(__name, _1, _2, _3)                                     \
    typedef struct __name __name;                                               \
    struct __protected_##__name {                                               \
        __PLOOC_CONNECT2(__PLOOC_PRO_,_1)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRO_,_2)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRO_,_3)                                       \
    };                                                                          \
    struct __name {                                                             \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_1)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_2)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_3)                                       \
    };
    
#   define __def_class5(__name, _1, _2, _3, _4)                                 \
    typedef struct __name __name;                                               \
    struct __protected_##__name {                                               \
        __PLOOC_CONNECT2(__PLOOC_PRO_,_1)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRO_,_2)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRO_,_3)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRO_,_4)                                       \
    };                                                                          \
    struct __name {                                                             \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_1)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_2)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_3)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_4)                                       \
    };
    
#   define __def_class6(__name, _1, _2, _3, _4, _5)                             \
    typedef struct __name __name;                                               \
    struct __protected_##__name {                                               \
        __PLOOC_CONNECT2(__PLOOC_PRO_,_1)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRO_,_2)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRO_,_3)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRO_,_4)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRO_,_5)                                       \
    };                                                                          \
    struct __name {                                                             \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_1)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_2)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_3)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_4)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_5)                                       \
    };
    
#   define __def_class7(__name, _1, _2, _3, _4, _5, _6)                         \
    typedef struct __name __name;                                               \
    struct __protected_##__name {                                               \
        __PLOOC_CONNECT2(__PLOOC_PRO_,_1)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRO_,_2)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRO_,_3)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRO_,_4)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRO_,_5)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRO_,_6)                                       \
    };                                                                          \
    struct __name {                                                             \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_1)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_2)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_3)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_4)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_5)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_6)                                       \
    };
    
#   define __def_class8(__name, _1, _2, _3, _4, _5, _6, _7)                     \
    typedef struct __name __name;                                               \
    struct __protected_##__name {                                               \
        __PLOOC_CONNECT2(__PLOOC_PRO_,_1)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRO_,_2)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRO_,_3)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRO_,_4)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRO_,_5)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRO_,_6)                                       \
        __PLOOC_CONNECT2(__PLOOC_PRO_,_7)                                       \
    };                                                                          \
    struct __name {                                                             \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_1)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_2)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_3)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_4)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_5)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_6)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_7)                                       \
    };



#   define __end_def_class(...)  

#   define __extern_class(...)                                    

#   define __end_extern_class(...)

#   undef  __class_protected
#   define __class_protected(__name)            struct __protected_##__name

#   undef  class_protected
#   define class_protected(__name)              __class_protected(__name)


#   undef __with_protected
#   define __with_protected(__type, __src, ...)                                 \
        {                                                                       \
            class_protected(__type)*_ =(class_protected(__type) *)(__src);      \
            PLOOC_UNUSED_PARAM(_);                                              \
            __VA_ARGS__;                                                        \
        }                                                                       \
        for (   class_protected(__type)*_ =(class_protected(__type) *)(__src);  \
                NULL != _;                                                      \
                _ = NULL)

#   undef with_protected
#   define with_protected(__type, __src, ...)                                   \
            __with_protected(__type, __src, __VA_ARGS__)       

#   undef __protected_internal
#   define __protected_internal(__src, __des, __type, ...)                      \
            class_protected(__type) *(__des)=(class_protected(__type) *)(__src);\
            PLOOC_UNUSED_PARAM(__des);                                          \
            __with_protected(__type, __src, __VA_ARGS__)

#   undef protected_internal            
#   define protected_internal(__src, __des, __type, ...)                        \
            __protected_internal(__src, __des, __type, __VA_ARGS__) 
            

#else  /* __PLOOC_CLASS_EXTERN */

#       define __def_class2(__name, _1)                                         \
    typedef struct __name __name;                                               \
    struct __name {                                                             \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_1)                                       \
    };        
    
#       define __def_class3(__name, _1, _2)                                     \
    typedef struct __name __name;                                               \
    struct __name {                                                             \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_1)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_2)                                       \
    };
    
#       define __def_class4(__name, _1, _2, _3)                                 \
    typedef struct __name __name;                                               \
    struct __name {                                                             \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_1)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_2)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_3)                                       \
    };
    
#       define __def_class5(__name, _1, _2, _3, _4)                             \
    typedef struct __name __name;                                               \
    struct __name {                                                             \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_1)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_2)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_3)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_4)                                       \
    };
    
#       define __def_class6(__name, _1, _2, _3, _4, _5)                         \
    typedef struct __name __name;                                               \
    struct __name {                                                             \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_1)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_2)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_3)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_4)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_5)                                       \
    };
    
#       define __def_class7(__name, _1, _2, _3, _4, _5, _6)                     \
    typedef struct __name __name;                                               \
    struct __name {                                                             \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_1)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_2)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_3)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_4)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_5)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_6)                                       \
    };
    
#       define __def_class8(__name, _1, _2, _3, _4, _5, _6, _7)                 \
    typedef struct __name __name;                                               \
    struct __name {                                                             \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_1)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_2)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_3)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_4)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_5)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_6)                                       \
        __PLOOC_CONNECT2(__PLOOC_EXT_,_7)                                       \
    };

#   define __end_def_class(...) 

#define __extern_class(...)             __PLOOC_EVAL(__def_class, __VA_ARGS__)  \
                                                    (__VA_ARGS__)

#define __end_extern_class(...)
       
#endif 

#define def_class(...)                  __PLOOC_EVAL(__def_class, __VA_ARGS__)  \
                                                    (__VA_ARGS__)
#define define_class(...)               def_class(__VA_ARGS__)

#define end_def_class(...)              __end_def_class(__VA_ARGS__)
#define end_define_class(...)           end_def_class(__VA_ARGS__)

#define declare_class(__name)           typedef struct __name __name;    
#define dcl_class(__name)               declare_class(__name)

#define extern_class(...)               __extern_class(__VA_ARGS__)
#define end_extern_class(...)           __end_extern_class(__VA_ARGS__)


#undef __PLOOC_CLASS_IMPLEMENT__
#undef __PLOOC_CLASS_INHERIT__
#undef __PLOOC_CLASS_IMPLEMENT
#undef __PLOOC_CLASS_INHERIT
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

//#endif                                /* deliberately comment this out! */

#ifdef __cplusplus
}
#endif