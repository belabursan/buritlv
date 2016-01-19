/*****************************************************************************
 * 
 * Copyright (c) 2016 Bela Bursan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 *  ---------------------------------------------------------------------------
 *
 * [ DISCLAIMER ]
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/
package com.buri.tlv;

import java.io.UnsupportedEncodingException;

/**
 * TLV class, represents a tag-length value objects
 * @author <a href="mailto:bursan@gmail.com">Bela Bursan</a>
 */
public final class TLV {

    private int tag;
    private int length;
    private byte[] value;
    private boolean tlvForm;    //pdo or cdo, true if cdo
    private short tlvClass;    //Universal, Application, Context Specific, Private 
    private short level;
    private TLV next;
    private TLV child;
    //
    private final static String PRINT_TAB = "   ";
    /**
     * Constant for Tag Class Universal
     */
    public final static short CLASS_UNIVERSAL = 0x00;
    /**
     * Constant for Tag Class Application
     */
    public final static short CLASS_APPLICATION = 0x01;
    /**
     * Constant for Tag Class Context Specific
     */
    public final static short CLASS_CONTEXT_SPEC = 0x02;
    /**
     * Constant for Tag Class Private
     */
    public final static short CLASS_PRIVATE = 0x03;

    /**
     * Default constructor<br>
     * Creates a new (empty) TLV-tag.
     */
    public TLV() {
        reset();
    }

    /**
     * Constructor<br>
     * Creates a new TLV-tag, tag, length, class and tlvForm must be set.<br>
     * If value is set, the length will be set by constructor
     *
     * @param tag integer tag number
     * @param isCdo boolean, true if CDO, false if PDO
     * @param tlvClass Universal = 0, Application = 1, Context Specific = 2,
     * Private = 3
     * @param value byte[] value, if CDO = content of CDO, if PDO value of PDO
     * @throws java.lang.IllegalArgumentException
     */
    TLV(int tag, boolean isCdo, short tlvClass, byte[] value) throws IllegalArgumentException {
        this();
        this.tag = tag;
        if (value == null) {
            this.length = 0;
        } else {
            this.value = value;
            this.length = value.length;
        }
        this.tlvForm = isCdo;
        this.tlvClass = tlvClass;
        this.level = 0;

        if (!isValid()) {
            throw new IllegalArgumentException("ERROR: Not valid value");
        }
    }

    /**
     * Creates a new TLV from bytes received
     *
     * @param rawValue
     * @param startpos start position in the buffer
     * @param length length of bytes in byte buffer
     * @throws IllegalArgumentException
     */
    public TLV(byte[] rawValue, int startpos, int length) throws IllegalArgumentException {
        this();
        getAndSetFirstTag(rawValue,startpos, length);
        this.extractValue();
    }
    
    private void getAndSetFirstTag(byte[] rawValue, int startPosition, int length){
        TLVFactory.getAndSetFirstTLVTag(this, rawValue, startPosition, length);
    }

    /**
     * Creates a new TLV from bytes received
     *
     * @param rawValue
     * @return the remaining bytes if those was longer then the whole TLV(maybe
     * from the next TLV?)
     * @throws IllegalArgumentException
     */
    byte[] createFromRawValue(byte[] rawValue) throws IllegalArgumentException {
        return createFromRawValue(rawValue, 0, rawValue.length);
    }

    /**
     * Creates a new TLV from bytes received
     *
     * @param rawValue
     * @param startpos start position in the buffer
     * @param length length of bytes in byte buffer
     * @return the remaining bytes if those was longer then the whole TLV(maybe
     * bytes from the next TLV?)
     * @throws IllegalArgumentException
     */
    byte[] createFromRawValue(byte[] rawValue, int startpos, int length) throws IllegalArgumentException {
        byte[] rem = TLVFactory.getAndSetFirstTLVTag(this, rawValue, startpos, length);
        this.extractValue();
        return rem;
    }

    /**
     * Check if this TLV tag is valid.
     *
     * @return true if tag, length and class is set and the length is the same
     * as value.length, otherwise returns false
     */
    public boolean isValid() {
        if (tag < 0 || length < 0 || tlvClass < 0) {
            return false;
        }
        return !(value != null && value.length != length);
    }

    /**
     * Resets the tag.<br>
     * This is recursive function, resets all the child and siblings<br>
     * Tag, length, class sets to -1, value, next and child to null and level to
     * 0.
     */
    public void reset() {
        if (child != null) {
            child.reset();
        }
        if (next != null) {
            next.reset();
        }
        this.tag = -1;
        this.length = -1;
        this.tlvClass = -1;
        this.tlvForm = false;
        this.value = null;
        this.child = null;
        this.next = null;
        this.level = 0;
    }

    /**
     * Prints out this Tag in form |-pdo(tag)[value] or |-cdo(tag)[]
     *
     * @return this TLV-tag as stringBuffer
     */
    @Override
    public String toString() {
        StringBuffer ss = toString(new StringBuffer(1024));
        ss.trimToSize();
        return ss.toString();
    }

    private StringBuffer toString(StringBuffer ss) {
        String tabs = "";
        for (int i = 0; i < level; i++) {
            tabs += PRINT_TAB;
        }
        if (tlvForm) {
            ss.append(tabs).append("|-cdo").append(tag).append("[]\n");
            if (child != null) {
                child.toString(ss);
            }
            if (next != null) {
                return next.toString(ss);
            } else {
                return ss;
            }
        } else {
            ss.append(tabs).append("|-pdo").append(tag).append("[").append(getValueAsString()).append("]\n");
            if (next != null) {
                return next.toString(ss);
            } else {
                return ss;
            }
        }
    }

    /**
     * Extracts the value of this TLV-tag and creates new TLV-tags from it.<br>
     * This function is recursive that means that all it's child will be
     * extracted too
     *
     * @throws java.lang.IllegalArgumentException if the value of this tag is
     * not valid BER string
     */
    private void extractValue() throws IllegalArgumentException {
        if (this.value != null) {
            this.child = new TLV();
            this.value = TLVFactory.getAndSetFirstTLVTag(this.child, this.value, 0, this.value.length);
            this.child.setLevel(this.level + 1);
            setLength(0);
            TLV last = this.child;
            if (last.isCdo()) {
                last.extractValue();
            }
            while (this.value != null) {
                TLV tempTlv = new TLV();
                this.value = TLVFactory.getAndSetFirstTLVTag(tempTlv, this.value, 0, this.value.length);
                tempTlv.setLevel(this.child.getLevel());
                last.setNext(tempTlv);
                last = tempTlv;
                if (last.isCdo()) {
                    last.extractValue();
                }
            }
        } else {
            this.next = null;
            this.child = null;
        }
    }

    /**
     * Creates a binary representation recursively of this TLV and all it's
     * child objects
     *
     * @return this object as a byte array
     * @throws java.lang.IllegalArgumentException
     */
    public byte[] convertToByteArray() throws IllegalArgumentException {
        byte[] ret = null;
        if (child != null) {
            TLVFactory.addElementToCdo(this, child.convertToByteArray());
            TLV temp = child.getNext();
            while (temp != null) {
                TLVFactory.addElementToCdoAtEnd(this, temp.convertToByteArray());
                temp = temp.getNext();
            }
            ret = TLVFactory.convertTLVToByteArray(this);
            setValue(null);
        }
        if (ret == null) {
            return TLVFactory.convertTLVToByteArray(this);
        }
        return ret;
    }

    /**
     * Finds all occurrence(in the hole structure) of TLVTags with the specified
     * tag number
     *
     * @param inTagNumber integer tag number to find
     * @return a TLV object defined by the tag number or null if not found
     */
    public TLV findTag(int inTagNumber) {
        if (this.tag == inTagNumber) {
            return this;
        }
        TLV temp;
        if (this.child != null) {
            temp = child.findTag(inTagNumber);
            if (temp != null) {
                return temp;
            }
        }
        if (this.next != null) {
            temp = next.findTag(inTagNumber);
            if (temp != null) {
                return temp;
            }
        }
        return null;
    }

    /**
     * Returns the value of this TLV-tag.
     * @return value as byte arrays
     */
    public byte[] getValue() {
        return value;
    }

    /**
     * Get the value of this TLV-tag as a string.<br>
     * If this tag is a CDO, null will be returned.<br>
     * If the PDO value is null an empty string will be returned
     *
     * @return the value of the TLV as a string
     */
    public String getValueAsString() {
        if (tlvForm) {
            return null;
        } else {
            if (value == null) {
                return "";
            }
            try {
                return new String(value, "ISO-8859-1");
            } catch (UnsupportedEncodingException ue) {
                System.err.println("¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤>> unsuported encoding: " + ue.getMessage());
                return new String(value);
            }
        }
    }

    /**
     * Sets the value of this TLV-tag and updates the length<br>
     * @param value byte array to be set, can be null
     */
    public void setValue(byte[] value) {
        if (value == null) {
            this.value = null;
            this.length = 0;
        } else {
            this.value = value;
            this.length = value.length;
        }
    }

    /**
     * Returns the length of the value of this TLV-tag.<br>
     * If this TLV-tag has no value 0 will be returned.<br>
     * If -1 is returned than the TLV is invalid(not yet set).
     *
     * @return the length of the value as integer
     */
    public int getLength() {
        return length;
    }

    /**
     * Sets the length of the value of this TLV.<br>
     * If this TLV has no value, the length
     * must be set to 0
     * @param length the length of the value
     */
    public void setLength(int length) {
        this.length = length;
    }

    /**
     * Returns the tag of this TLV
     * @return the tag as integer
     */
    public int getTag() {
        return tag;
    }

    /**
     * Sets the tag part of this Tag
     * @param tag the tag of this TLV
     */
    public void setTag(int tag) {
        this.tag = tag;
    }

    /**
     * Returns the TLV class part of this Tag
     * @return the TLV class as short
     */
    public short getClassType() {
        return tlvClass;
    }

    /**
     * Sets the tag class type of this TLV
     * @param tlvClass class to set
     */
    public void setTlvClass(int tlvClass) {
        this.tlvClass = (short) tlvClass;
    }

    /**
     * Sets the tag level in the TLV-tree
     * @param level level to set
     */
    public void setLevel(int level) {
        this.level = (short) level;
        if (child != null) {
            child.setLevel(level + 1);
        }
        if (next != null) {
            next.setLevel(level);
        }
    }

    /**
     * Returns the tag level in the TLV-tree
     * @return the Tag level as integer
     */
    public int getLevel() {
        return (int) level;
    }

    /**
     * Sets a boolean that should represent the tlvForm of this TLV<br>
     * Therefore should not be changed by hand, only by PipeTLVFactory
     *
     * @param tlvForm true if is constructed, false otherwise
     */
    public void setIsCdo(boolean tlvForm) {
        this.tlvForm = tlvForm;
    }

    /**
     * Used to query if this BER tag structure is constructed
     * @return True if tag is constructed or false
     */
    public boolean isCdo() {
        return tlvForm;
    }

    /**
     * Returns the sibling(in the tree) to this tag
     * @return TLV or null
     */
    public TLV getNext() {
        return next;
    }

    /**
     * Sets the sibling of this tag
     * @param next
     */
    public void setNext(TLV next) {
        this.next = next;
        next.setLevel(level);
    }

    /**
     * Returns the child(in the tree) to this tag
     * @return TLV or null
     */
    public TLV getChild() {
        if (this.tlvForm) {
            return this.child;
        }
        return null;
    }

    /**
     * Sets the child of this tag
     * @param tlv
     * @return returns the child (to be able to chain)
     */
    public TLV setChild(TLV tlv) {
        if (this.tlvForm) {
            this.child = tlv;
            child.setLevel(this.level + 1);
        } else {
            this.child = null;
        }
        return child;
    }

    /**
     * Adds a child to this tag. If child already exists, this will be append at
     * the end of child list.
     * @param tlv child to add
     * @return the child
     */
    public TLV addChild(TLV tlv) {
        if (this.child == null) {
            setChild(tlv);
        } else {
            this.child.addNextAtEnd(tlv);
        }
        return tlv;
    }

    /**
     * Adds a child to the end of next. That means that if next is null the tag
     * will be append there.<br>
     * If next is not null the same procedure will be made on the next.
     * @param tlv tag to add
     */
    public void addNextAtEnd(TLV tlv) {
        if (this.next == null) {
            setNext(tlv);
        } else {
            next.addNextAtEnd(tlv);
        }
    }

    /**
     * Used by the TLV class
     * @see TLV
     */
    private final static class TLVFactory {

        /**
         * Gets the first TLV-object and returns the remaining bytes if there is
         * more TLV after
         *
         * @param tlv TLV to be set
         * @param byteArr from the TLV is set
         * @return byte array with the remaining bytes
         * @throws java.lang.IllegalArgumentException
         */
        static byte[] getAndSetFirstTLVTag(TLV tlv, byte[] byteArr, int startpos, int length)
                throws IllegalArgumentException {
            return setTLVTag(tlv, byteArr, startpos, length, true);
        }

        /**
         * Converts a TLV object to a byte array
         *
         * @param tlv to convert
         * @return byte[] the byte representation of the TLV-object
         * @throws java.lang.IllegalArgumentException
         */
        static synchronized byte[] convertTLVToByteArray(TLV tlv) throws IllegalArgumentException {
            if (tlv == null || !tlv.isValid()) {
                throw new IllegalArgumentException("Error: tlv is null or empty");
            }
            byte[] bytesOut;
            int tagLen = 0;
            int lengthLen;
            int tag = tlv.getTag();
            byte tempByte = 0x00;

            tempByte = setTlvClass(tempByte, tlv.getClassType());  //set class in first byte
            tempByte = setTlvForm(tempByte, tlv.isCdo());         //set form
            if (tlv.getTag() < 31) {
                tempByte = setTagInFirstByte(tempByte, tlv.getTag());

            } else {
                tempByte = setTagInFirstByte(tempByte, (byte) 0x1F);
                tagLen = getNrOfUsedTagBytes(tlv.getTag());
            }

            lengthLen = getNrOfUsedLenBytes(tlv.getLength());
            bytesOut = new byte[(1 + tagLen + 1 + lengthLen + tlv.getLength())];

            bytesOut[0] = tempByte;

            setTagBytes(1, tag, tagLen, bytesOut);     //set tag
            setLengthBytes(1 + tagLen, tlv.getLength(), lengthLen, bytesOut);     //set Length
            if (tlv.getLength() > 0) {
                System.arraycopy(tlv.getValue(), 0, bytesOut, 1 + tagLen + lengthLen + 1, tlv.getLength());
            }

            return bytesOut;
        }

        /**
         * Adds a byte array at the end of the TLV-value and sets the new length
         *
         * @param tlv
         * @param bytesToAdd
         * @throws java.lang.IllegalArgumentException
         */
        static synchronized void addElementToCdoAtEnd(TLV tlv, byte[] bytesToAdd) throws IllegalArgumentException {
            if (tlv == null || !tlv.isValid()) {
                throw new IllegalArgumentException("Error: parameter tlv is null or invalid");
            }
            if (bytesToAdd == null) {
                throw new IllegalArgumentException("Error: parameter bytesToAdd is null(level:" + tlv.getLevel() + ", tag:" + tlv.getTag() + ")");
            }
            if (tlv.isCdo()) {
                byte[] newValue = new byte[tlv.getLength() + bytesToAdd.length];
                if (tlv.getValue() != null) {
                    System.arraycopy(tlv.getValue(), 0, newValue, 0, tlv.getLength());
                }
                System.arraycopy(bytesToAdd, 0, newValue, tlv.getLength(), bytesToAdd.length);
                tlv.setValue(newValue);
                tlv.setLength(newValue.length);
            } else {
                throw new IllegalArgumentException("Error: tlv is not cdo(level:" + tlv.getLevel() + ", tag:" + tlv.getTag() + ")");
            }
        }

        /**
         * Adds a byte array at the beginning of the TLV-value and sets the new
         * length
         *
         * @param tlv
         * @param bytesToAdd
         * @throws java.lang.IllegalArgumentException
         */
        static synchronized void addElementToCdo(TLV tlv, byte[] bytesToAdd) throws IllegalArgumentException {

            if (tlv == null || !tlv.isValid()) {
                throw new IllegalArgumentException("Error: parameter tlv is null or invalid");
            }
            if (bytesToAdd == null) {
                throw new IllegalArgumentException("Error: parameter bytesToAdd is null(level:" + tlv.getLevel() + ", tag:" + tlv.getTag() + ")");
            }
            if (tlv.isCdo()) {
                byte[] newValue = new byte[tlv.getLength() + bytesToAdd.length];
                System.arraycopy(bytesToAdd, 0, newValue, 0, bytesToAdd.length);
                if (tlv.getValue() != null) {
                    System.arraycopy(tlv.getValue(), 0, newValue, bytesToAdd.length, tlv.getLength());
                }
                tlv.setValue(newValue);
                tlv.setLength(newValue.length);
            } else {
                throw new IllegalArgumentException("Error: tlv is not cdo(level:" + tlv.getLevel() + ", tag:" + tlv.getTag() + ")");
            }
        }

        /**
         * Adds a TLV object at the end of the TLV-value and sets the new length
         *
         * @param tlvContainer TLV that holds the new TLV
         * @param tlvToAdd TLV to be added
         * @throws java.lang.IllegalArgumentException
         */
        static synchronized void addTLVToCDO(TLV tlvContainer, TLV tlvToAdd) throws IllegalArgumentException {
            addElementToCdo(tlvContainer, convertTLVToByteArray(tlvToAdd));
        }

        /**
         * Adds a TLV object at the beginning of the TLV-value and sets the new
         * length
         *
         * @param tlvContainer TLV that holds the new TLV
         * @param tlvToAdd TLV to be added
         * @throws java.lang.IllegalArgumentException
         */
        static synchronized void addTLVToCDOAtEnd(TLV tlvContainer, TLV tlvToAdd) throws IllegalArgumentException {
            addElementToCdoAtEnd(tlvContainer, convertTLVToByteArray(tlvToAdd));
        }

        /**
         * Creates a new TLV object and sets all the values(class, form, tag,
         * length, value)
         *
         * @param bytes bytes to set in new TLV object
         * @param level
         * @return new TLV object initialized with bytes
         * @throws java.lang.IllegalArgumentException
         */
        static synchronized TLV createNewTlv(byte[] bytes, int startpos, int length, int level) throws IllegalArgumentException {
            TLV t = new TLV();
            setTLVTag(t, bytes, startpos, length, false);
            t.setLevel(level);
            return t;
        }

        /**
         * Sets the first TLV-object in the byte array
         *
         * @param tlv TLV to be set
         * @param bytes from the TLV is set
         * @throws java.lang.IllegalArgumentException
         */
        static synchronized void setExistingTag(TLV tlv, byte[] bytes, int startpos, int length) throws IllegalArgumentException {
            setTLVTag(tlv, bytes, startpos, length, false);
        }

        /////////////////// privates //////////////////////////////////////////
        static private byte[] setTLVTag(TLV tlvIn, byte[] origByteArray, int startpos, int length, boolean doReturn) throws IllegalArgumentException {
            if (tlvIn == null) {
                throw new IllegalArgumentException("Error: Can't set TLVTag, tag is null");
            }
            if (origByteArray == null) {
                throw new IllegalArgumentException("Error: ByteArray is null");
            }
            if (length < 2) {
                throw new IllegalArgumentException("Error: ByteArray length < 2");
            }

            int byteArrPosition = startpos;
            int temp = 0;
            boolean checkNext = true;
            int origLen = length;

            tlvIn.setTlvClass(getClass(origByteArray[startpos]));
            tlvIn.setIsCdo(getForm(origByteArray[startpos]));

            byte b = ((byte) (origByteArray[byteArrPosition] & 0x1F));    //get last 5 bits

            //set tag
            if (b == 0x1F) {  //check next byte
                while (checkNext) {
                    byteArrPosition++;
                    if (byteArrPosition > 4) {
                        //the fourth bytes most significant bit must be zero
                        throw new IllegalArgumentException("Error: Corrupt bytearray, Tag is more then 4 bytes long");
                    }
                    if (byteArrPosition >= origLen) {
                        throw new IllegalArgumentException("Error: Corrupt bytearray, Tag is longer then the hole array");
                    }
                    b = origByteArray[byteArrPosition];
                    if ((b & 0x80) == 0) {
                        checkNext = false;
                    }
                    temp = temp << 7;
                    temp += (byte) (b & 0x7F);
                }
                tlvIn.setTag(temp);
            } else {
                tlvIn.setTag(b);
            }

            //set length
            byteArrPosition++;
            if (byteArrPosition >= origLen) {
                throw new IllegalArgumentException("Error: Length byte missing(level:" + tlvIn.getLevel() + ", tag:" + tlvIn.getTag() + ")");
            }

            b = origByteArray[byteArrPosition];
            if ((b & 0x80) == 0) {
                tlvIn.setLength((int) b);
            } else {
                int lenLength = (byte) (b & 0x7F);
                if (lenLength > 4) {
                    throw new IllegalArgumentException("Error: Length too long, max 4 bytes(now: " + lenLength + ");(level:" + tlvIn.getLevel() + ", tag:" + tlvIn.getTag() + ")");
                }
                temp = 0 ^ (origByteArray[++byteArrPosition] & 0x000000FF);
                for (int i = 0; i < lenLength - 1; i++) {
                    temp = temp << 8;
                    temp = temp ^ (origByteArray[++byteArrPosition] & 0x000000FF);
                }
                tlvIn.setLength(temp);
            }

            //set value;
            byteArrPosition++;
            if (tlvIn.getLength() > 0) {
                if (origLen - byteArrPosition < tlvIn.getLength()) {
                    throw new IllegalArgumentException("Error: Value lengt is not valid(tag:" + tlvIn.getTag() + "); length: " + tlvIn.getLength() + "; real length: " + (origLen - byteArrPosition));
                }
                tlvIn.setValue(new byte[tlvIn.getLength()]);
                System.arraycopy(origByteArray, byteArrPosition, tlvIn.getValue(), 0, tlvIn.getLength());
            }

            //return remaining bytes
            if (doReturn) {
                byteArrPosition += tlvIn.getLength();
                temp = origLen - byteArrPosition;
                if (temp > 0) {
                    byte[] ret = new byte[temp];
                    System.arraycopy(origByteArray, byteArrPosition, ret, 0, temp);
                    return ret;
                } else {
                    return null;
                }
            } else {
                return null;
            }
        }

        static private short getClass(byte b) {
            return (short) ((0x000000FF & b) >> 6);
        }

        static private boolean getForm(byte b) {
            return (b & 0x20) != 0;
        }

        static private void setTagBytes(int startpos, int tagAndLen, int len, byte[] origByteArray) {
            origByteArray[startpos] = 0x00;
            for (int i = 0; i < len; i++) {
                origByteArray[i + startpos] = (byte) (tagAndLen >> ((len - i - 1) * 7));
                if (i + 1 < len) {
                    origByteArray[i + startpos] = (byte) (origByteArray[i + startpos] | 0x80);
                } else {
                    origByteArray[i + startpos] = (byte) (origByteArray[i + startpos] & 0x7f);
                }
            }
        }

        static private void setLengthBytes(int startpos, int tagAndLen, int len, byte[] origByteArray) {
            origByteArray[startpos] = 0x00;
            if (len == 0) {
                origByteArray[startpos] = (byte) tagAndLen;
            } else {
                origByteArray[startpos] = (byte) (len | 0x80);
                byte[] bb = new byte[4];
                for (int i = 0; i < 4; i++) {
                    bb[i] = (byte) ((tagAndLen >>> ((bb.length - 1 - i) * 8)) & 0xFF);
                }
                int bbIndex = 3;
                for (int i = len; i > 0; i--) {
                    origByteArray[startpos + i] = bb[bbIndex--];
                }
            }
        }

        /**
         * Returns number of bytes to use when storing the value
         *
         * @param value Tag or length value
         * @return number of bytes as integer
         * @throws java.lang.IllegalArgumentException
         */
        static private int getNrOfUsedTagBytes(int value) throws IllegalArgumentException {
            if (value < 0) {
                throw new IllegalArgumentException("Error: Tag or Length value < 0");
            } else if (value > Integer.MAX_VALUE) {
                throw new IllegalArgumentException("Error: Tag or Length value > Integer.MAX_VALUE(more then 4 bytes), value:" + value);
            } else if (value == 0) {
                return 1;
            }
            return (int) Math.floor((Math.log(value) / Math.log(2)) / 7) + 1;
        }

        static private int getNrOfUsedLenBytes(int value) throws IllegalArgumentException {
            if (value < 0) {
                throw new IllegalArgumentException("Error: Tag or Length value < 0");
            } else if (value > Integer.MAX_VALUE) {
                throw new IllegalArgumentException("Error: Tag or Length value > Integer.MAX_VALUE(more then 4 bytes), value:" + value);
            } else if (value >= 0 && value <= 127) {
                return 0;
            } else if (value > 127 && value <= 255) {
                return 1;
            } else if (value > 255 && value <= 65535) {
                return 2;
            } else if (value > 65535 && value <= 1048575) {
                return 3;
            } else {
                return 4;
            }
        }

        static private byte setTlvClass(byte firstByte, short tlvClass) {
            firstByte = (byte) (firstByte & 0x3f);
            return (byte) (firstByte | ((byte) (tlvClass << 6)));
        }

        static private byte setTlvForm(byte firstByte, boolean cdo) {
            firstByte = (byte) (firstByte & 0xdf);
            if (cdo) {
                firstByte = (byte) (firstByte | 0x20);
            }
            return firstByte;

        }

        static private byte setTagInFirstByte(byte firstByte, int tag) {
            firstByte = (byte) (firstByte & 0xE0);
            return (byte) (firstByte | tag);
        }

    }

}
