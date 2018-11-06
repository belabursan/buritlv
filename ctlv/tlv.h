/* 
 * File:   tlv.h
 * Author: Bela Bursan
 *
 * Created on October 12, 2016, 9:23 AM
 */

#ifndef TLV_H_2016
#define TLV_H_2016

#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif

    typedef struct sTLV tlv_t;
    typedef uint16_t tlv_tag_t;
    typedef uint16_t tlv_length_t;
#ifdef TLV_DEBUG
    typedef uint16_t tlv_level_t;
#endif
    
    typedef enum {
        TLV_NOT_SET = 0,
        TLV_CDO = 0xFA,
        TLV_PDO = 0xBA
    } tlv_type_t;

    
    struct sTLV {
        tlv_t *next;
        tlv_t *child;
        uint8_t *value;
        tlv_tag_t tag;
        tlv_length_t length;
        tlv_type_t type;
        bool value_is_dynamic;
#ifdef TLV_DEBUG
        tlv_level_t level;
#endif
    };


    /**
     * @brief Creates a new TLV object of CDO type
     * @param tag The tag of the tlv object
     * @return The tlv object or NULL
     */
    tlv_t* tlv_new_cdo(const tlv_tag_t tag);


    /**
     * @brief Creates a new TLV object of PDO type
     * @param tag The tag of the object
     * @param length The length of the data
     * @param value Pointer to data to set
     * @return The tlv object or NULL
     */
    tlv_t* tlv_new_pdo(const tlv_tag_t tag, const tlv_length_t length, uint8_t *value);


    /**
     * @brief Appends a TLV object to the end of "next" chain
     * @param tlv Tlv object to append the next element on
     * @param next Tlv object to append
     * @return Pointer to "next" or NULL if "next" or "tlv" is null
     */
    tlv_t* tlv_append_next(tlv_t *tlv, tlv_t *next);


    /**
     * @brief Appends a TLV object to the end of "child" chain
     * @param tlv Tlv object to append the child element on
     * @param child Tlv object to append
     * @return Pointer to "child" or NULL if "next" or "tlv" is null or "tlv" isn't CDO
     */
    tlv_t* tlv_append_child(tlv_t *tlv, tlv_t *child);


    /**
     * @brief Finds a tlv object with the specified tag
     * @param tlv The tlv object to search recursively
     * @param tag Tag of the tlv to search for
     * @return The requested tlv object or NULL if not found 
     */
    const tlv_t* tlv_find_by_tag(const tlv_t *tlv, const tlv_tag_t tag);


    /**
     * @brief Deletes a tlv object recursively
     * The function deletes the allocated resources for the tlv objects but not for the values
     * @param tlv Tlv object to delete
     */
    void tlv_delete(tlv_t **tlv);


    /**
     * @brief Deletes a tlv object recursively
     * Also deletes the value, child and next struct recursively
     * @param tlv Tlv object to delete
     */
    void tlv_delete_all(tlv_t **tlv);
    
    //bool tlv_to_byte_array(tlv_t *tlv, uint8_t *barray, size_t *size);
    
    
    //tlv_t* tlv_from_byte_array(uint8_t *barray, size_t size);
    
    
#ifdef TLV_DEBUG   
    
    /**
     * Returns the string representation of the tlv object
     * Note: the returned string is dynamically allocated and must be "freed" after use
     * 
     * @param tlv Tlv object to "stringify"
     * @return The string or null
     */
    const char* tlv_to_string(const tlv_t *tlv);
    
    
    /**
     * Callback function for debug information
     * Note: This function is defined as weak, override is possible
     * 
     * @param txt Debug text
     * @param ... Variable length arguments
     */
    extern void tlv_debug_cb(const char *txt, ...);
    
#endif /* TLV_DEBUG */

#ifdef __cplusplus
}
#endif

#endif /* TLV_H_2016 */
