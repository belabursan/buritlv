#include "tlv.h"
#include <stdio.h>
#include <string.h>
#ifdef TLV_DEBUG
#include <stdarg.h>
#endif


/********** PRIVATE DECLARATIONS **********************************************/
/**
 * @brief Creates a new tlv object
 * Allocates memory for the tlv struct
 * @return Pointer to the struct or NULL if allocation failed
 */
static tlv_t* tlv_new(void);


/**
 * @brief Resets a tlv object
 * All members will be set to defaults like NULL and 0
 * @param[in] tlv Tlv object to reset
 */
static tlv_t* tlv_reset(tlv_t *tlv);


/**
 * @brief Sets the child of this tag
 * @param[in] tlv Tlv object to set the child element on
 * @param[in] child TLV object to be set as child
 * @return Returns the child element or NULL
 */
static tlv_t* tlv_set_child(tlv_t *tlv, tlv_t *child);


/**
 * @brief Returns the total length of the specified tlv object
 * @param[in] tlv Tlv object to get the length for
 * @param[out] length The length of the tlv object in bytes
 * @return True if the function succeeded, false otherwise
 */
static bool get_total_length(const tlv_t *tlv, size_t *length);


/**
 * @brief Saves a Tlv header in an array
 * @param[in] tlv Tlv to save the header for
 * @param[in,out] array Array to save the header in
 * @param[in] index Pointer to the index of the array
 * @param[in] length Total length of the array
 * @return True if setting header succeeded, false otherwise
 */
static bool set_header(const tlv_t *tlv, uint8_t *array, uint32_t *index, const size_t length);


/**
 * @brief Converts a Tlv to a byte array
 * @param[in] tlv Tlv to convert
 * @param[in,out] array Return point of the array
 * @param[in] index Pointer to the index of the array
 * @param[in] length Total length of the array
 * @return True if conversion succeeded, false otherwise
 */
static bool to_byte_array(const tlv_t *tlv, uint8_t **array, uint32_t *index, const size_t length);

#ifdef TLV_DEBUG
/**
 * @brief Returns the string representation of a tlv object
 *
 * @param[in] tlv TLV object to represent as string
 * @param[out] str Pointer to string to be filled with the representation of the tlv object
 * @param[in] size Length of the string
 * @param[in] level Corresponds to the level of the TLV object in the thread structure
 * @return ++++++++++++True or False in case of the string is truncated by "snprintf()"
 */
static bool to_string(const tlv_t *tlv, char *str, size_t size, tlv_level_t level);

#endif


/********** PUBLIC DEFINITIONS ************************************************/
tlv_t* tlv_new_cdo(const tlv_tag_t tag) {
    tlv_t *tlv;

    if ((tlv = tlv_new()) == NULL) {
#ifdef TLV_DEBUG
        tlv_debug_cb("Error - Failed to create CDO");
#endif
        return NULL;
    }
    tlv->type = TLV_CDO;
    tlv->tag = tag;

    return tlv;
}


tlv_t*tlv_new_pdo(const tlv_tag_t tag, const tlv_length_t length, uint8_t *value) {
    tlv_t*tlv;

    if ((tlv = tlv_new()) == NULL) {
#ifdef TLV_DEBUG
        tlv_debug_cb("Error - Failed to create PDO");
#endif
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
#ifdef TLV_DEBUG
            tlv_debug_cb("Error - Cannot append child. Tlv is not CDO");
#endif
            return NULL;
        }

        if (tlv->child == NULL) {
            tlv_set_child(tlv, child);
        } else {
            tlv_append_next(tlv->child, child);
        }

        return child;
    }
#ifdef TLV_DEBUG
    tlv_debug_cb("Error - Append failed, argument is null");
#endif
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
        tlv_delete(&(*tlv)->next);
        tlv_delete(&(*tlv)->child);
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


bool tlv_to_byte_array(tlv_t *tlv, uint8_t **barray, size_t *size) {
    size_t length = 0;
    if (barray != NULL && size != NULL) {
        if (get_total_length(tlv, &length)) {
            printf("Length: %u\n", (unsigned) length);
            uint8_t *array = malloc(length);

            if (array != NULL) {
                memset(array, 0, length);
                uint32_t index = 0;
                to_byte_array(tlv, &array, &index, length);
                *barray = array;
                *size = length;
                return true;
            }
#ifdef TLV_DEBUG            
            tlv_debug_cb("Fatal - Failed to allocate memory for array");
#endif
            return false;
        }
#ifdef TLV_DEBUG
        tlv_debug_cb("Error - Failed to get size of the object");
#endif
        return false;
    }

    return false;
}


#ifdef TLV_DEBUG


const char* tlv_to_string(const tlv_t *tlv) {
    if (tlv != NULL) {
        size_t tlv_length = 0;
        if (!get_total_length(tlv, &tlv_length)) {
            tlv_debug_cb("Error - Got wrong tlv length(%u) when printing object", (unsigned) tlv_length);
            return NULL;
        }

        size_t total_length = tlv_length * 5;
        char *instr = malloc(total_length);
        memset(instr, 0, total_length);
        if (!to_string(tlv, instr, total_length, 0)) {
            tlv_debug_cb("Warning - The string may not be complete");
        }

        return instr;
    }

    tlv_debug_cb("Error - Tlv is null when trying to convert it to string");
    return NULL;
}


void __attribute__ ((weak)) tlv_debug_cb(const char *txt, ...) {
    va_list args;
    va_start(args, txt);
    vprintf(txt, args);
    printf("\n");
    va_end(args);
}

#endif


/********** PRIVATE DEFINITIONS ***********************************************/
static tlv_t* tlv_new(void) {
    tlv_t *tlv;

    tlv = (tlv_t*) malloc(sizeof (*tlv));
    if (tlv == NULL) {
#ifdef TLV_DEBUG
        tlv_debug_cb("Fatal - Out of memory");
#endif
        return NULL;
    }

    return tlv_reset(tlv);
}


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


static tlv_t* tlv_set_child(tlv_t *tlv, tlv_t *child) {
    if (tlv && child) {
        if (tlv->type != TLV_CDO) {
#ifdef TLV_DEBUG
            tlv_debug_cb("Error - Set_child failed. Tlv is not CDO");
#endif            
            return NULL;
        }
        if (tlv->child != NULL) {
#ifdef TLV_DEBUG
            tlv_debug_cb("Warning - Deleting already existing child");
#endif            
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


static bool get_total_length(const tlv_t *tlv, size_t *length) {
    size_t buffer_length = BER_HEADER_BYTE_LENGTH;
    size_t tmp_len = 0;

    if (tlv != NULL) {
        if (tlv->type == TLV_PDO) {
            buffer_length += tlv->length; // value length + header length
        }
        if (tlv->child != NULL) {
            get_total_length(tlv->child, &tmp_len);
            buffer_length += tmp_len;
            ((tlv_t*) tlv)->length = tmp_len;
        }

        if (tlv->next != NULL) {
            get_total_length(tlv->next, &tmp_len);
            buffer_length += tmp_len;
        }

        *length = buffer_length;
        return true;
    }
    return false;
}


static bool set_header(const tlv_t *tlv, uint8_t *array, uint32_t *index, const size_t length) {
    if (*index + BER_HEADER_BYTE_LENGTH > length) {
#ifdef TLV_DEBUG
        tlv_debug_cb("Error - not enough room for tlv header");
#endif
        return false;
    }

    array[(*index)++] = tlv->type;
    array[(*index)++] = (uint8_t) (tlv->tag >> 8);
    array[(*index)++] = (uint8_t) (tlv->tag & 0x00ff);
    array[(*index)++] = (uint8_t) (tlv->length >> 8);
    array[(*index)++] = (uint8_t) (tlv->length & 0x00ff);

    return true;
}


static bool to_byte_array(const tlv_t *tlv, uint8_t **array, uint32_t *index, const size_t length) {
    if (set_header(tlv, *array, index, length)) {
        uint16_t len = BER_HEADER_BYTE_LENGTH;
        if (tlv->type == TLV_CDO) {
            if (tlv->child != NULL) {
                if (!to_byte_array(tlv->child, array, index, length)) {
                    return false;
                }
            }
        } else { //pdo
            uint8_t *a = *array;
            memcpy(&a[*index], tlv->value, tlv->length);
            *index += tlv->length;
            len += tlv->length;
        }
        //set_length

        if (tlv->next != NULL) {
            return to_byte_array(tlv->next, array, index, length);
        }
        return true;
    }
    return false;
}

#ifdef TLV_DEBUG


static bool to_string(const tlv_t *tlv, char *str, size_t size, tlv_level_t level) {
    int ret;
    char space[(4 * level) + 1];

    memset(space, ' ', (sizeof (space) - 1));
    space[sizeof (space) - 1] = '\0';

    if (tlv->type == TLV_CDO) {
        ret = snprintf(str, size, "%s|cdo+%u|-[]\n", space, tlv->tag);
        if (ret < 0 || (size_t) ret > size) {
            return false;
        }
        if (tlv->child != NULL) {
            size_t len = strlen(str);

            if (!to_string(tlv->child, &str[len], size - len, (level + 1))) {
                return false;
            }
        }
    } else {
        if (tlv->length > 0) {
            char value[(tlv->length * 5) + 1];

            for (uint16_t i = 0; i < tlv->length; i++) {
                ret = snprintf(&value[i * 5], sizeof (value), "0x%02X ", tlv->value[i]);
                if (ret < 0 || (size_t) ret > sizeof (value)) {
                    return false;
                }
            }
            value[(tlv->length * 5)] = 0;

            ret = snprintf(str, size, "%s|pdo+%u|-[ %s]\n", space, tlv->tag, value);
            if (ret < 0 || (size_t) ret > size) {
                return false;
            }
        } else {
            ret = snprintf(str, size, "%s|pdo+%u|-[]\n", space, tlv->tag);
            if (ret < 0 || (size_t) ret > size) {
                return false;
            }
        }
    }
    if (tlv->next != NULL) {
        size_t len = strlen(str);
        return to_string(tlv->next, &str[len], size - len, level);
    }

    return true;
}

#endif
