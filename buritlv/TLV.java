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

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.Charset;

/**
 *
 * @author buri
 */
public final class TLV {

    private static final int MAX_VALUE_LENGTH = 65535;//32767*2;
    private static final int MAX_TAG_LENGTH = 127;//7F;

    private static final Charset UTF8_CHARSET = Charset.forName("UTF-8");

    private static final String errorGreater = String.format("Tag value greater then %s", MAX_TAG_LENGTH);
    private static final String errorNegative = "Tag value cannot be negative";
    private static final String errorTooLong = String.format("Value length greater then %s", MAX_VALUE_LENGTH);
    private static final String errorUnsupportedChild = "Cannot add child to PDO";
    private static final String tab = "    ";

    private static final ByteOrder byteOrder = java.nio.ByteOrder.nativeOrder();

    public static final boolean PDO = false;
    public static final boolean CDO = true;

    private int tag;
    private int length;
    private byte[] value;
    private int level;

    private TLV sibling;
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
    private TLV(int tag, byte[] value, boolean isCdo) throws IllegalArgumentException {
        if (tag > MAX_TAG_LENGTH) {
            throw new IllegalArgumentException(errorGreater);
        }
        if (tag < 0) {
            throw new IllegalArgumentException(errorNegative);
        }
        if (value != null && value.length > MAX_VALUE_LENGTH) {
            throw new IllegalArgumentException(errorTooLong);
        }

        this.isCdo = isCdo;
        this.tag = tag;
        this.value = value;
        this.length = 0;
        this.level = 0;
        this.sibling = null;
        this.child = null;
    }

    public byte[] toByteArray() {
        ByteBuffer buffer = ByteBuffer.allocate(3 + (value == null ? 0 : value.length));
        setFirstByte(tag, isCdo, buffer);
        setLengthAndValue(value, buffer);

        return buffer.array();
    }

    public byte[] toBytes() {
        int totalLength = getByteLength();
        ByteBuffer buffer = ByteBuffer.allocate(totalLength);
        
//todo fix bytes
        return buffer.array();
    }
    
    /**
     * Returns the length of this TLV (value + TLV data)
     * @return the length of the value + 3 or just 3 if it is a CDO
     */
    public int getLength(){
        int len = 3;
        if (this.value != null) {
            len += this.value.length;
        }
        return len;
    }

    /**
     * Returns the total length of this TLV (this TLV + child and sibling lengths)
     * @return length of this TLV inclusive the length of the underlying TLV's
     */
    public int getByteLength() {
        this.length = getLength();
        if (this.child != null) {
            this.length += this.child.getByteLength();
        }
        int len = this.length;
        if (this.sibling != null) {
            len += this.sibling.getByteLength();
        }

        return len;
    }

    /**
     * Creates a new TLV of CDO type with tag number
     *
     * @param tag tag to set
     * @return new TLV of CDO type
     * @throws IllegalArgumentException if tag or value length is greater then
     * allowed
     */
    public static TLV newCDO(int tag) throws IllegalArgumentException {
        return new TLV(tag, null, CDO);
    }

    /**
     * Creates a new TLV of PDO type with tag number and value
     *
     * @param tag tag number to set
     * @param value byte array or null to set
     * @return new TLV of PDO type
     * @throws IllegalArgumentException if tag or value length is greater then
     * allowed
     */
    public static TLV newPDO(int tag, byte[] value) throws IllegalArgumentException {
        return new TLV(tag, value, PDO);
    }

    /**
     * Creates a new TLV of PDO type with tag number and value
     *
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
     *
     * @return true if the TLV is PDO, false otherwise
     */
    public boolean isCDO() {
        return this.isCdo;
    }

    /**
     * Getter for tag
     *
     * @return the tag number, positive number between 0 and 127 (inclusive 0
     * and 127)
     */
    public int getTag() {
        return this.tag;
    }

    /**
     * Getter for value
     *
     * @return byte array or null
     */
    public byte[] getValue() {
        return this.value;
    }

    /**
     * Getter for child
     *
     * @return TLV object corresponding the direct child
     */
    public TLV getChild() {
        return this.child;
    }

    /**
     * Getter for sibling
     *
     * @return TLV object corresponding the direct "sibling"
     */
    public TLV getSibling() {
        return this.sibling;
    }

    /**
     * Returns the last child or null if this hasn't any child
     *
     * @return Last child as TLV object or null
     */
    public TLV getLastChild() {
        if (this.child == null) {
            return null;
        }
        return child.getLastSibling();
    }

    /**
     * Returns the last TLV object in the "sibling" chain If the sibling of the
     * object is null, then this object is returned
     *
     * @return TLV object
     */
    public TLV getLastSibling() {
        if (sibling == null) {
            return this;
        }
        return sibling.getLastSibling();
    }

    /**
     * Adds a child to this TLV object Current child with siblings will be
     * replaced
     *
     * @param tlv TLV object to add as first child
     * @return the currently added TLV object
     * @throws UnsupportedOperationException when trying to add child to a PDO
     */
    public TLV setChild(TLV tlv) throws UnsupportedOperationException {
        if (!this.isCdo) {
            throw new UnsupportedOperationException(errorUnsupportedChild);
        }
        this.child = tlv;
        this.child.level = this.level + 1;
        return this.child;
    }

    /**
     * Adds a child as last child to this TLV object
     *
     * @param tlv TLV to add as last child
     * @return the currently added TLV object
     * @throws UnsupportedOperationException when trying to add child to a PDO
     */
    public TLV appendChild(TLV tlv) throws UnsupportedOperationException {
        if (this.child == null) {
            return setChild(tlv);
        }
        return this.child.appendSibbling(tlv);
    }

    /**
     * Adds a sibling to this TLV object Current sibling, if any, will be
     * replaced
     *
     * @param tlv TLV object to add as first sibling
     * @return the currently added TLV object
     */
    public TLV setSibbling(TLV tlv) {
        this.sibling = tlv;
        this.sibling.level = this.level + 1;
        return this.sibling;
    }

    /**
     * Adds a sibling as last sibling to this TLV object
     *
     * @param tlv TLV object to add as last sibling
     * @return the currently added TLV object
     */
    public TLV appendSibbling(TLV tlv) {
        if (this.sibling == null) {
            return this.setSibbling(tlv);
        }
        return this.sibling.appendSibbling(tlv);
    }
    
    public String toStringAll(){
        StringBuilder sb = new StringBuilder();
        String t ="";
        for (int i = 0; i < level; i++) {
            t += tab;
        }
        sb.append(t).append(toString());
        if(child != null){
            sb.append(child.toStringAll());
        }
        if(sibling != null){
            sb.append(sibling.toStringAll());
        }
        return sb.toString();
    }
    
    @Override
    public String toString(){
        StringBuilder sb = new StringBuilder();
        
        if(isCdo) sb.append("CDO-");
        else sb.append("PDO-");
        sb.append(String.valueOf(tag));
        sb.append("(");
        if(value != null) sb.append(decodeUTF8(value));
        sb.append(")");
        sb.trimToSize();
        return sb.toString();
    }
    ///////////////////////////////////////////////////////////////////////////

    /**
     * Converts a a byte array to UTF-8 string
     *
     * @param bytes byte array to convert
     * @return converted string or null if argument is null if the byte array is
     * not null but has zero length an empty string will be returned
     */
    public static String decodeUTF8(byte[] bytes) {
        if (bytes == null) {
            return null;
        }
        return new String(bytes, UTF8_CHARSET);
    }

    /**
     * Converts a UTF-8 string to a byte array
     *
     * @param value string to convert
     * @return byte array or null if argument is null or empty array if string
     * is empty
     */
    public static byte[] encodeUTF8(String value) {
        if (value == null) {
            return null;
        }
        return value.getBytes(UTF8_CHARSET);
    }

    private static void setFirstByte(int tag, boolean cdo, ByteBuffer buffer) {
        if (cdo) {
            buffer.put((byte) (tag | 0x00000080));
        } else {
            buffer.put((byte) (tag & 0x0000007F));
        }
    }

    private void setLengthAndValue(byte[] value, ByteBuffer buffer) {
        if (value == null || value.length == 0) {
            buffer.put((byte)0x00).put((byte)0x00);
        } else {
            if (byteOrder == ByteOrder.BIG_ENDIAN) {
                buffer.put((byte) ((value.length >> 8) & 0xFF));
                buffer.put((byte) (value.length & 0xFF));
            } else {
                buffer.put((byte) (0x000000FF & value.length));
                buffer.put((byte) ((value.length >> 8) & 0xff));
            }
            buffer.put(value);
        }
    }
}
