package com.indigobio;

import com.indigobio.test.Test;

/**
 * Hello world!
 *
 */
public class App 
{
    public static void main( String[] args ) throws Exception
    {
        Test.Everything msg = Test.Everything.parseFrom(System.in);
        msg.writeTo(System.out);
    }
}
