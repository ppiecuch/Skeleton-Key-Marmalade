set dzip="c:\Marmalade\6.4\tools\dzip\dzip"
@echo off

echo "Compressing sound"
rmdir sound /s/q
mkdir sound
copy ..\data\*.raw sound\
del sound.dz
%dzip% sound.dcl

if not exist ..\data-ram\data-gles1 goto atitc
echo "Compressing gles1 textures"
rmdir /s/q gles1\*.*
mkdir gles1
copy ..\data-ram\data-gles1\*.bin gles1\
del gles1.dz
%dzip% gles1.dcl

:atitc
if not exist ..\data-ram\data-gles1-atitc goto end
echo "Compressing ATITC textures"
rmdir /s/q gles1-atitc\*.*
mkdir gles1-atitc
copy ..\data-ram\data-gles1-atitc\*.bin gles1-atitc\
del gles1-atitc.dz
%dzip% gles1-atitc.dcl

:end
