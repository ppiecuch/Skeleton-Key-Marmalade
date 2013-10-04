
#if !defined(__arm__)

// dummy definitions for incomplete x86 build for Windows Phone 8;
// since we build for mobile only, exclude only __arm__ builds.

void s3eDebugIsDebuggerPresent() {}
void s3eFileClose() {}

#endif
