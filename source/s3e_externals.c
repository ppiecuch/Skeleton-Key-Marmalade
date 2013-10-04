
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
void s3eFreeBase() {}
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
void s3eMemorySetUserMemMgr() {}
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

void _glGetString() {}
void _glGetIntegerv() {}
void _glGetError() {}
void _glClearColor() {}
void _glClear() {}
void _glColorMask() {}
void _glTexParameterf() {}
void _glTexParameterfv() {}
void _glActiveTexture() {}
void _glBindTexture() {}
void _glCompressedTexImage2D() {}
void _glCompressedTexSubImage2D() {}
void _glCopyTexImage2D() {}
void _glCopyTexSubImage2D() {}
void _glDeleteTextures() {}
void _glGenTextures() {}
void _glIsTexture() {}
void _glTexImage2D() {}
void _glTexParameteri() {}
void _glTexParameterx() {}
void _glTexParameteriv() {}
void _glTexParameterxv() {}
void _glTexSubImage2D() {}
void _glGenerateMipmap() {}
void _glAlphaFunc() {}
void _glClearDepthf() {}
void _glClipPlanef() {}
void _glColor4f() {}
void _glDepthRangef() {}
void _glFogf() {}
void _glFogfv() {}
void _glFrustumf() {}
void _glGetClipPlanef() {}
void _glGetLightfv() {}
void _glGetMaterialfv() {}
void _glGetTexEnvfv() {}
void _glGetTexParameterfv() {}
void _glLightModelf() {}
void _glLightModelfv() {}
void _glLightf() {}
void _glLightfv() {}
void _glLineWidth() {}
void _glMaterialf() {}
void _glMaterialfv() {}
void _glMultMatrixf() {}
void _glMultiTexCoord4f() {}
void _glNormal3f() {}
void _glOrthof() {}
void _glPointParameterf() {}
void _glPointParameterfv() {}
void _glPointSize() {}
void _glPolygonOffset() {}
void _glRotatef() {}
void _glScalef() {}
void _glTexEnvf() {}
void _glTexEnvfv() {}
void _glTranslatef() {}
void _glAlphaFuncx() {}
void _glBlendFunc() {}
void _glClearColorx() {}
void _glClearDepthx() {}
void _glClearStencil() {}
void _glClientActiveTexture() {}
void _glClipPlanex() {}
void _glColor4x() {}
void _glColorPointer() {}
void _glCullFace() {}
void _glDepthFunc() {}
void _glDepthMask() {}
void _glDepthRangex() {}
void _glDisable() {}
void _glDisableClientState() {}
void _glDrawArrays() {}
void _glDrawElements() {}
void _glEnable() {}
void _glEnableClientState() {}
void _glFinish() {}
void _glFlush() {}
void _glFogx() {}
void _glFogxv() {}
void _glFrontFace() {}
void _glFrustumx() {}
void _glGetClipPlanex() {}
void _glGetLightxv() {}
void _glGetMaterialxv() {}
void _glGetPointerv() {}
void _glGetTexEnviv() {}
void _glGetTexEnvxv() {}
void _glGetTexParameteriv() {}
void _glGetTexParameterxv() {}
void _glHint() {}
void _glIsEnabled() {}
void _glLightModelx() {}
void _glLightModelxv() {}
void _glLightx() {}
void _glLightxv() {}
void _glLineWidthx() {}
void _glLogicOp() {}
void _glMaterialx() {}
void _glMaterialxv() {}
void _glMultMatrixx() {}
void _glMultiTexCoord4x() {}
void _glNormal3x() {}
void _glNormalPointer() {}
void _glOrthox() {}
void _glPixelStorei() {}
void _glPointParameterx() {}
void _glPointParameterxv() {}
void _glPointSizex() {}
void _glPolygonOffsetx() {}
void _glPopMatrix() {}
void _glPushMatrix() {}
void _glReadPixels() {}
void _glRotatex() {}
void _glSampleCoveragex() {}
void _glScalex() {}
void _glShadeModel() {}
void _glStencilFunc() {}
void _glStencilMask() {}
void _glStencilOp() {}
void _glTexCoordPointer() {}
void _glTexEnvi() {}
void _glTexEnvx() {}
void _glTexEnviv() {}
void _glTexEnvxv() {}
void _glTranslatex() {}
void _glVertexPointer() {}
void _glPointSizePointerOES() {}
void _glBlendColor() {}
void _glBlendEquation() {}
void _glBlendEquationSeparate() {}
void _glBlendFuncSeparate() {}
void _glDisableVertexAttribArray() {}
void _glEnableVertexAttribArray() {}
void _glGetRenderbufferParameteriv() {}
void _glGetVertexAttribfv() {}
void _glGetVertexAttribiv() {}
void _glGetVertexAttribPointerv() {}
void _glReleaseShaderCompiler() {}
void _glStencilFuncSeparate() {}
void _glStencilMaskSeparate() {}
void _glStencilOpSeparate() {}
void _glVertexAttrib1f() {}
void _glVertexAttrib1fv() {}
void _glVertexAttrib2f() {}
void _glVertexAttrib2fv() {}
void _glVertexAttrib3f() {}
void _glVertexAttrib3fv() {}
void _glVertexAttrib4f() {}
void _glVertexAttrib4fv() {}
void _glVertexAttribPointer() {}
void _glGetFloatv() {}
void _glLoadMatrixf() {}
void _glLoadIdentity() {}
void _glLoadMatrixx() {}
void _glMatrixMode() {}
void _glScissor() {}
void _glViewport() {}
void _glGetBooleanv() {}
void _glGetFixedv() {}
void _glBindBuffer() {}
void _glBufferData() {}
void _glBufferSubData() {}
void _glDeleteBuffers() {}
void _glGetBufferParameteriv() {}
void _glGenBuffers() {}
void _glIsBuffer() {}
void _glAttachShader() {}
void _glBindAttribLocation() {}
void _glCompileShader() {}
void _glCreateProgram() {}
void _glCreateShader() {}
void _glDeleteProgram() {}
void _glDeleteShader() {}
void _glDetachShader() {}
void _glGetActiveAttrib() {}
void _glGetActiveUniform() {}
void _glGetAttachedShaders() {}
void _glGetAttribLocation() {}
void _glGetProgramiv() {}
void _glGetProgramInfoLog() {}
void _glGetShaderiv() {}
void _glGetShaderInfoLog() {}
void _glGetShaderPrecisionFormat() {}
void _glGetShaderSource() {}
void _glGetUniformfv() {}
void _glGetUniformiv() {}
void _glGetUniformLocation() {}
void _glIsProgram() {}
void _glIsShader() {}
void _glLinkProgram() {}
void _glShaderBinary() {}
void _glShaderSource() {}
void _glUniform1f() {}
void _glUniform1fv() {}
void _glUniform1i() {}
void _glUniform1iv() {}
void _glUniform2f() {}
void _glUniform2fv() {}
void _glUniform2i() {}
void _glUniform2iv() {}
void _glUniform3f() {}
void _glUniform3fv() {}
void _glUniform3i() {}
void _glUniform3iv() {}
void _glUniform4f() {}
void _glUniform4fv() {}
void _glUniform4i() {}
void _glUniform4iv() {}
void _glUniformMatrix2fv() {}
void _glUniformMatrix3fv() {}
void _glUniformMatrix4fv() {}
void _glUseProgram() {}
void _glValidateProgram() {}
void _glBindFramebuffer() {}
void _glBindRenderbuffer() {}
void _glCheckFramebufferStatus() {}
void _glDeleteFramebuffers() {}
void _glDeleteRenderbuffers() {}
void _glFramebufferRenderbuffer() {}
void _glFramebufferTexture2D() {}
void _glGenFramebuffers() {}
void _glGenRenderbuffers() {}
void _glGetFramebufferAttachmentParameteriv() {}
void _glIsFramebuffer() {}
void _glIsRenderbuffer() {}
void _glRenderbufferStorage() {}

#endif
