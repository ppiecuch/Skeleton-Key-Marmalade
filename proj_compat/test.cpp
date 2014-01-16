#include "source/compat/AffineTransform.h"
#include "source/compat/TileSetUtility.h"
#include "source/stb/stb_image.h"

typedef affinetransform::AffineTransformT<float> AffineTransform;

int main() {

  AffineTransform m,m1,m2,m3,m4;
  
  m.identity();
  m.rotate(1,50,100);
  m.description();
  printf("-----\n");
  m1.identity();
  m1.translate(-50,-100);
  m2.identity();
  m2.rotate(1);
  m3.identity();
  m3.translate(50,100);
  m=m1*m2*m3;
  m.description();

  // --
  
  int width, height, channels;
  const int force_channels = 0;

  const char *path = "../data/fonts/font_deutsch_26.tga";

  // try to read raw data:
  unsigned char *idata = stbi_load( path, &width, &height, &channels, force_channels );
  // font is in alpha channel
  // bounds are in green channel
  if( idata == NULL ) {
    fprintf(stderr, "Failed to get raw data from the file %s - image not supported or not an image (%s).\n", 
	    path, stbi_failure_reason());
    return 1;
  }

  printf("\nGot: %dx%d, channels: %d\n", width, height, channels);
  if (channels == 4) {
    BufferedImage img(idata, width, height);
    ArrayRegions rc = TileSetUtilityRGBA::inferNumberColumns(img);
    for(int x=0; x<rc.size(); ++x) {
      printf("%d) %d - %d\n", x, rc[x].first, rc[x].second);
    }
    ArrayRegions rr = TileSetUtilityRGBA::inferNumberRows(img);
    for(int x=0; x<rr.size(); ++x) {
      printf("%d) %d - %d\n", x, rr[x].first, rr[x].second);
    }
    std::vector<TileCoord> tiles = TileSetUtilityRGBA::getTiles(img, rc, rr);
    printf("Got %lu tiles info.\n", tiles.size());
  }

  return 0;
}
