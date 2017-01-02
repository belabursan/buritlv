#include "TLV.h"
#include "dbg.h"
#include <string.h>
#include <stdio.h>
#include <math.h>


static struct TLV* tlv_new(struct TlvError **err);
static void tlv_reset(struct TLV *tlv);
static int tlv_as_bytes(struct TLV *self, uint8_t **bytes, uint32_t *length, TlvError **err);
static int tlv_set_error(struct TlvError **err, enum TlvErrorType error_type, char const *txt);

/////////////////////////////////////////////////////////////////////////////

/**
 * @brief Creates a new struct TLV object
 * Allocates memory for the struct
 * @param err error to set in case of failure
 * @return pointer to the struct or NULL if allocation failed
 */
static struct TLV* tlv_new(struct TlvError **err) {
    struct TLV *tlv;

    tlv = (struct TLV*) malloc(sizeof (struct TLV));
    if (tlv == NULL) {
        tlv_set_error(err, TLV_MALLOC_FAILED, "Failed to allocate memory for the TLV object");
        return NULL;
    }

    tlv_reset(tlv);

    return tlv;
}

/**
 * @brief Resets a struct TLV
 * All members will be set to defaults like NULL and 0
 * @param tlv struct TLV to reset
 */
static void tlv_reset(struct TLV *tlv)
{
    tlv->child = NULL;
    tlv->next = NULL;
    tlv->length = 0;
    tlv->tlvform = 0;
    tlv->tag = 0;
    tlv->value = NULL;
    tlv->level = 0;
}

/**
 * Populates the TlvError struct
 * @param err TlvError struct to populate
 * @param error_type error number to set
 * @param txt text to set 
 * @return 0 if succeeded, -1 in case of failure 
 */
static int tlv_set_error(struct TlvError **err, enum TlvErrorType error_type, char const *txt)
{
    if (err) {
        *err = (struct TlvError*) malloc(sizeof (struct TlvError));
        if (*err == NULL) {
            error("tlv_set_error: could not allocate memory for struct TlvError");
            return -1;
        }

        (*err)->error_type = error_type;
        memset((*err)->message, 0, sizeof ((*err)->message));
        snprintf((*err)->message, sizeof ((*err)->message), "%s", txt);
    }
    return 0;
}

/**
 * @brief Prints the library version to the standard out
 */
void tlv_version() {
    printf("TLV version: v%s\n", TLV_VERSION);
}

/**
 * @brief Deletes a TLV struct
 * Also deletes the value, child and next structs recursively
 * @param tlv struct to delete
 */
void tlv_free(struct TLV **tlv) {
    if (tlv != NULL && *tlv != NULL) {
        dbg("tlv_free: deleting tlv with tag %u\n", (*tlv)->tag);
        tlv_free(&(*tlv)->next);
        tlv_free(&(*tlv)->child);
        if ((*tlv)->value != NULL) {
#ifdef TLV_SECURE
            memset((*tlv)->value, 0, (*tlv)->length);
#endif
            free((*tlv)->value);
            (*tlv)->value = NULL;
            (*tlv)->length = 0;
        }
#ifdef TLV_SECURE
        memset(*tlv, 0, sizeof (struct TLV));
#endif
        free(*tlv);
        *tlv = NULL;
        tlv = NULL;
    }
}

/**
 * @brief Creates a new TLV object of CDO type
 * @param tag the tag of the tlv object
 * @param err error to set in case of failure
 * @return the tlv object or NULL
 */
struct TLV *tlv_new_cdo(const uint32_t tag, struct TlvError **err) {
    struct TLV *tlv;

    if ((tlv = tlv_new(err)) == NULL) {
        dbg("tlv_new_cdo: ERROR - Failed to create CDO");
        return NULL;
    }
    tlv->tlvform = TLV_CDO;
    tlv->tag = tag;

    return tlv;
}

/**
 * @brief Creates a new TLV object of PDO type
 * @param tag the tag of the object
 * @param length the length of the data(value)
 * @param value pointer to the data(value) to set
 * @param err error to set in case of failure
 * @return the tlv object or NULL in case of error
 */
struct TLV *tlv_new_pdo(const uint32_t tag, const uint32_t length, uint8_t *value, TlvError **err) {
    struct TLV *tlv;

    if ((tlv = tlv_new(err)) == NULL) {
        dbg("tlv_new_pdo: ERROR - Failed to create for PDO");
        return NULL;
    }

    tlv->tlvform = TLV_PDO;
    tlv->tag = tag;
    tlv->length = length;
    tlv->value = value;

    return tlv;
}

/**
 * @brief Appends a TLV object to the end of "next" chain
 * @param self tlv object to append the next element on
 * @param next tlv object to append
 * @param err error to set in case of failure
 * @return pointer to "next" or NULL if next or self is null
 */
struct TLV* tlv_append_next(struct TLV *self, struct TLV *next, struct TlvError **err) {
    struct TLV *tmp = self;
    dbg("tlv_append_next() - start");

    if (self == NULL) {
        tlv_set_error(err, TLV_NULL, "self is null");
        dbg("failed to append next, self is null");
        return NULL;
    }

    if (next == NULL) {
        tlv_set_error(err, TLV_NULL, "next is null");
        dbg("failed to append next, next is null");
        return NULL;
    }

    while (tmp->next != NULL) {
        //find the last "next" element
        tmp = tmp->next;
    }
    tmp->next = next;
    next->level = tmp->level;

    return next;
}

/**
 * @brief Appends a TLV object to the end of "child" chain
 * @param self tlv object to append the child element on
 * @param child tlv object to append
 * @param err error to set in case of failure
 * @return pointer to "child" or NULL if next or self is null or self isn't CDO
 */
struct TLV* tlv_append_child(struct TLV *self, struct TLV *child, struct TlvError **err) {
    dbg("tlv_append_child() - start");

    if (self == NULL) {
        tlv_set_error(err, TLV_NULL, "self is null");
        dbg("failed to append child, self is null");
        return NULL;
    }

    if (child == NULL) {
        tlv_set_error(err, TLV_NULL, "child is null");
        dbg("failed to append child, child is null");
        return NULL;
    }

    if (self->tlvform != TLV_CDO) {
        tlv_set_error(err, TLV_NOT_CDO, "not possible to append child to a PDO");
        dbg("failed to append child, self is not CDO");
        return NULL;
    }

    if (self->child == NULL) {
        tlv_set_child(self, child, err);
    } else {
        tlv_append_next(self->child, child, err);
    }

    return child;
}

/**
 * @brief Sets the sibling of this object
 * @param self tlv object to set the sibling element on
 * @param next TLV object to be set as sibling
 * @param err error to set in case of failure
 * @return the sibling or NULL
 */
struct TLV *tlv_set_next(struct TLV *self, struct TLV *next, struct TlvError **err) {
    dbg("tlv_set_next() - start");
    if (self == NULL) {
        tlv_set_error(err, TLV_NULL, "self is null");
        dbg("failed to set next, self is null");
        return NULL;
    }

    if (next == NULL) {
        tlv_set_error(err, TLV_NULL, "next is null");
        dbg("failed to set next, next is null");
        return NULL;
    }
    
    if (self->next != NULL) {
        tlv_free(&self->next);
    }
    next->level = self->level;
    self->next = next;

    return next;
}

/**
 * @brief Sets the child of this tag
 * @param self tlv object to set the child element on
 * @param child TLV object to be set as child
 * @param err error to set in case of failure
 * @return returns the child element or NULL
 */
struct TLV *tlv_set_child(struct TLV *self, struct TLV *child, struct TlvError **err) {
    dbg("tlv_set_child() - start");

    if (self == NULL) {
        tlv_set_error(err, TLV_NULL, "self is null");
        dbg("failed to set child, self is null");
        return NULL;
    }

    if (child == NULL) {
        tlv_set_error(err, TLV_NULL, "child is null");
        dbg("failed to set child, child is null");
        return NULL;
    }

    if (self->tlvform != TLV_CDO) {
        tlv_set_error(err, TLV_NOT_CDO, "not possible to append child to a PDO");
        dbg("failed to set child, self is not CDO");
        return NULL;
    }
    
    if (self->child != NULL) {
        tlv_free(&self->child);
    }
    self->child = child;
    child->level = self->level + 1;

    return child;
}

/**
 * @brief Finds a TLV object with the specified tag
 * @param self the TLV object to search recursively
 * @param tag tag of the TLV to search for
 * @return the requested TLV or NULL if not found 
 */
struct TLV *tlv_find_by_tag(const struct TLV *self, const int32_t tag) {
    struct TLV *tlv = NULL;
    if (self) {
        if (self->tag == tag) {
            tlv = (struct TLV*) self;
        } else {
            if ((tlv = tlv_find_by_tag(self->next, tag)) == NULL) {
                tlv = tlv_find_by_tag(self->child, tag);
            }
        }
    }

    return tlv;
}

/**
 * @brief Converts the TLV object and all it's child to an byte array
 * @param self [in] TLV object to convert
 * @param bytes [out] byte array representing the TLV object
 * @param length [out] the length of the byte array
 * @param err error to set in case of failure
 * @return 0 if successful, negative number otherwise
 */
int tlv_to_byte_array(struct TLV *self, uint8_t **bytes, uint32_t *length, TlvError **err) {
    struct TLV *tmp = self->child;
    if (tmp) {
        //...
        tmp = tmp->next;
    }

    return tlv_as_bytes(self, bytes, length, err);
}

/**
 * 
 * @param self TLV object to convert to byte array
 * @param bytes [out] byte array representing the TLV object 
 * @param length [out] the length of the byte array
 * @param err error to set in case of failure
 * @return 0 if succeeded, -1 in case of error
 */
static int tlv_as_bytes(struct TLV *self, uint8_t **bytes, uint32_t *length, TlvError **err) {
    uint8_t noofTagBytes;
    uint8_t noofLenBytes;
    uint8_t firstByte = 0;

    firstByte = self->tlvclass & 0xC0; //set class
    firstByte |= self->tlvform & 0x20;

    if (self->tag < 31) {
        // tag lower then 31, there is place in the first byte, set it
        firstByte |= (uint8_t) (self->tag & 0x1F);
        noofTagBytes = 0;
    } else {
        firstByte |= 0x1F;
        noofTagBytes = (int) floor((log((double) self->tag) / log(2)) / 7) + 1;
    }
    //...


    return -1;
}