#ifndef __compat_iwresmanager_h__
#define __compat_iwresmanager_h__

void IwResManagerInit();
void IwResManagerTerminate();

struct IwGetResManagerS;

struct IwGetResManagerS {
  void LoadGroup(const char *grp);
  IwGetResManagerS *operator ()() { return this; }
} IwGetResManager;

#endif /* __compat_iwresmanager_h__ */
