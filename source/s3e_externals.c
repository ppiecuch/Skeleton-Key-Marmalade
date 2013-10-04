
#if defined(__arm__) || defined(arm) || defined(__ARM__) || defined(__ARM_NEON__)

// skip - arm build is fine

#else

// dummy definitions for incomplete x86 build for Windows Phone 8;
// since we build for mobile only, exclude only __arm__ builds.

void s3eTimerGetMs() {}
void s3eDebugIsDebuggerPresent() {}
void s3eDebugOutputString() {}
void s3ePointerGetInt() {}
void s3ePointerRegister() {}
void s3ePointerUnRegister() {}
void s3eConfigGetString() {}
void s3eDeviceYield() {}
void s3eDeviceRegister() {}
void s3eDeviceUnRegister() {}
void s3eDeviceCheckQuitRequest() {}
void s3eKeyboardUpdate() {}
void s3ePointerUpdate() {}
void s3eDebugAssertShow() {}
void s3eExtGetHash() {}
void s3eDeviceLoaderCallStart() {}
void s3eDeviceLoaderCallDone() {}
void s3eAccelerometerStart() {}
void s3eAccelerometerStop() {}
void s3eAccelerometerGetX() {}
void s3eAccelerometerGetY() {}
void s3eAccelerometerGetZ() {}
void s3eFileOpen() {}
void s3eFileClose() {}
void s3eFileRead() {}
void s3eFileGetSize() {}
void s3eMallocBase() {}
void __s3eMallocBase() {}
void s3eFreeBase() {}
void __s3eFreeBase() {}
void s3eSoundSetInt() {}
void s3eSoundGetErrorString() {}
void s3eSoundGetFreeChannel() {}
void s3eSoundChannelPlay() {}
void s3eAudioSetInt() {}
void s3eAudioPlay() {}
void s3eAudioStop() {}
void s3eAudioIsPlaying() {}
void s3eAudioIsCodecSupported() {}
void s3eFileSeek() {}
void s3eFileTell() {}
void s3eGLGetInt() {}
void s3eFileCheckExists() {}
void s3eConfigGetInt() {}
void s3eKeyboardGetState() {}
void s3eFileWrite() {}
void s3eSurfaceGetInt() {}
void s3eDeviceGetInt() {}
void s3eMemoryGetInt() {}
void s3eMemorySetInt() {}
void s3eMemoryHeapCreate() {}
void s3eFileGetFileInt() {}
void s3eSurfaceRegister() {}
void s3eSurfaceUnRegister() {}
void s3eSurfacePtr() {}
void s3eSurfaceInfo() {}
void s3eSurfaceSetup() {}
void s3eFileOpenFromMemory() {}
void s3ePointerGetX() {}
void s3ePointerGetY() {}
void s3eSurfaceShow() {}
void s3eFileGetChar() {}
void s3eReallocBase() {}
void __s3eReallocBase() {}
void s3eGLRegister() {}
void s3eGLUnRegister() {}
void s3eGLGetNativeWindow() {}
void s3eFileGetFileString() {}
void s3eFileMakeDirectory() {}
void s3eFileDelete() {}
void s3eFileAddUserFileSys() {}
void s3eCompressionDecompInit() {}
void s3eFile() {}
void s3eCompressionDecompRead() {}
void s3eCompressionDecompFinal() {}
void s3eFileFlush() {}
void s3eFileEOF() {}
void s3eDebugErrorShow() {}
void s3eTimerGetUTC() {}
void s3eDeviceGetString() {}
void s3eDeviceYieldUntilEvent() {}
void s3eDeviceExit() {}
void s3eFileGetError() {}
void s3eFileDeleteDirectory() {}
void s3eFileRename() {}
void s3eFileTruncate() {}
void s3eTimerGetLocaltimeOffset() {}
void s3eDebugSetInt() {}
void s3eDebugRegister() {}
void s3eDebugTraceLine() {}
void s3eDebugTraceChannelSwitch() {}
void s3eMemoryHeapAddress() {}
void s3eTimerGetUST() {}
void s3eSocketGetString() {}
void __s3eMemorySetUserMemMgr() {}
void s3eMemoryUsrMgr() {}
void s3eSocketGetError() {}
void s3eInetAddress() {}
void s3eInetNtohl() {}
void s3eInetHtonl() {}
void s3eInetNtohs() {}
void s3eInetHtons() {}
void s3eSocketCreate() {}
void s3eSocketClose() {}
void s3eSocketBind() {}
void s3eSocketGetLocalName() {}
void s3eSocketGetPeerName() {}
void s3eSocketConnect() {}
void s3eSocketSend() {}
void s3eSocketRecv() {}
void s3eSocketRecvFrom() {}
void s3eSocketSendTo() {}
void s3eSocketListen() {}
void s3eSocketAccept() {}
void s3eSocketReadable() {}
void s3eSocketWritable() {}
void s3eDebugGetInt() {}
void s3eDebugPrint() {}
void s3eKeyboardSetInt() {}
void s3eKeyboardRegister() {}
void s3eTimerSetTimer() {}
void s3eDeviceSetInt() {}
void s3eInetAton() {}
void s3eInetNtoa() {}
void s3eInetLookup() {}
void s3eFileListDirectory() {}
void s3eFileListNext() {}
void s3eFileListClose() {}
void s3eCompressionDecomp() {}

// --

void glGetString() {}
void __glGetIntegerv() {}
void __glGetError() {}
void __glClearColor() {}
void __glClear() {}
void __glColorMask() {}
void __glTexParameterf() {}
void __glTexParameterfv() {}
void __glActiveTexture() {}
void __glBindTexture() {}
void __glCompressedTexImage2D() {}
void __glCompressedTexSubImage2D() {}
void __glCopyTexImage2D() {}
void __glCopyTexSubImage2D() {}
void __glDeleteTextures() {}
void __glGenTextures() {}
void __glIsTexture() {}
void __glTexImage2D() {}
void __glTexParameteri() {}
void __glTexParameterx() {}
void __glTexParameteriv() {}
void __glTexParameterxv() {}
void __glTexSubImage2D() {}
void __glGenerateMipmap() {}
void __glAlphaFunc() {}
void __glClearDepthf() {}
void __glClipPlanef() {}
void __glColor4f() {}
void __glDepthRangef() {}
void __glFogf() {}
void __glFogfv() {}
void __glFrustumf() {}
void __glGetClipPlanef() {}
void __glGetLightfv() {}
void __glGetMaterialfv() {}
void __glGetTexEnvfv() {}
void __glGetTexParameterfv() {}
void __glLightModelf() {}
void __glLightModelfv() {}
void __glLightf() {}
void __glLightfv() {}
void __glLineWidth() {}
void __glMaterialf() {}
void __glMaterialfv() {}
void __glMultMatrixf() {}
void __glMultiTexCoord4f() {}
void __glNormal3f() {}
void __glOrthof() {}
void __glPointParameterf() {}
void __glPointParameterfv() {}
void __glPointSize() {}
void __glPolygonOffset() {}
void __glRotatef() {}
void __glScalef() {}
void __glTexEnvf() {}
void __glTexEnvfv() {}
void __glTranslatef() {}
void __glAlphaFuncx() {}
void __glBlendFunc() {}
void __glClearColorx() {}
void __glClearDepthx() {}
void __glClearStencil() {}
void __glClientActiveTexture() {}
void __glClipPlanex() {}
void __glColor4x() {}
void __glColorPointer() {}
void __glCullFace() {}
void __glDepthFunc() {}
void __glDepthMask() {}
void __glDepthRangex() {}
void __glDisable() {}
void __glDisableClientState() {}
void __glDrawArrays() {}
void __glDrawElements() {}
void __glEnable() {}
void __glEnableClientState() {}
void __glFinish() {}
void __glFlush() {}
void __glFogx() {}
void __glFogxv() {}
void __glFrontFace() {}
void __glFrustumx() {}
void __glGetClipPlanex() {}
void __glGetLightxv() {}
void __glGetMaterialxv() {}
void __glGetPointerv() {}
void __glGetTexEnviv() {}
void __glGetTexEnvxv() {}
void __glGetTexParameteriv() {}
void __glGetTexParameterxv() {}
void __glHint() {}
void __glIsEnabled() {}
void __glLightModelx() {}
void __glLightModelxv() {}
void __glLightx() {}
void __glLightxv() {}
void __glLineWidthx() {}
void __glLogicOp() {}
void __glMaterialx() {}
void __glMaterialxv() {}
void __glMultMatrixx() {}
void __glMultiTexCoord4x() {}
void __glNormal3x() {}
void __glNormalPointer() {}
void __glOrthox() {}
void __glPixelStorei() {}
void __glPointParameterx() {}
void __glPointParameterxv() {}
void __glPointSizex() {}
void __glPolygonOffsetx() {}
void __glPopMatrix() {}
void __glPushMatrix() {}
void __glReadPixels() {}
void __glRotatex() {}
void __glSampleCoveragex() {}
void __glScalex() {}
void __glShadeModel() {}
void __glStencilFunc() {}
void __glStencilMask() {}
void __glStencilOp() {}
void __glTexCoordPointer() {}
void __glTexEnvi() {}
void __glTexEnvx() {}
void __glTexEnviv() {}
void __glTexEnvxv() {}
void __glTranslatex() {}
void __glVertexPointer() {}
void __glPointSizePointerOES() {}
void __glBlendColor() {}
void __glBlendEquation() {}
void __glBlendEquationSeparate() {}
void __glBlendFuncSeparate() {}
void __glDisableVertexAttribArray() {}
void __glEnableVertexAttribArray() {}
void __glGetRenderbufferParameteriv() {}
void __glGetVertexAttribfv() {}
void __glGetVertexAttribiv() {}
void __glGetVertexAttribPointerv() {}
void __glReleaseShaderCompiler() {}
void __glStencilFuncSeparate() {}
void __glStencilMaskSeparate() {}
void __glStencilOpSeparate() {}
void __glVertexAttrib1f() {}
void __glVertexAttrib1fv() {}
void __glVertexAttrib2f() {}
void __glVertexAttrib2fv() {}
void __glVertexAttrib3f() {}
void __glVertexAttrib3fv() {}
void __glVertexAttrib4f() {}
void __glVertexAttrib4fv() {}
void __glVertexAttribPointer() {}
void __glGetFloatv() {}
void __glLoadMatrixf() {}
void __glLoadIdentity() {}
void __glLoadMatrixx() {}
void __glMatrixMode() {}
void __glScissor() {}
void __glViewport() {}
void __glGetBooleanv() {}
void __glGetFixedv() {}
void __glBindBuffer() {}
void __glBufferData() {}
void __glBufferSubData() {}
void __glDeleteBuffers() {}
void __glGetBufferParameteriv() {}
void __glGenBuffers() {}
void __glIsBuffer() {}
void __glAttachShader() {}
void __glBindAttribLocation() {}
void __glCompileShader() {}
void __glCreateProgram() {}
void __glCreateShader() {}
void __glDeleteProgram() {}
void __glDeleteShader() {}
void __glDetachShader() {}
void __glGetActiveAttrib() {}
void __glGetActiveUniform() {}
void __glGetAttachedShaders() {}
void __glGetAttribLocation() {}
void __glGetProgramiv() {}
void __glGetProgramInfoLog() {}
void __glGetShaderiv() {}
void __glGetShaderInfoLog() {}
void __glGetShaderPrecisionFormat() {}
void __glGetShaderSource() {}
void __glGetUniformfv() {}
void __glGetUniformiv() {}
void __glGetUniformLocation() {}
void __glIsProgram() {}
void __glIsShader() {}
void __glLinkProgram() {}
void __glShaderBinary() {}
void __glShaderSource() {}
void __glUniform1f() {}
void __glUniform1fv() {}
void __glUniform1i() {}
void __glUniform1iv() {}
void __glUniform2f() {}
void __glUniform2fv() {}
void __glUniform2i() {}
void __glUniform2iv() {}
void __glUniform3f() {}
void __glUniform3fv() {}
void __glUniform3i() {}
void __glUniform3iv() {}
void __glUniform4f() {}
void __glUniform4fv() {}
void __glUniform4i() {}
void __glUniform4iv() {}
void __glUniformMatrix2fv() {}
void __glUniformMatrix3fv() {}
void __glUniformMatrix4fv() {}
void __glUseProgram() {}
void __glValidateProgram() {}
void __glBindFramebuffer() {}
void __glBindRenderbuffer() {}
void __glCheckFramebufferStatus() {}
void __glDeleteFramebuffers() {}
void __glDeleteRenderbuffers() {}
void __glFramebufferRenderbuffer() {}
void __glFramebufferTexture2D() {}
void __glGenFramebuffers() {}
void __glGenRenderbuffers() {}
void __glGetFramebufferAttachmentParameteriv() {}
void __glIsFramebuffer() {}
void __glIsRenderbuffer() {}
void __glRenderbufferStorage() {}

void eglBindAPI() {}
void eglMakeCurrent() {}
void eglGetConfigAttrib() {}
void eglCreatePbufferSurface() {}
void eglDestroySurface() {}
void eglReleaseTexImage() {}
void eglCreateContext() {}
void eglDestroyContext() {}
void eglGetCurrentContext() {}
void eglGetError() {}
void eglGetDisplay() {}
void eglInitialize() {}
void eglTerminate() {}
void eglQueryString() {}
void eglCreateWindowSurface() {}
void eglGetCurrentSurface() {}
void eglGetCurrentDisplay() {}
void eglQueryContext() {}
void eglSwapBuffers() {}
void __eglGetProcAddress() {}
void eglGetConfigs() {}
void eglQuerySurface() {}
void __eglBindTexImage() {}

#endif
