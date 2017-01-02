/* 
 * File:   TLV.h
 * @brief TLV class, represents a tag-length-value objects
 * @author <a href="mailto:bursan@gmail.com">Bela Bursan</a>
 * Created on October 12, 2016, 9:23 AM
 */

#ifndef TLV_H
#define TLV_H

//#define TLV_SECURE 1
#define TLV_MAJOR "0"
#define TLV_MINOR "0"
#define TLV_BUILD "1"    
#define TLV_VERSION  TLV_MAJOR "." TLV_MINOR "." TLV_BUILD

#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <inttypes.h>

    
    enum TlvClass
    {
        CLASS_UNIVERSAL     = 0x00,
        CLASS_APPLICATION   = 0x40,
        CLASS_CONTEXT_SPEC  = 0x80,
        CLASS_PRIVATE       = 0xC0
    };
    
    enum TlvForm
    {
        TLV_PDO = 0x00,
        TLV_CDO = 0x20
    };
    
    enum TlvErrorType
    {
        TLV_NULL,
        TLV_NOT_CDO,
        TLV_NOT_PDO,
        TLV_MALLOC_FAILED
    };
    
    struct TlvError
    {
        TlvErrorType error_type;
        char message[128];
    };

    struct TLV
    {
        struct TLV *next;   // next element on the same level, 
        struct TLV *child;  // child element, the first if many
        int32_t tag;        // tag
        uint32_t length;    // length of the value(in case of pdo, cdo has no data)
        uint8_t *value;     // data in pdo, null for cdo but can hold child elements when converting to byte array
        uint8_t tlvclass;   // the class of the tlv
        uint8_t tlvform;    // boolean indicating if cdo  
        uint8_t level;      // only used for printing the tlv object
    };

    void tlv_version();
    void tlv_free(struct TLV **tlv);
    struct TLV *tlv_new_cdo(const uint32_t tag, struct TlvError **err);
    struct TLV *tlv_new_pdo(const uint32_t tag, const uint32_t length, uint8_t *value, struct TlvError **err);
    struct TLV *tlv_append_next(struct TLV *self, struct TLV *next, struct TlvError **err);
    struct TLV *tlv_append_child(struct TLV *self, struct TLV *child, struct TlvError **err);
    struct TLV *tlv_set_next(struct TLV *self, struct TLV *next, struct TlvError **err);
    struct TLV *tlv_set_child(struct TLV *self, struct TLV *child, struct TlvError **err);
    struct TLV *tlv_find_by_tag(const struct TLV *self, const int32_t tag);
    //
    int tlv_to_byte_array(struct TLV *self, uint8_t **bytes, uint32_t *length, TlvError **err);
    




#ifdef __cplusplus
}
#endif

//g++ -m64   -c -O3 -Werror -s -fPIC  -MMD -MP -MF "build/Release/GNU-Linux/TLV.o.d" -o build/Release/GNU-Linux/TLV.o TLV.c
//g++ -m64   -c -O3 -Werror -s -fPIC  -MMD -MP -MF "build/Release/GNU-Linux/dbg.o.d" -o build/Release/GNU-Linux/dbg.o dbg.c
//g++ -m64    -o dist/Release/GNU-Linux/libctlv.so build/Release/GNU-Linux/TLV.o build/Release/GNU-Linux/dbg.o  -shared -fPIC

#endif /* TLV_H */
