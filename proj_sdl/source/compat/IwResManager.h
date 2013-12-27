#ifndef __compat_iwresmanager_h__
#define __compat_iwresmanager_h__

#include <stdio.h>

void IwResManagerInit();
inline static void IwResManagerTerminate() { }

struct CIwResGroup
{
};

struct IwGetResManagerS;

static struct IwGetResManagerS 
{
  void LoadGroup(const char *grp) { }
  int GetNumGroups() { return 0; }
  CIwResGroup *GetGroup(int index) { return NULL; }
  void DestroyGroup(CIwResGroup *g) { }
  IwGetResManagerS *operator ()() { return this; }
} IwGetResManager;

#endif /* __compat_iwresmanager_h__ */
