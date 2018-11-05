#include "tlv.h"
#include <string.h>


/********** PRIVATE DECLARATIONS **********************************************/
/**
 * @brief Creates a new tlv object
 * Allocates memory for the struct
 * @return Pointer to the struct or NULL if allocation failed
 */
static tlv_t* tlv_new();


/**
 * @brief Resets a tlv object
 * All members will be set to defaults like NULL and 0
 * @param tlv Tlv object to reset
 */
static tlv_t* tlv_reset(tlv_t *tlv);

/**
 * @brief Sets the sibling of this object
 * @param tlv Tlv object to set the sibling element on
 * @param next TLV object to be set as sibling
 * @return the sibling or NULL
 */
//static tlv_t* tlv_set_next(tlv_t *tlv, tlv_t *next);


/**
 * @brief Sets the child of this tag
 * @param tlv Tlv object to set the child element on
 * @param child TLV object to be set as child
 * @return Returns the child element or NULL
 */
static tlv_t* tlv_set_child(tlv_t *tlv, tlv_t *child);

/********** PUBLIC DEFINITIONS ************************************************/
tlv_t* tlv_new_cdo(const tlv_tag_t tag) {
    tlv_t *tlv;

    if ((tlv = tlv_new()) == NULL) {
        return NULL;
    }
    tlv->type = TLV_CDO;
    tlv->tag = tag;

    return tlv;
}

tlv_t*tlv_new_pdo(const tlv_tag_t tag, const tlv_length_t length, uint8_t *value) {
    tlv_t*tlv;

    if ((tlv = tlv_new()) == NULL) {
        return NULL;
    }

    tlv->type = TLV_PDO;
    tlv->tag = tag;
    tlv->length = length;
    tlv->value = value;

    return tlv;
}

tlv_t* tlv_append_next(tlv_t *tlv, tlv_t *next) {
    if (tlv && next) {
        tlv_t *tmp = tlv;

        while (tmp->next != NULL) {
            //find the last "next" element
            tmp = tmp->next;
        }
        tmp->next = next;
#ifdef TLV_DEBUG
        next->level = tmp->level;
#endif
        return next;
    }

    return NULL;
}

tlv_t* tlv_append_child(tlv_t *tlv, tlv_t*child) {
    if (tlv && child) {
        if (tlv->type != TLV_CDO) {
            return NULL;
        }

        if (tlv->child == NULL) {
            tlv_set_child(tlv, child);
        } else {
            tlv_append_next(tlv->child, child);
        }

        return child;
    }

    return NULL;
}

const tlv_t* tlv_find_by_tag(const tlv_t *tlv, const tlv_tag_t tag) {
    const tlv_t *out_tlv = NULL;
    if (tlv) {
        if (tlv->tag == tag) {
            out_tlv = tlv;
        } else {
            if ((out_tlv = tlv_find_by_tag(tlv->next, tag)) == NULL) {
                out_tlv = tlv_find_by_tag(tlv->child, tag);
            }
        }
    }

    return out_tlv;
}

void tlv_delete(tlv_t **tlv) {
    if (tlv != NULL && *tlv != NULL) {
        tlv_delete_all(&(*tlv)->next);
        tlv_delete_all(&(*tlv)->child);
        (*tlv)->value = NULL;
        (*tlv)->length = 0;
        free(*tlv);

        *tlv = NULL;
    }
}

void tlv_delete_all(tlv_t **tlv) {
    if (tlv != NULL && *tlv != NULL) {
        tlv_delete_all(&(*tlv)->next);
        tlv_delete_all(&(*tlv)->child);
        if ((*tlv)->value != NULL) {
            free((*tlv)->value);
            (*tlv)->value = NULL;
            (*tlv)->length = 0;
        }

        free(*tlv);
        *tlv = NULL;
    }
}

/********** PRIVATE DEFINITIONS ***********************************************/
static tlv_t* tlv_new() {
    tlv_t *tlv;

    tlv = (tlv_t*) malloc(sizeof (*tlv));
    if (tlv == NULL) {

        return NULL;
    }

    return tlv_reset(tlv);
}

/**
 * @brief Resets a tlv object
 * All members will be set to defaults like NULL and 0
 * @param tlv Tlv object to reset
 */
static tlv_t* tlv_reset(tlv_t *tlv) {
    tlv->child = NULL;
    tlv->next = NULL;
    tlv->length = 0;
    tlv->tag = 0;
    tlv->value = NULL;
    tlv->type = TLV_NOT_SET;
#ifdef TLV_DEBUG
    tlv->level = 0;
#endif

    return tlv;
}

/*
static tlv_t* tlv_set_next(tlv_t *tlv, tlv_t *next)
{
    if (tlv && next) {
        if(tlv->next != NULL){
            tlv_delete(&tlv->next);
        }
        tlv->next = next;
#ifdef TLV_DEBUG
        next->level = tlv->level;
#endif
        return next;
    }
    
    return NULL;
}
 */

static tlv_t* tlv_set_child(tlv_t *tlv, tlv_t *child) {
    if (tlv && child) {
        if (tlv->type != TLV_CDO) {
            return NULL;
        }
        if (tlv->child != NULL) {
            tlv_delete(&tlv->child);
        }
        tlv->child = child;
#ifdef TLV_DEBUG
        child->level = tlv->level + 1;
#endif
        return child;
    }

    return NULL;
}
