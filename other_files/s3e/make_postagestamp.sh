#!/bin/bash
./postagestamp -d 30 -s 20 -b 30 -m center icon_512.png icons/icon_512_p.png
echo -n ".";convert icons/icon_512_p.png -geometry 86 icons/icon_86_p.png
echo -n ".";convert icons/icon_512_p.png -geometry 90 icons/icon_90_p.png
echo -n ".";convert icons/icon_512_p.png -geometry 114 icons/icon_114_p.png
echo -n ".";convert icons/icon_512_p.png -geometry 300 icons/icon_300_p.png
echo ""
