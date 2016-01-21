/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package tlvtest;

import com.buri.tlv.TLV;

/**
 *
 * @author belabursan
 */
public class TLVTestMain {

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        // TODO code application logic here
        TLV root = new TLV(123, TLV.CDO, TLV.CLASS_UNIVERSAL, null);
        System.out.println(root.toString());
        
        
        TLV firstChild = root.addChild(new TLV(2222, TLV.PDO, TLV.CLASS_UNIVERSAL, new byte[]{'d', 'f', 'g'}));
        firstChild.setNext(new TLV(6666, TLV.CDO, TLV.CLASS_CONTEXT_SPEC, null));
        root.addChild(new TLV(3333, TLV.PDO, TLV.CLASS_APPLICATION, new byte[]{'x', 'y', 'z'}));
        
        TLV childChild = root.findTag(6666);
        
        
        childChild.addChild(new TLV(444,TLV.CDO, TLV.CLASS_PRIVATE, null));
        childChild.addChild(new TLV(5555, TLV.PDO, TLV.CLASS_PRIVATE, new byte[]{'t', 't', 't', 't'}));
        childChild.addChild(new TLV(88, TLV.CLASS_PRIVATE, "bábébíböbåbä"));
        
        
        
        System.out.println(root.toString());
        byte[] b = root.convertToByteArray();
        System.out.println(javax.xml.bind.DatatypeConverter.printHexBinary(b));
        System.out.println("");
        
        TLV newTlv = new TLV(b, 0, b.length);
        
        System.out.println(newTlv.toString());
    }
    
}
