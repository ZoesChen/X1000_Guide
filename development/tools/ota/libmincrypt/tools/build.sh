##!/bin/sh
rm classes -rf
mkdir classes
javac DumpPublicKey.java -d classes
jar cvfm dumpkey.jar DumpPublicKey.mf -C classes .
rm classes -rf
