package com.indigobio;

import com.google.protobuf.ByteString;
import com.indigobio.test.Test;

public class App
{
    public static void main( String[] args ) throws Exception
    {
        Test.Everything msg = Test.Everything.parseFrom(System.in);
        verify(msg.getFInt32(),  0x7fffffff);
        verify(msg.getFInt64(),  0x7fffffffffffffffL);
        verify(msg.getFUint32(), 0xffffffff);
        verify(msg.getFUint64(), 0xffffffffffffffffL);
        verify(msg.getFSint32(), 0x7fffffff);
        verify(msg.getFSint64(), 0x7fffffffffffffffL);
        verify(msg.getFBool(), true);
        verify(msg.getFEnum(), Test.Everything.State.ACTIVE);
        verify(msg.getFFixed64(), 0xffffffffffffffffL);
        verify(msg.getFSfixed64(), 0x7fffffffffffffffL);
        verify(msg.getFDouble(), 3.141592d);
        verify(msg.getFString(), "hello");

        ByteString fbytes = msg.getFBytes();
        verify(fbytes.byteAt(0), (byte)0);
        verify(fbytes.byteAt(1), (byte)1);
        verify(fbytes.byteAt(2), (byte)2);
        verify(fbytes.byteAt(3), (byte)3);
        verify(fbytes.byteAt(4), (byte)255);

        verify(msg.getFEmbedded().getA(), "goodbye");

        verify(msg.getFPackedCount(), 3);
        verify(msg.getFPacked(0), 1);
        verify(msg.getFPacked(1), 1000);
        verify(msg.getFPacked(2), 1000000);

        System.err.println("pbtest verification passed");

        msg.writeTo(System.out);
    }

    static <T> void verify(T actual, T expected) {
        if (!actual.equals(expected)) {
            System.err.println(String.format("pbtest expected: %s, actual: %s", expected, actual));
            System.exit(1);
        }
    }
}
