/* 
 * File:   TLV.h
 * Author: Bela Bursan
 *
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
    
    enum TLV_TYPE
    {
        TLV_CDO = 0,
        TLV_PDO = 1
    };

    struct TLV
    {
        struct TLV *next;
        struct TLV *child;
        uint8_t *value;
        int32_t tag;
        uint32_t length;
        uint8_t isCdo;
        uint8_t level;
    };

    void tlv_version();
    void tlv_free(struct TLV **tlv);
    struct TLV *tlv_new_cdo(const uint32_t tag);
    struct TLV *tlv_new_pdo(const uint32_t tag, const uint32_t length, uint8_t *value);
    struct TLV *tlv_append_next(struct TLV *self, struct TLV *next);
    struct TLV *tlv_append_child(struct TLV *self, struct TLV *child);
    struct TLV *tlv_set_next(struct TLV *self, struct TLV *next);
    struct TLV *tlv_set_child(struct TLV *self, struct TLV *child);
    struct TLV *tlv_find_by_tag(const struct TLV *self, const int32_t tag);
    




#ifdef __cplusplus
}
#endif

#endif /* TLV_H */
