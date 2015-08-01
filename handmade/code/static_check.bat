@echo off


set filetypes=*.h *.cpp *.c

echo "static" KEYWORD FOUND:
findstr /S /N /I /L "static" %filetypes%

echo .
echo ------------------
echo .

echo GLOBAL DEFINITION FOUND
findstr /S /N /I /L "local_persist" %filetypes%
findstr /S /N /I /L "global_variable" %filetypes%
