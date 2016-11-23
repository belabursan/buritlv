#include "TLV.h"
#include "dbg.h"
#include <string.h>
#include <stdio.h>

static struct TLV* tlv_new();
static void tlv_reset(struct TLV *tlv);


/////////////////////////////////////////////////////////////////////////////

/**
 * @brief Creates a new struct TLV object
 * Allocates memory for the struct
 * @return pointer to the struct or NULL if allocation failed
 */
static struct TLV* tlv_new()
{
    struct TLV *tlv;

    tlv = (struct TLV*) malloc(sizeof (struct TLV));
    if (tlv == NULL) {
        error("tlv_new: Failed to allocate memory for the TLV object");
        sysl(LOG_CRIT, "tlv_new: ERROR - Failed to allocate memory for TLV");

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
    tlv->isCdo = 0;
    tlv->tag = 0;
    tlv->value = NULL;
    tlv->level = 0;
}

/**
 * @brief Prints the library version to the standard out
 */
void tlv_version()
{
    printf("TLV version: v%s\n", TLV_VERSION);
}

/**
 * @brief Deletes a TLV struct
 * Also deletes the value, child and next structs recursively
 * @param tlv struct to delete
 */
void tlv_free(struct TLV **tlv)
{
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
 * @return the tlv object or NULL
 */
struct TLV *tlv_new_cdo(const uint32_t tag)
{
    struct TLV *tlv;
    
    if ((tlv = tlv_new()) == NULL) {
        error("tlv_new_cdo: ERROR - Failed to create CDO");
        return NULL;
    }
    tlv->isCdo = TLV_CDO;
    tlv->tag = tag;

    return tlv;
}

/**
 * @brief Creates a new TLV object of PDO type
 * @param tag the tag of the object
 * @param length the length of the data
 * @param value pointer data to set
 * @return the tlv object or NULL
 */
struct TLV *tlv_new_pdo(const uint32_t tag, const uint32_t length, uint8_t *value)
{
    struct TLV *tlv;
    
    if ((tlv = tlv_new()) == NULL) {
        error("tlv_new_pdo: ERROR - Failed to create for PDO");
        return NULL;
    }

    tlv->isCdo = TLV_PDO;
    tlv->tag = tag;
    tlv->length = length;
    tlv->value = value;

    return tlv;
}

/**
 * @brief Appends a TLV object to the end of "next" chain
 * @param self tlv object to append the next element on
 * @param next tlv object to append
 * @return pointer to "next" or NULL if next or self is null
 */
struct TLV* tlv_append_next(struct TLV *self, struct TLV *next)
{
    if (self && next) {
        struct TLV *tmp = self;

        while (tmp->next != NULL) {
            //find the last "next" element
            tmp = tmp->next;
        }
        tmp->next = next;
        next->level = tmp->level;
        return next;
    }

    dbg("tlv_append_next: self or next is null");
    return NULL;
}

/**
 * @brief Appends a TLV object to the end of "child" chain
 * @param self tlv object to append the child element on
 * @param child tlv object to append
 * @return pointer to "child" or NULL if next or self is null or self isn't CDO
 */
struct TLV* tlv_append_child(struct TLV *self, struct TLV *child)
{
    if (self && child) {
        if (self->isCdo != TLV_CDO) {
            dbg("tlv_append_child: not possible to append child to a PDO");
            return NULL;
        }

        if (self->child == NULL) {
            tlv_set_child(self, child);
        } else {
            tlv_append_next(self->child, child);
        }

        return child;
    }

    dbg("tlv_append_child: self or child is null");
    return NULL;
}

/**
 * @brief Sets the sibling of this object
 * @param self tlv object to set the sibling element on
 * @param next TLV object to be set as sibling
 * @return the sibling or NULL
 */
struct TLV *tlv_set_next(struct TLV *self, struct TLV *next)
{
    if (self && next) {
        if(self->next != NULL){
            tlv_free(&self->next);
        }
        next->level = self->level;
        self->next = next;
        
        return next;
    }
    
    dbg("tlv_set_next: self or next is null");
    return NULL;
}

/**
 * @brief Sets the child of this tag
 * @param self tlv object to set the child element on
 * @param child TLV object to be set as child
 * @return returns the child element or NULL
 */
struct TLV *tlv_set_child(struct TLV *self, struct TLV *child)
{
    if (self && child) {
        if (self->isCdo != TLV_CDO) {
            dbg("tlv_set_child: not possible to append child to a PDO");
            return NULL;
        }
        if (self->child != NULL) {
            tlv_free(&self->child);
        }
        self->child = child;
        child->level = self->level + 1;

        return child;
    }
    
    dbg("tlv_set_child: self or child is null");
    return NULL;
}

/**
 * @brief Finds a TLV object with the specified tag
 * @param self the TLV object to search recursively
 * @param tag tag of the TLV to search for
 * @return the requested TLV or NULL if not found 
 */
struct TLV *tlv_find_by_tag(const struct TLV *self, const int32_t tag)
{
    struct TLV *tlv = NULL;
    if(self){
        if(self->tag == tag){
            tlv = (struct TLV*)self;
        } else {        
            if((tlv = tlv_find_by_tag(self->next, tag)) == NULL){
                tlv = tlv_find_by_tag(self->child, tag);
            }
        }
    }
    
    return tlv;
}
