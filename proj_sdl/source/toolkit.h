#include <SDL.h>
#include <SDL_opengl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "MersenneTwister.h"
#include "AngelcodeFont.h"

#ifdef _MSC_VER
#pragma warning(disable:4244) 
#pragma warning(disable:4305) 
#endif

#define USE_BASIC_PARTICLES
#define USE_BASIC_POPUPS


// If you're going to render widgets to the same
// UI from different source files, you can avoid
// ID collisions by defining IMGUI_SRC_ID before
// this define block:
#ifdef IMGUI_SRC_ID
#define GEN_ID ((IMGUI_SRC_ID) + (__LINE__))
#else
#define GEN_ID (__LINE__)
#endif


struct UIState
{
	int mousex;
	int mousey;
	int mousedown;

	int hotitem;
	int activeitem;

	int kbditem;
	int keyentered;
	int keymod;
	int keychar;
	
	int lastwidget;
};

extern UIState gUIState;

#ifdef USE_BASIC_PARTICLES
struct Particle
{
    int live;
    float x, y;
    float dirx, diry;  
    int tex;
};

#ifndef MAX_PARTICLES
#define MAX_PARTICLES 2000
#endif

extern Particle particle[];

#endif

#ifdef USE_BASIC_POPUPS
struct Popup
{
    int live;
    float x, y;
    char * text;
};

#ifndef MAX_POPUPS
#define MAX_POPUPS 16
#endif

extern Popup popup[];

#endif

enum keystates
{
    KEY_UP    = 1,
    KEY_DOWN  = 2,
    KEY_LEFT  = 4,
    KEY_RIGHT = 8,
    KEY_FIRE  = 16
};

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

extern MersenneTwister gVisualRand, gPhysicsRand;

GLuint load_texture(char * aFilename, int clamp = 1);
void drawrect(float x, float y, float w, float h, int color);
void quickfont_drawchar(int ch, float x, float y, float w, float h);
void quickfont_drawchar(int ch, float x, float y, float w, float h, int color);
void quickfont_drawstring(int tex, char * string, float x, float y, int color, float alpha, float size = 1.0);
void drawsprite(int tex, float x, float y, float w, float h, float alpha = 0.95f);
void spawn_popup(char * text, float x, float y);
void spawn_particle(float x, float y, int tex);
void physics_popups();
void reset_popups();
void reset_particles();

int imgui_button(int id, ACFont &font, char *text, float x, float y, float w, float h, int base, int face, int hot, int textcolor);
int imgui_slider(int id, float x, float y, float w, float h, int bg, int thumb, int hot, int max, int &value);
int imgui_textfield(int id, ACFont &font, int x, int y, int w, int h, char *buffer, int maxlen, int base, int face, int hot, int textcolor);
void imgui_prepare();
void imgui_finish();

const char *resourceRoot ();
const char *resourcePath (const char *filename);
const char *writePath (const char *file);
