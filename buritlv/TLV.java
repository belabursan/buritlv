/*
 * The MIT License
 *
 * Copyright 2015 buri.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
package buritlv;

import java.nio.charset.Charset;

/**
 *
 * @author buri
 */
public final class TLV {

    private static final int MAX_VALUE_LENGTH = 65535;//32767*2;
    private static final int MAX_TAG_LENGTH = 127;//7F;
    private static final Charset UTF8_CHARSET = Charset.forName("UTF-8");
    public static final boolean PDO = false;
    public static final boolean CDO = true;

    private int tag;
    private int length;
    private byte[] value;

    private TLV next;
    private TLV child;

    private boolean isCdo;

    /**
     * Constructor, creates a new TLV object
     *
     * @param tag tag value, must be lower then 128
     * @param value byte array, can be null and length must be lower then 65535
     * @param isCdo boolean, true if CDO, false otherwise
     * @throws IllegalArgumentException if tag or value length is greater then
     * allowed
     */
    public TLV(int tag, byte[] value, boolean isCdo) throws IllegalArgumentException {
        if (tag > MAX_TAG_LENGTH) {
            throw new IllegalArgumentException("Tag value greater then " + MAX_TAG_LENGTH);
        }
        if (value != null && value.length > MAX_VALUE_LENGTH) {
            throw new IllegalArgumentException("Value length greater then " + MAX_VALUE_LENGTH);
        }

        this.isCdo = isCdo;
        this.tag = tag;
        this.value = value;
        this.next = null;
        this.child = null;
    }
    
    /**
     * Creates a new TLV of CDO type with tag number
     * @param tag tag to set
     * @return new TLV of CDO type
     * @throws IllegalArgumentException if tag or value length is greater then
     * allowed
     */
    public static TLV newCDO(int tag) throws IllegalArgumentException {
        return new TLV(tag, null, true);
    }
    
    /**
     * Creates a new TLV of PDO type with tag number and value
     * @param tag tag number to set
     * @param value byte array or null to set
     * @return new TLV of PDO type
     * @throws IllegalArgumentException if tag or value length is greater then
     * allowed
     */
    public static TLV newPDO(int tag, byte[] value) throws IllegalArgumentException {
        return new TLV(tag, value, false);
    }
    
    /**
     * Creates a new TLV of PDO type with tag number and value
     * @param tag tag number to set
     * @param value string or null to set
     * @return new TLV of PDO type
     * @throws IllegalArgumentException if tag or value length is greater then
     * allowed
     */
    public static TLV newPDO(int tag, String value) throws IllegalArgumentException {
        return newPDO(tag, encodeUTF8(value));
    }
    
    /**
     * Getter for isCDO
     * @return true if the TLV is PDO, false otherwise
     */
    public boolean isCDO(){
        return this.isCdo;
    }
    
    /**
     * Getter for tag
     * @return the tag number, positive number between 0 and 127
     * (inclusive 0 and 127)
     */
    public int getTag(){
        return this.tag;
    }
    
    /**
     * Getter for value
     * @return byte array or null
     */
    public byte[] getValue(){
        return this.value;
    }

    /**
     * Getter for child
     * @return TLV object corresponding the direct child
     */
    public TLV getChild(){
        return this.child;
    }
    
    /**
     * Getter for next
     * @return TLV object corresponding the direct "next"
     */
    public TLV getNext(){
        return this.next;
    }
    
    /**
     * Returns the last child or null if this hasn't any child
     * @return Last child as TLV object or null
     */
    public TLV getLastChild(){
        if(this.child == null) return null;
        return child.getLastNext();
    }
    
    /**
     * Returns the last TLV object in the "next" chain
     * If the next of the object is null, then this object is returned
     * @return TLV object
     */
    public TLV getLastNext(){
        if(next == null) return this;
        return next.getLastNext();
     }
    // // // // // // // // // // // // // // // // // // // // // // // //

    /**
     * Adds a child to this TLV object
     * Current child will be replaced
     * @param tlv TLV object or null
     */
    public void addChild(TLV tlv){
        this.child = tlv;
    }
    ///////////////////////////////////////////////////////////////////////////

     /**
     * Converts a a byte array to UTF-8 string
     *
     * @param bytes byte array to convert
     * @return converted string or null if argument is null
     * if the byte array is not null but has zero length an empty string 
     * will be returned
     */
    public static String decodeUTF8(byte[] bytes) {
        if(bytes == null) return null;
        return new String(bytes, UTF8_CHARSET);
    }

     /**
     * Converts a UTF-8 string to a byte array
     *
     * @param value string to convert
     * @return byte array
     *          or null if argument is null
     *          or empty array if string is empty
     */
    public static byte[] encodeUTF8(String value) {
        if (value == null) return null;
        return value.getBytes(UTF8_CHARSET);
    }
}
