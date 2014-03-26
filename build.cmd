@echo off

echo Building xtype library...
cd xtype\src
make install
emxomf \emx\lib\xtype.a
cd ..\..

echo Building Quoteriser...
make bin
echo Done.
