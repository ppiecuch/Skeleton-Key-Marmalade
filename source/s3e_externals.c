
#if !defined(__arm__)

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

#endif
