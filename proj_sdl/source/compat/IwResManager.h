#ifndef __compat_iwresmanager_h__
#define __compat_iwresmanager_h__

void IwResManagerInit();
void IwResManagerTerminate();

struct CIwResGroup
{
};

struct IwGetResManagerS;

static struct IwGetResManagerS 
{
  void LoadGroup(const char *grp) { }
  int GetNumGroups();
  CIwResGroup *GetGroup(int index);
  void DestroyGroup(CIwResGroup *g) { }
  IwGetResManagerS *operator ()() { return this; }
} IwGetResManager;

#endif /* __compat_iwresmanager_h__ */
