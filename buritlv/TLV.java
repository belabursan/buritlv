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

/**
 *
 * @author buri
 */
public final class TLV {
    
    private static final int MAX_VALUE_LENGTH = 65535;//32767*2;
    private static final int MAX_TAG_LENGTH = 127;//7F;
    
    private int tag;
    private int length;
    private byte[] value;
    
    private TLV next;
    private TLV child;
    
    private boolean isCdo;
    
    /**
     * Constructor
     * @param tag tag value, must be lower then 128 
     * @param value byte array, can be null and length must be lower then 65535
     * @param isCdo boolean, true if CDO, false otherwise
     * @throws IllegalArgumentException if tag or value length is greater then allowed
     */
    public TLV(int tag, byte[] value, boolean isCdo) throws IllegalArgumentException {
        if(tag > MAX_TAG_LENGTH)
            throw new IllegalArgumentException("Tag value greater then " + MAX_TAG_LENGTH);
        if(value != null && value.length > MAX_VALUE_LENGTH)
            throw new IllegalArgumentException("Value length greater then " + MAX_VALUE_LENGTH);
        this.isCdo = isCdo;
        this.tag = tag;
        this.value = value;
    }
    
    /**
     * Constructor
     * @param tag tag value, must be lower then 128 
     * @param value string, can be null and length must be lower then 65535
     * @param isCdo boolean, true if CDO, false otherwise
     * @throws IllegalArgumentException if tag or value length is greater then allowed
     */
    public TLV (int tag, String value, boolean isCdo) throws IllegalArgumentException {
        this (tag, TlvUtil.ToByteArray(value), isCdo);
    }
    
}

