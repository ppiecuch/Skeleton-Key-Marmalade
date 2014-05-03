#include <stdio.h>
#include <stdlib.h>
#include <SDL_opengl.h>
#include "toolkit.h"

static int readbyte(FILE * f)
{
    char i = 0;
    fread(&i,1,1,f);
    return i;
}

static int readword(FILE * f)
{
    short i = 0;
    fread(&i,2,1,f);
    return i;
}

static int readint(FILE * f)
{
    int i = 0;
    fread(&i,4,1,f);
    return i;
}

static void readchars(FILE * f, char * dst, int count)
{
    while (count)
    {
        *dst = readbyte(f);
        dst++;
        count--;
    }
}


void ACFontInfoBlock::load(FILE * f)
{
    int blocksize = readint(f) - 4;
    fontSize = readword(f);
    int db = readbyte(f);
    smooth  = !!(db & (1 << 7));
    unicode = !!(db & (1 << 6));
    italic  = !!(db & (1 << 5));
    bold    = !!(db & (1 << 4));
    charSet = readbyte(f);
    stretchH = readword(f);
    aa = readbyte(f);
    paddingUp = readbyte(f);
    paddingRight = readbyte(f);
    paddingDown = readbyte(f);
    paddingLeft = readbyte(f);
    spacingHoriz = readbyte(f);
    spacingVert = readbyte(f);
    outline = readbyte(f);
    int fontnamelen = blocksize - (2 + 1 + 1 + 2 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1);
    fontName = new char[fontnamelen];
    readchars(f, fontName, fontnamelen);    
}
ACFontInfoBlock::~ACFontInfoBlock()
{
    delete[] fontName;
}
ACFontInfoBlock::ACFontInfoBlock()
{
    fontSize = bold = italic = unicode = smooth = charSet = stretchH = aa = paddingUp = 
    paddingRight = paddingDown = spacingHoriz = spacingVert = outline = 0;
    fontName = 0;
}    

void ACFontCommonBlock::load(FILE * f)
{
    int blockSize = readint(f) - 4;
    lineHeight = readword(f);
    base = readword(f);
    scaleW = readword(f);
    scaleH = readword(f);
    pages = readword(f);
    int db = readbyte(f);
    packed  = !!(db & (1 << 0));
    encoded = !!(db & (1 << 1));
}
ACFontCommonBlock::ACFontCommonBlock()
{
    lineHeight = base = scaleW = scaleH = pages = packed = encoded = 0;
}

void ACFontPagesBlock::load(FILE * f)
{
    int blocksize = readint(f) - 4;
    int p = ftell(f);
    int l = 1;
    while (readbyte(f)) l++;
    fseek(f,p,SEEK_SET);
    pages = blocksize / l;
    int i;
    name = new char*[pages];
    glhandle = new int[pages];
    for (i = 0; i < pages; i++)
    {
        name[i] = new char[l];
        readchars(f, name[i], l);
        glhandle[i] = load_texture(name[i],1);
    }
}
ACFontPagesBlock::~ACFontPagesBlock()
{
    int i;
    for (i = 0; i < pages; i++)
        delete[] name[i];
    delete[] name;
    delete[] glhandle;
}
ACFontPagesBlock::ACFontPagesBlock()
{
    pages = 0;
    name = 0;
}

void ACFontCharBlock::load(FILE * f)
{
    id = readword(f);
    x = readword(f);
    y = readword(f);
    width = readword(f);
    height = readword(f);
    xoffset = readword(f);
    yoffset = readword(f);
    xadvance = readword(f);
    page = readbyte(f);
    channel = readbyte(f);
}

ACFontCharBlock::ACFontCharBlock()
{
    id = x = y = width = height = xoffset = yoffset = xadvance = page = channel = 0;
}

void ACFontCharsBlock::load(FILE * f)
{
    int blocksize = readint(f) - 4;
    charcount = blocksize / (2 * 8 + 1 + 1);
    chars = new ACFontCharBlock[charcount];
    int i;
    for (i = 0; i < charcount; i++)
        chars[i].load(f);
}

ACFontCharsBlock::ACFontCharsBlock()
{
    charcount = 0;
    chars = 0;
}

ACFontCharsBlock::~ACFontCharsBlock()
{
    delete[] chars;
}


void ACFontKerningPair::load(FILE * f)
{
    first = readword(f);
    second = readword(f);
    amount = readword(f);
}

ACFontKerningPair::ACFontKerningPair()
{
    first = second = amount = 0;
}    

void ACFontKerningPairsBlock::load(FILE * f)
{
    int blocksize = readint(f) - 4;
    kerningPairs = blocksize / (2 + 2 + 2);
    pair = new ACFontKerningPair[kerningPairs];
    int i;
    for (i = 0; i < kerningPairs; i++)
        pair[i].load(f);
}

ACFontKerningPairsBlock::~ACFontKerningPairsBlock()
{
    delete[] pair;
}

ACFontKerningPairsBlock::ACFontKerningPairsBlock()
{
    kerningPairs = 0;
    pair = 0;
}

void ACFont::load(const char *filename)
{
    FILE * f = fopen(filename, "rb");
    if (f)
    {
        load(f);
        fclose(f);
    }
}

void ACFont::load(FILE * f)
{
    if (f == NULL) return;
    if (readbyte(f) != 0x42) return; // B
    if (readbyte(f) != 0x4d) return; // M
    if (readbyte(f) != 0x46) return; // F
    if (readbyte(f) != 2) return;    // ver 2
    if (readbyte(f) != 1) return;
    info.load(f);
    if (readbyte(f) != 2) return;
    common.load(f);
    if (readbyte(f) != 3) return;
    pages.load(f);
    if (readbyte(f) != 4) return;
    chars.load(f);
    if (readbyte(f) != 5) return; // this is ok to fail
    kerning.load(f);         
}

ACFontCharBlock * ACFont::findcharblock(int ch)
{
    int i;
    for (i = 0; i < chars.charcount; i++)
    {
        if (chars.chars[i].id == ch)
            return &(chars.chars[i]);
    }
    return chars.chars;
}

int ACFont::findkern(int id1, int id2)
{
    int i;
    for (i = 0; i < kerning.kerningPairs; i++)
        if (kerning.pair[i].first == id1 && kerning.pair[i].second == id2)
            return kerning.pair[i].amount;
    return 0;
}


void ACFont::drawstring(const char * string, float x, float y, int color, float desired_ht)
{
    float scalefactor;
    if (desired_ht == 0.0f)
        scalefactor = 1.0f;
    else
        scalefactor = desired_ht / common.lineHeight;
    
    int currentpage = 0;
    if (pages.pages == 0)
        return;
    int len = strlen(string);
    float xofs, yofs;
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, pages.glhandle[0]);
#if 0
    glColor4f(((color >> 16) & 0xff) / 256.0f,
              ((color >> 8) & 0xff) / 256.0f,
              ((color >> 0) & 0xff) / 256.0f,
              ((color >> 24) & 0xff) / 256.0f);
    xofs = x;
    yofs = y;
    int lastid = 0;
    while (*string)
    {
        xofs += findkern(lastid, *string) * scalefactor;
        lastid = *string;
        if (*string == '\n')
        {
            xofs = x;
            yofs += common.lineHeight * scalefactor;
        }
        else
        {
            ACFontCharBlock *curr = findcharblock(*string);
            if (curr->page != currentpage)
            {
                currentpage = curr->page;
                glBindTexture(GL_TEXTURE_2D, pages.glhandle[currentpage]);
            }
        glBegin(GL_TRIANGLE_STRIP);
            glTexCoord2f(curr->x / (float)common.scaleW, curr->y / (float)common.scaleH);
            glVertex2f(xofs + curr->xoffset * scalefactor, yofs + curr->yoffset * scalefactor);

            glTexCoord2f(curr->x / (float)common.scaleW, (curr->y + curr->height) / (float)common.scaleH);
            glVertex2f(xofs + curr->xoffset * scalefactor, yofs + (curr->yoffset + curr->height) * scalefactor);

            glTexCoord2f((curr->x + curr->width) / (float)common.scaleW, curr->y / (float)common.scaleH);
            glVertex2f(xofs + (curr->xoffset + curr->width) * scalefactor, yofs + curr->yoffset * scalefactor);

            glTexCoord2f((curr->x + curr->width) / (float)common.scaleW, (curr->y + curr->height) / (float)common.scaleH);
            glVertex2f(xofs + (curr->xoffset + curr->width) * scalefactor, yofs + (curr->yoffset + curr->height) * scalefactor);
        glEnd();
            xofs += curr->xadvance * scalefactor;
        }
        string++;
    }
#else
    xofs = x;
    yofs = y;
    int lastid = 0;
    while (*string)
    {
        xofs += findkern(lastid, *string) * scalefactor;
        lastid = *string;
        if (*string == '\n')
        {
            xofs = x;
            yofs += common.lineHeight * scalefactor;
        }
        else
        {
            ACFontCharBlock *curr = findcharblock(*string);
            if (curr->page != currentpage)
            {
                currentpage = curr->page;
                glBindTexture(GL_TEXTURE_2D, pages.glhandle[currentpage]);
            }
	    struct _v2c4 {
	      GLfloat v[2];
	      GLfloat t[2];
	      uint32_t c;
	    } vertices[] = {
	      { {xofs + curr->xoffset * scalefactor, yofs + curr->yoffset * scalefactor}, {curr->x / (float)common.scaleW, curr->y / (float)common.scaleH}, color },
	      { {xofs + curr->xoffset * scalefactor, yofs + (curr->yoffset + curr->height) * scalefactor}, {curr->x / (float)common.scaleW, (curr->y + curr->height)}, color },
		{ {xofs + (curr->xoffset + curr->width) * scalefactor, yofs + curr->yoffset * scalefactor}, {(curr->x + curr->width) / (float)common.scaleW, curr->y / (float)common.scaleH}, color },
		{ {xofs + (curr->xoffset + curr->width) * scalefactor, yofs + (curr->yoffset + curr->height) * scalefactor}, {(curr->x + curr->width) / (float)common.scaleW, (curr->y + curr->height) / (float)common.scaleH}, color },
	      };
	      glVertexPointer(2, GL_FLOAT, sizeof(_v2c4), vertices->v);
	      glTexCoordPointer(2, GL_FLOAT, sizeof(_v2c4), vertices->t);
	      glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(_v2c4), &vertices->c);
	      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	      xofs += curr->xadvance * scalefactor;
        }
        string++;
    }
#endif
    glDisable(GL_TEXTURE_2D);
}

void ACFont::stringmetrics(const char * string, float &w, float &h, float desired_ht)
{
    float scalefactor;
    if (desired_ht == 0.0f)
        scalefactor = 1.0f;
    else
        scalefactor = desired_ht / common.lineHeight;

    if (pages.pages == 0)
    {
        w = h = 0;
        return;
    }
    float maxx = 0;
    int len = strlen(string);
    float xofs, yofs;
    xofs = 0;
    yofs = 0;
    int lastid = 0;
    while (*string)
    {
        xofs += findkern(lastid, *string) * scalefactor;
        lastid = *string;
        if (*string == '\n')
        {
            xofs = 0;
            yofs += common.lineHeight * scalefactor;
        }
        else
        {
            ACFontCharBlock *curr = findcharblock(*string);
            xofs += curr->xadvance * scalefactor;
        }
        if (maxx < xofs) maxx = xofs;
        string++;
    }
    w = maxx;
    h = yofs + common.lineHeight * scalefactor;
}
