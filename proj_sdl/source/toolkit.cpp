#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <math.h>
#if 0
# include "glee/glee.h"
#endif
#include "toolkit.h"

#ifdef USE_BASIC_POPUPS
Popup popup[MAX_POPUPS];
#endif

#ifdef USE_BASIC_PARTICLES
Particle particle[MAX_PARTICLES];
#endif

MersenneTwister gVisualRand, gPhysicsRand;
UIState gUIState = {0,0,0,0,0,0,0,0,0,0};


GLuint load_texture(char * aFilename, int clamp)
{
    // Create OpenGL texture handle and bind it to use
    GLuint texname;
    glGenTextures(1,&texname);
    glBindTexture(GL_TEXTURE_2D,texname);

    // Load texture using SDL_Image
    SDL_Surface *temp = IMG_Load(aFilename);

    // Set up opengl-compatible pixel format
    SDL_PixelFormat pf;
    pf.palette = NULL;
    pf.BitsPerPixel = 32;
    pf.BytesPerPixel = 4;
    pf.alpha = 0;
    pf.colorkey = 0;
    pf.Rmask = 0x000000ff;
    pf.Rshift = 0;
    pf.Rloss = 0;
    pf.Gmask = 0x0000ff00;
    pf.Gshift = 8;
    pf.Gloss = 0;
    pf.Bmask = 0x00ff0000;
    pf.Bshift = 16;
    pf.Bloss = 0;
    pf.Amask = 0xff000000;
    pf.Ashift = 24;
    pf.Aloss = 0;

    // Convert texture to said format
    SDL_Surface *tm = SDL_ConvertSurface(temp, &pf,SDL_SWSURFACE);

    // Cleanup
    SDL_FreeSurface(temp);

    // Lock the converted surface for reading
    SDL_LockSurface(tm);

    int w,h,l;
    w = tm->w;
    h = tm->h;
    l = 0;
    unsigned int * mip = new unsigned int[w * h * 5];
    unsigned int * src = (unsigned int *)tm->pixels;

    memset(mip,0,tm->w*tm->h*4);

    // mark all pixels with alpha = 0 to black
    int i, j;
    for (i = 0; i < h; i++)
    {
        for (j = 0; j < w; j++)
        {
            if ((src[i*w+j] & 0xff000000) == 0)
                src[i*w+j] = 0;
        }
    }


    // Tell OpenGL to read the texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tm->w, tm->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)src);

    if (mip)
    {
        // precalculate summed area tables
        // it's a box filter, which isn't very good, but at least it's fast =)
        int ra = 0, ga = 0, ba = 0, aa = 0;
        int i, j, c;
        unsigned int * rbuf = mip + (tm->w * tm->h * 1);
        unsigned int * gbuf = mip + (tm->w * tm->h * 2);
        unsigned int * bbuf = mip + (tm->w * tm->h * 3);
        unsigned int * abuf = mip + (tm->w * tm->h * 4);
        
        for (j = 0, c = 0; j < tm->h; j++)
        {
            ra = ga = ba = aa = 0;
            for (i = 0; i < tm->w; i++, c++)
            {
                ra += (src[c] >>  0) & 0xff;
                ga += (src[c] >>  8) & 0xff;
                ba += (src[c] >> 16) & 0xff;
                aa += (src[c] >> 24) & 0xff;
                if (j == 0)
                {
                    rbuf[c] = ra;
                    gbuf[c] = ga;
                    bbuf[c] = ba;
                    abuf[c] = aa;
                }
                else
                {
                    rbuf[c] = ra + rbuf[c - tm->w];
                    gbuf[c] = ga + gbuf[c - tm->w];
                    bbuf[c] = ba + bbuf[c - tm->w];
                    abuf[c] = aa + abuf[c - tm->w];
                }
            }
        }

        while (w > 1 || h > 1)
        {
            l++;
            w /= 2;
            h /= 2;
            if (w == 0) w = 1;
            if (h == 0) h = 1;

            int dw = tm->w / w;
            int dh = tm->h / h;

            for (j = 0, c = 0; j < h; j++)
            {
                for (i = 0; i < w; i++, c++)
                {
                    int x1 = i * dw;
                    int y1 = j * dh;
                    int x2 = x1 + dw - 1;
                    int y2 = y1 + dh - 1;
                    int div = (x2 - x1) * (y2 - y1);
                    y1 *= tm->w;
                    y2 *= tm->w;
                    int r = rbuf[y2 + x2] - rbuf[y1 + x2] - rbuf[y2 + x1] + rbuf[y1 + x1];
                    int g = gbuf[y2 + x2] - gbuf[y1 + x2] - gbuf[y2 + x1] + gbuf[y1 + x1];
                    int b = bbuf[y2 + x2] - bbuf[y1 + x2] - bbuf[y2 + x1] + bbuf[y1 + x1];
                    int a = abuf[y2 + x2] - abuf[y1 + x2] - abuf[y2 + x1] + abuf[y1 + x1];

                    r /= div;
                    g /= div;
                    b /= div;
                    a /= div;

                    if (a == 0)
                        mip[c] = 0;
                    else
                        mip[c] = ((r & 0xff) <<  0) | 
                                 ((g & 0xff) <<  8) | 
                                 ((b & 0xff) << 16) | 
                                 ((a & 0xff) << 24); 
                }
            }
            glTexImage2D(GL_TEXTURE_2D, l, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)mip);
        }
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR); // Linear Filtering
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering
        delete[] mip;
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering
    }

    // Unlock..
    SDL_UnlockSurface(tm);

    // and cleanup.
    SDL_FreeSurface(tm);


    if (clamp)
    {
        // Set up texture parameters
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    }
    else
    {
        // Set up texture parameters
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }


    return texname;
}

void drawrect(float x, float y, float w, float h, int color)
{
#if 0
    glColor4f(((color >> 16) & 0xff) / 256.0f,
              ((color >> 8) & 0xff) / 256.0f,
              ((color >> 0) & 0xff) / 256.0f,
              ((color >> 24) & 0xff) / 256.0f);
    glBegin(GL_TRIANGLE_STRIP);
      glVertex2f(x,y);
      glVertex2f(x,y+h);
      glVertex2f(x+w,y);
      glVertex2f(x+w,y+h);
    glEnd();
#else 
    struct _v2c4 {
      GLfloat v[2];
      uint32_t c;
    } vertices[] = {
      { {x, y}, color },
      { {x,y+h}, color },
      { {x+w,y}, color },
      { {x+w,y+h}, color },
    };
    glVertexPointer(2, GL_FLOAT, sizeof(_v2c4), vertices->v);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(_v2c4), &vertices->c);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#endif
}

void quickfont_drawchar(int ch, float x, float y, float w, float h)
{
    // font = 6 rows, 16 chars per row
    float uofs = (ch & 15) * (1.0f / 16.0f) + 0.002f;
    float vofs = ((ch - 32) / 16) * (1.0f / 6.0f) + 0.002f;
    float uwid = (1.0f / 16.0f) - 0.004f;
    float vwid = (1.0f / 6.0f) - 0.004f;
#if 0
    glBegin(GL_TRIANGLE_STRIP);
      glTexCoord2f(uofs, vofs);
      glVertex2f(x, y);
      glTexCoord2f(uofs, vofs + vwid);
      glVertex2f(x, y + h);
      glTexCoord2f(uofs + uwid, vofs);
      glVertex2f(x + w, y);
      glTexCoord2f(uofs + uwid, vofs + vwid);
      glVertex2f(x + w, y + h);
    glEnd();
#else 
    struct _v2c4 {
      GLfloat v[2];
      GLfloat t[2];
    } vertices[] = {
      { {x, y}, {uofs, vofs} },
      { {x,y+h}, {uofs, vofs + vwid} },
      { {x+w,y}, {uofs + uwid, vofs} },
      { {x+w,y+h}, {uofs + uwid, vofs + vwid} },
    };
    glVertexPointer(2, GL_FLOAT, sizeof(_v2c4), vertices->v);
    glTexCoordPointer(2, GL_FLOAT, sizeof(_v2c4), vertices->t);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#endif
}

void quickfont_drawchar(int ch, float x, float y, float w, float h, int color)
{
    // font = 6 rows, 16 chars per row
    float uofs = (ch & 15) * (1.0f / 16.0f) + 0.002f;
    float vofs = ((ch - 32) / 16) * (1.0f / 6.0f) + 0.002f;
    float uwid = (1.0f / 16.0f) - 0.004f;
    float vwid = (1.0f / 6.0f) - 0.004f;
#if 0
    glBegin(GL_TRIANGLE_STRIP);
      glTexCoord2f(uofs, vofs);
      glVertex2f(x, y);
      glTexCoord2f(uofs, vofs + vwid);
      glVertex2f(x, y + h);
      glTexCoord2f(uofs + uwid, vofs);
      glVertex2f(x + w, y);
      glTexCoord2f(uofs + uwid, vofs + vwid);
      glVertex2f(x + w, y + h);
    glEnd();
#else 
    struct _v2c4 {
      GLfloat v[2];
      GLfloat t[2];
      uint32_t c;
    } vertices[] = {
      { {x, y}, {uofs, vofs}, color },
      { {x,y+h}, {uofs, vofs + vwid}, color },
      { {x+w,y}, {uofs + uwid, vofs}, color },
      { {x+w,y+h}, {uofs + uwid, vofs + vwid}, color },
    };
    glVertexPointer(2, GL_FLOAT, sizeof(_v2c4), vertices->v);
    glTexCoordPointer(2, GL_FLOAT, sizeof(_v2c4), vertices->t);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(_v2c4), &vertices->c);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#endif
}

void quickfont_drawstring(int tex, char * string, float x, float y, int color, float alpha, float size)
{
    glBindTexture(GL_TEXTURE_2D, tex);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
#if 0
    glColor4f(((color >> 16) & 0xff) / 256.0f,
              ((color >> 8) & 0xff) / 256.0f,
              ((color >> 0) & 0xff) / 256.0f,
              alpha);
    while (*string)
    {
        quickfont_drawchar(*string, x, y, 8 * size, 16 * size);
        string++;
        x += 8 * size;
    }
#else
    while (*string)
    {
      quickfont_drawchar(*string, x, y, 8 * size, 16 * size, color);
        string++;
        x += 8 * size;
    }
#endif
    glDisable(GL_TEXTURE_2D);
}

void drawsprite(int tex, float x, float y, float w, float h, float alpha)
{
    glBindTexture(GL_TEXTURE_2D, tex);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
#if 0
    glColor4f(1.0f,1.0f,1.0f,alpha);
    glBegin(GL_TRIANGLE_STRIP);
      glTexCoord2f(0,0);
      glVertex2f(x - w / 2,y - h / 2);
      glTexCoord2f(0,1);
      glVertex2f(x - w / 2,y + h / 2);
      glTexCoord2f(1,0);
      glVertex2f(x + w / 2,y - h / 2);
      glTexCoord2f(1,1);
      glVertex2f(x + w / 2,y + h / 2);
    glEnd();
#else 
    uint32_t color = (((uint8_t)(alpha*255.)) << 24 ) | 0x00ffffff;
    struct _v2c4 {
      GLfloat v[2];
      GLfloat t[2];
      uint32_t c;
    } vertices[] = {
      { {x, y}, {0, 0}, color },
      { {x,y+h}, {0, 1}, color },
      { {x+w,y}, {1, 0}, color },
      { {x+w,y+h}, {1, 1}, color },
    };
    glVertexPointer(2, GL_FLOAT, sizeof(_v2c4), vertices->v);
    glTexCoordPointer(2, GL_FLOAT, sizeof(_v2c4), vertices->t);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(_v2c4), &vertices->c);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#endif
    glDisable(GL_TEXTURE_2D);
}

#ifdef USE_BASIC_POPUPS
void spawn_popup(char * text, float x, float y)
{
    int i = 0;
    while (i < MAX_POPUPS && popup[i].live) i++;
    if (i >= MAX_POPUPS) return;
    popup[i].live = 100;
    popup[i].text = strdup(text); // strdup - needs to be free():d
    popup[i].y = y;
    popup[i].x = x - strlen(text) * 4;
}

void physics_popups()
{
    int j;
    for (j = 0; j < MAX_POPUPS; j++)
    {
        if (popup[j].live)
        {
            popup[j].live--;
            popup[j].y -= 0.05f;
            if (!popup[j].live)
            {
                free(popup[j].text);
                popup[j].text = 0;
            }
        }
    }
}

void reset_popups()
{
    int i;
    for (i = 0; i < MAX_POPUPS; i++)
    {
        popup[i].live = 0;
        free(popup[i].text);
        popup[i].text = 0;
    }
}
#endif

#ifdef USE_BASIC_PARTICLES
void spawn_particle(float x, float y, int tex)
{
    int i = gVisualRand.genrand_int31() % MAX_PARTICLES;
    if (particle[i].live)
        return;
    particle[i].live = 100 + gVisualRand.genrand_int31() % 100;
    particle[i].x = x;
    particle[i].y = y;
    float r = (gVisualRand.genrand_int31() % 1000) / 1000.0f;
    float d = (gVisualRand.genrand_int31() % 1000) / 1000.0f;
    particle[i].dirx = sin(r * 2 * M_PI) * 0.25 * (1 + d);
    particle[i].diry = -cos(r * 2 * M_PI) * 0.25 * (1 + d);    
    particle[i].tex = tex;
}

void reset_particles()
{
    int i;
    for (i = 0; i < MAX_PARTICLES; i++)
        particle[i].live = 0;
}
#endif

///////////////////////////////////////////////////
// imgui stuff

// Check whether current mouse position is within a rectangle
static int regionhit(int x, int y, int w, int h)
{
	if (gUIState.mousex < x ||
		gUIState.mousey < y ||
		gUIState.mousex >= x + w ||
		gUIState.mousey >= y + h)
		return 0;
	return 1;
}

// Simple button IMGUI widget
int imgui_button(int id, ACFont &font, char *text, float x, float y, float w, float h, int base, int face, int hot, int textcolor)
{
    float th, tw;
    font.stringmetrics(text,tw,th);
	// Check whether the button should be hot
	if (regionhit(x, y, w, h))
	{
		gUIState.hotitem = id;
		if (gUIState.activeitem == 0 && gUIState.mousedown)
			gUIState.activeitem = id;
	}

	// If no widget has keyboard focus, take it
	if (gUIState.kbditem == 0)
		gUIState.kbditem = id;

    drawrect(x,y,w,h,base);

    //font.drawstring(text, x+2, y+2, 0xff000000);

    if (gUIState.kbditem == id || gUIState.hotitem == id)
    {
		if (gUIState.activeitem == id)
		{
			// Button is both 'hot' and 'active'
            drawrect(x+3,y+3,w-4,h-4,hot);
            font.drawstring(text, x+(w-tw)/2+1,y+(h-th)/2+1,textcolor);
		}
		else
		{
            drawrect(x+2,y+2,w-4,h-4,hot);
			// Button is merely 'hot'
            font.drawstring(text, x+(w-tw)/2,y+(h-th)/2,textcolor);
		}
    }
    else
    {
        drawrect(x+2,y+2,w-4,h-4,face);
        font.drawstring(text, x+(w-tw)/2,y+(h-th)/2,textcolor);
    }

	// If we have keyboard focus, we'll need to process the keys
	if (gUIState.kbditem == id)
	{
		switch (gUIState.keyentered)
		{
		case SDLK_TAB:
			// If tab is pressed, lose keyboard focus.
			// Next widget will grab the focus.
			gUIState.kbditem = 0;
			// If shift was also pressed, we want to move focus
			// to the previous widget instead.
			if (gUIState.keymod & KMOD_SHIFT)
				gUIState.kbditem = gUIState.lastwidget;
			// Also clear the key so that next widget
			// won't process it
			gUIState.keyentered = 0;
			break;
		case SDLK_RETURN:
			// Had keyboard focus, received return,
			// so we'll act as if we were clicked.
			return 1;
		}
	}

	gUIState.lastwidget = id;

	// If button is hot and active, but mouse button is not
	// down, the user must have clicked the button.
	// Also give the widget keyboard focus
	if (gUIState.mousedown == 0 && 
		gUIState.hotitem == id && 
		gUIState.activeitem == id)
    {
		gUIState.kbditem = id;
		return 1;
    }

	// Otherwise, no clicky.
	return 0;
}

// Simple scroll bar IMGUI widget
int imgui_slider(int id, float x, float y, float w, float h, int bg, int thumb, int hot, int max, int &value)
{
    int thumbht = (int)h - max;
    if (thumbht < (w - 4)) thumbht = w - 4;
	// Calculate thumb's relative y offset
	int ypos = ((h - thumbht - 4) * value) / max + 2;

	// Check for hotness
	if (regionhit(x, y, w, h))
	{
		gUIState.hotitem = id;
		if (gUIState.activeitem == 0 && gUIState.mousedown)
			gUIState.activeitem = id;
	}

	// If no widget has keyboard focus, take it
	if (gUIState.kbditem == 0)
		gUIState.kbditem = id;

	// Render the scrollbar
	drawrect(x, y, w, h, bg);
	
	if (gUIState.activeitem == id || gUIState.hotitem == id || gUIState.kbditem == id)
	{
		drawrect(x+2, y + ypos, w - 4, thumbht, hot);
	}
	else
	{
		drawrect(x+2, y + ypos, w - 4, thumbht, thumb);
	}

	// If we have keyboard focus, we'll need to process the keys
	if (gUIState.kbditem == id)
	{
		switch (gUIState.keyentered)
		{
		case SDLK_TAB:
			// If tab is pressed, lose keyboard focus.
			// Next widget will grab the focus.
			gUIState.kbditem = 0;
			// If shift was also pressed, we want to move focus
			// to the previous widget instead.
			if (gUIState.keymod & KMOD_SHIFT)
				gUIState.kbditem = gUIState.lastwidget;
			// Also clear the key so that next widget
			// won't process it
			gUIState.keyentered = 0;
			break;
		case SDLK_UP:
			// Slide slider up (if not at zero)
			if (value > 0)
			{
				value--;
				return 1;
			}
			break;
		case SDLK_DOWN:
			// Slide slider down (if not at max)
			if (value < max)
			{
				value++;
				return 1;
			}
			break;
		}
	}

	gUIState.lastwidget = id;

	// Update widget value
	if (gUIState.activeitem == id)
	{
        gUIState.kbditem = id;
		int mousepos = gUIState.mousey - (y + 2);
		if (mousepos < 0) mousepos = 0;
		if (mousepos > h - 4) mousepos = h - 4;
		int v = (mousepos * max) / (h - 4);
		if (v != value)
		{
			value = v;
			return 1;
		}
	}

    // If button is hot and active, but mouse button is not
	// down, the user must have clicked the widget; give it 
	// keyboard focus.
	if (gUIState.mousedown == 0 && 
		gUIState.hotitem == id && 
		gUIState.activeitem == id)
		gUIState.kbditem = id;

	return 0;
}

int imgui_textfield(int id, ACFont &font, int x, int y, int w, int h, char *buffer, int maxlen, int base, int face, int hot, int textcolor)
{
	int len = strlen(buffer);
	int changed = 0;

	// Check for hotness
	if (regionhit(x, y, w, h))
	{
		gUIState.hotitem = id;
		if (gUIState.activeitem == 0 && gUIState.mousedown)
			gUIState.activeitem = id;
	}

	// If no widget has keyboard focus, take it
	if (gUIState.kbditem == 0)
		gUIState.kbditem = id;

    drawrect(x,y,w,h,base);
	// Render the text field
	if (gUIState.activeitem == id || gUIState.hotitem == id || gUIState.kbditem == id)
	{
		drawrect(x+2, y+2, w-4, h-4, hot);
	}
	else
	{
		drawrect(x+2, y+2, w-4, h-4, face);
	}

	font.drawstring(buffer,x+2,y+(h-font.common.lineHeight)/2,textcolor);

	// Render cursor if we have keyboard focus
	if (gUIState.kbditem == id && (SDL_GetTicks() >> 8) & 1)
    {
        float th,tw;
        font.stringmetrics(buffer,tw,th);
	font.drawstring("_",x + tw + 2, y + (h-font.common.lineHeight)/2,textcolor);
    }

	// If we have keyboard focus, we'll need to process the keys
	if (gUIState.kbditem == id)
	{
		switch (gUIState.keyentered)
		{
		case SDLK_TAB:
			// If tab is pressed, lose keyboard focus.
			// Next widget will grab the focus.
			gUIState.kbditem = 0;
			// If shift was also pressed, we want to move focus
			// to the previous widget instead.
			if (gUIState.keymod & KMOD_SHIFT)
				gUIState.kbditem = gUIState.lastwidget;
			// Also clear the key so that next widget
			// won't process it
			gUIState.keyentered = 0;
			break;
		case SDLK_BACKSPACE:
			if (len > 0)
			{
				len--;
				buffer[len] = 0;
				changed = 1;
			}
			break;			
		}
		if (gUIState.keychar >= 32 && gUIState.keychar < 127 && len < maxlen)
		{
			buffer[len] = gUIState.keychar;
			len++;
			buffer[len] = 0;
			changed = 1;
		}
	}

	// If button is hot and active, but mouse button is not
	// down, the user must have clicked the widget; give it 
	// keyboard focus.
	if (gUIState.mousedown == 0 && 
		gUIState.hotitem == id && 
		gUIState.activeitem == id)
		gUIState.kbditem = id;

	gUIState.lastwidget = id;

	return changed;
}

void imgui_prepare()
{
	gUIState.hotitem = 0;
}

void imgui_finish()
{
	if (gUIState.mousedown == 0)
	{
		gUIState.activeitem = 0;
	}
	else
	{
		if (gUIState.activeitem == 0)
			gUIState.activeitem = -1;
	}
	// If no widget grabbed tab, clear focus
	if (gUIState.keyentered == SDLK_TAB)
		gUIState.kbditem = 0;
	// Clear the entered key
	gUIState.keyentered = 0;	
	gUIState.keychar = 0;
}
