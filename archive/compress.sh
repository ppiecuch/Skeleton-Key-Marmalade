dzip='/Developer/Marmalade/6.4/tools/dzip/dzip'

echo "Compressing sound"
rm -rf sound && mkdir sound
cp ../data/*.raw sound/
rm sound.dz
$dzip sound.dcl

echo "Compressing gles1 textures"
if [ -x ../data-ram/data-gles1 ]; then
	rm -rf gles1 && mkdir gles1
	cp ../data-ram/data-gles1/*.bin gles1/
	rm gles1.dz
	$dzip gles1.dcl
fi

echo "Compressing ATITC textures"
if [ -x ./data-ram/data-gles1-atitc ]; then
	rm -rf gles1-atitc && mkdir gles1-atitc
	cp ../data-ram/data-gles1-atitc/*.bin gles1-atitc/
	rm gles1-atitc.dz
	$dzip gles1-atitc.dcl
fi
