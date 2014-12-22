package com.indigobio;

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
        msg.writeTo(System.out);
    }

    static <T> void verify(T actual, T expected) {
        if (!actual.equals(expected)) {
            System.err.println(String.format("expected: %s, actual: %s", expected, actual));
            System.exit(1);
        }
    }
}
