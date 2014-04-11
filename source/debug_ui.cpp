
#include "debug_ui.h"
#include "ig2d/ig_distorter.h"

#ifdef DEUG_UI

#include "Turs2DebugPanel.h"

static Turs2DebugPanel *_dbg = NULL;

void init_debug_ui()
{
  _dbg = new Turs2DebugPanel(IGDistorter::getInstance()->screenWidth, IGDistorter::getInstance()->screenHeight);
}

bool handle_debug_ui_event(uint32_t x, uint32_t y)
{
  return false; 
}

void display_debug_ui()
{
  if (_dbg == NULL)
    init_debug_ui();
  _dbg->render();
}

#else

void init_debug_ui() { }
bool handle_debug_ui_event(uint32_t x, uint32_t y) { return false; }
void display_debug_ui() { }

#endif
