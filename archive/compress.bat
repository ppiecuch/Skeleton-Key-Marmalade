dzip=c:\Developer\Marmalade\6.4\tools\dzip\dzip
@echo off

echo "Compressing sound"
del /q sound\*.*
copy ..\data\*.raw sound\
del sound.dz
%dzip% sound.dcl

if not exist ..\data-ram\data-gles1 goto atitc
echo "Compressing gles1 textures"
del /q gles1\*.*
copy ..\data-ram\data-gles1\*.bin gles1\
del gles1.dz
%dzip% gles1.dcl

:atitc
if not exist ..\data-ram\data-gles1-atitc goto end
echo "Compressing ATITC textures"
del /q gles1-atitc\*.*
copy ..\data-ram\data-gles1-atitc\*.bin gles1-atitc\
del gles1-atitc.dz
%dzip% gles1-atitc.dcl

:end
