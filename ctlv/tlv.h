#ifndef TLV_H_2016
#define TLV_H_2016
/**
 * File:   tlv.h
 * Author: Bela Bursan
 *
 * Created on October 12, 2016, 9:23 AM
 *
 * There is no licence, use it as you wish.
 *
 * @brief This is an implementation of a special type of TLV parser
 * TLV stands for Tag-Length-Value which is an encoding scheme used for optional information element in a certain protocol
 *
 * Tag - A binary code, often simply alphanumeric, which indicates the kind of field that this part of the message represents
 * Length - The size of the value field (in bytes)
 * Value - Variable-sized series of bytes which contains data for this part of the message
 */

#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif

    //
    // Header of a TLV object:
    // _________________ _________________ _________________ _________________ ________________            _________________
    // |t|t|t|t|t|t|t|t| |T|T|T|T|T|T|T|T| |T|T|T|T|T|T|T|T| |L|L|L|L|L|L|L|L| |L|L|L|L|L|L|L|L|   if pdo: |V|V|V|V|V|V|V|V| ...
    // ----------------- ----------------- ----------------- ----------------- -----------------           -----------------
    //       Byte 1           Byte 2           Byte 3             Byte 4          Byte 5                      byte 6 - ... 
    //
    // Where:
    //      - byte 1 holds the TYPE, 0xFA or 0xBA for CDO/PDO
    //      - bytes 2-3 holds the TAG of the TLV, it is an uint16_t
    //      - bytes 4-5 holds the LENGTH of the TLV
    //
    // Structure:
    //   Example 1:
    //   |cdo+10|-[]                - root CDO
    //      |pdo+12|-[ hello ]      - hello is the data(value) in a PDO
    //
    //   Example 2:
    //   |cdo+1|-[]
    //      |pdo+2|-[abc]
    //      |cdo+3|-[]
    //         |cdo+30|-[]
    //         |pdo+35|-[abcd]
    //         |pdo+50|-[abcdefghi]
    //      |pdo+222|-[abcdefghijklmno]
    //
    // TLV structures starts always with a CDO, the root
    // CDO's contains only other CDO's and PDO's, no data(value)
    // The total length of a TLV structure, inclusive the length of value cannot be greater then 65535 bytes since the length is represented by an uint16_t
    //

    typedef struct stTLV tlv_t;
    typedef uint16_t tlv_tag_t;
    typedef uint16_t tlv_length_t;
    typedef uint8_t tlv_type_t;
#ifdef TLV_DEBUG
    typedef uint16_t tlv_level_t;
#endif
    static const uint8_t BER_HEADER_BYTE_LENGTH = 5; // 1 byte type, 2 bytes tag(uint16_t) and 2 bytes length(uint16_t)

    /**
     * Represents the types of a TLV object
     */
    typedef enum {
        TLV_NOT_SET = 0, /**< @brief Not set */
        TLV_CDO = 0xFA, /**< @brief CDO - Constructed Data Object, acts as a directory contains other CDO's or PDO's, cannot hold data(value) */
        TLV_PDO = 0xBA /**< @brief PDO - Primitive Data Object, can only hold data (value) */
    } tlv_types_t;

    /**
     * Struct representing a TLV object
     */
    struct stTLV {
        tlv_t *next;            /**< @brief Pointer to a "next" TLV object */
        tlv_t *child;           /**< @brief Pointer to a "child" TLV object */
        uint8_t *value;         /**< @brief Pointer to value, used only in PDO's */
        tlv_tag_t tag;          /**< @brief Tag of the TLV */
        tlv_length_t length;    /**< @brief Length of data in PDO's, total length of childs for CDO's */
        tlv_type_t type;        /**< @brief Type of the TLV (CDO/PDO) */
#ifdef TLV_DEBUG
        tlv_level_t level;      /**< @brief Level, used when (debug) printing the TLV */
#endif
    };


    /**
     * @brief Creates a new TLV object of CDO type
     * @param[in] tag The tag of the tlv object
     * @return The tlv object or NULL
     */
    tlv_t* tlv_new_cdo(const tlv_tag_t tag);


    /**
     * @brief Creates a new TLV object of PDO type
     * @param[in] tag The tag of the object
     * @param[in] length The length of the data
     * @param[in] value Pointer to data to set
     * @return The tlv object or NULL
     */
    tlv_t* tlv_new_pdo(const tlv_tag_t tag, const tlv_length_t length, uint8_t *value);


    /**
     * @brief Appends a TLV object to the end of "next" chain
     * @param[in] tlv Tlv object to append the next element on
     * @param[in] next Tlv object to append
     * @return Pointer to "next" or NULL if "next" or "tlv" is null
     */
    tlv_t* tlv_append_next(tlv_t *tlv, tlv_t *next);


    /**
     * @brief Appends a TLV object to the end of "child" chain
     * @param[in] tlv Tlv object to append the child element on
     * @param[in] child Tlv object to append
     * @return Pointer to "child" or NULL if "next" or "tlv" is null or "tlv" isn't CDO
     */
    tlv_t* tlv_append_child(tlv_t *tlv, tlv_t *child);


    /**
     * @brief Finds a tlv object with the specified tag
     * @param[in] tlv The tlv object to search recursively
     * @param[in] tag Tag of the tlv to search for
     * @return The requested tlv object or NULL if not found 
     */
    const tlv_t* tlv_find_by_tag(const tlv_t *tlv, const tlv_tag_t tag);


    /**
     * @brief Deletes a tlv object recursively
     * The function deletes the allocated resources for the tlv objects but not for the values
     * @param[in] tlv Tlv object to delete
     */
    void tlv_delete(tlv_t **tlv);


    /**
     * @brief Deletes a tlv object recursively
     * Also deletes the value, child and next struct recursively
     * @param[in] tlv Tlv object to delete
     */
    void tlv_delete_all(tlv_t **tlv);


    /**
     * Converts a tlv object to a byte array
     * @param[in] tlv Tlv object to convert
     * @param[out] barray Return point of the byte array
     * @param[out] size Size of the returned byte array
     * @return True if successful, false otherwise
     */
    bool tlv_to_byte_array(tlv_t *tlv, uint8_t **barray, size_t *size);


    //tlv_t* tlv_from_byte_array(uint8_t *barray, size_t size);


#ifdef TLV_DEBUG   

    /**
     * Returns the string representation of the tlv object
     * Note: the returned string is dynamically allocated and must be "freed" after use
     * 
     * @param[in] tlv Tlv object to "stringify"
     * @return The string or null
     */
    const char* tlv_to_string(const tlv_t *tlv);


    /**
     * Callback function for debug information
     * Note: This function is defined as weak, override is possible
     * 
     * @param[in] txt Debug text
     * @param[in] ... Variable length arguments
     */
    extern void tlv_debug_cb(const char *txt, ...);

#endif /* TLV_DEBUG */

#ifdef __cplusplus
}
#endif

#endif /* TLV_H_2016 */
