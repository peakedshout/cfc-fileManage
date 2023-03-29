/* Code generated by cmd/cgo; DO NOT EDIT. */

/* package command-line-arguments */


#line 1 "cgo-builtin-export-prolog"

#include <stddef.h>

#ifndef GO_CGO_EXPORT_PROLOGUE_H
#define GO_CGO_EXPORT_PROLOGUE_H

#ifndef GO_CGO_GOSTRING_TYPEDEF
typedef struct { const char *p; ptrdiff_t n; } _GoString_;
#endif

#endif

/* Start of preamble from import "C" comments.  */


#line 3 "main.go"
#include <stdlib.h>
#include "./cfcfile_api2.h"

#line 1 "cgo-generated-wrapper"


/* End of preamble from import "C" comments.  */


/* Start of boilerplate cgo prologue.  */
#line 1 "cgo-gcc-export-header-prolog"

#ifndef GO_CGO_PROLOGUE_H
#define GO_CGO_PROLOGUE_H

typedef signed char GoInt8;
typedef unsigned char GoUint8;
typedef short GoInt16;
typedef unsigned short GoUint16;
typedef int GoInt32;
typedef unsigned int GoUint32;
typedef long long GoInt64;
typedef unsigned long long GoUint64;
typedef GoInt64 GoInt;
typedef GoUint64 GoUint;
typedef size_t GoUintptr;
typedef float GoFloat32;
typedef double GoFloat64;
#ifdef _MSC_VER
#include <complex.h>
typedef _Fcomplex GoComplex64;
typedef _Dcomplex GoComplex128;
#else
typedef float _Complex GoComplex64;
typedef double _Complex GoComplex128;
#endif

/*
  static assertion to make sure the file is being used on architecture
  at least with matching size of GoInt.
*/
typedef char _check_for_64_bit_pointer_matching_GoInt[sizeof(void*)==64/8 ? 1:-1];

#ifndef GO_CGO_GOSTRING_TYPEDEF
typedef _GoString_ GoString;
#endif
typedef void *GoMap;
typedef void *GoChan;
typedef struct { void *t; void *v; } GoInterface;
typedef struct { void *data; GoInt len; GoInt cap; } GoSlice;

#endif

/* End of boilerplate cgo prologue.  */

#ifdef __cplusplus
extern "C" {
#endif

extern __declspec(dllexport) char* InitCFC(GoString name, GoString addr, GoString key);
extern __declspec(dllexport) void Close(CFC_ClientContext_t c);
extern __declspec(dllexport) char* InitRemoteFileContext(CFC_ClientContext_t c, GoString name, GoString key);
extern __declspec(dllexport) char* OpenRemoteFileContext(CFC_FileContext_t fc, GoString path);
extern __declspec(dllexport) char* GetRemoteStatusFileContext(CFC_FileContext_t fc);
extern __declspec(dllexport) char* DownRemoteFileContextToNewTask(CFC_FileContext_t fc, GoString path, GoString out, GoUint8 tough, GoInt sizeBuff);
extern __declspec(dllexport) char* DownRemoteFileContextToStartDown(CFC_FileContext_t fc, GoString path, GoUint8 tough);
extern __declspec(dllexport) char* UpRemoteFileContextToNewTask(CFC_FileContext_t fc, GoString path, GoString out, GoUint8 tough, GoInt sizeBuff);
extern __declspec(dllexport) char* UpRemoteFileContextToStartUp(CFC_FileContext_t fc, GoString path, GoUint8 tough);
extern __declspec(dllexport) void TransmissionFileRemoteFileContextToStop(CFC_FileContext_t fc, GoString path);
extern __declspec(dllexport) char* TransmissionRemoteFileContextToGetProgress(CFC_FileContext_t fc, GoString path);
extern __declspec(dllexport) char* CopyRemoteFileContext(CFC_FileContext_t fc, GoString oldPath, GoString newPath, GoUint8 tough);
extern __declspec(dllexport) char* MoveRemoteFileContext(CFC_FileContext_t fc, GoString oldPath, GoString newPath, GoUint8 tough);
extern __declspec(dllexport) char* RenameRemoteFileContext(CFC_FileContext_t fc, GoString oldPath, GoString newPath, GoUint8 tough);
extern __declspec(dllexport) char* RemoveRemoteFileContext(CFC_FileContext_t fc, GoString path);
extern __declspec(dllexport) char* MkDirRemoteFileContext(CFC_FileContext_t fc, GoString path);
extern __declspec(dllexport) char* ReadRemoteFileContext(CFC_FileContext_t fc, GoString path, GoInt64 offset, GoInt limit, GoInt64 timeout);
extern __declspec(dllexport) char* ScanCFCInfoRemoteFileContext(CFC_FileContext_t fc, GoString localPath, GoUint8 ignoreErr);
extern __declspec(dllexport) char* InitLocalFileContext();
extern __declspec(dllexport) char* OpenLocalFileContext(CFC_FileContext_t fc, GoString path);
extern __declspec(dllexport) char* CopyLocalFileContext(CFC_FileContext_t fc, GoString oldPath, GoString newPath, GoUint8 tough);
extern __declspec(dllexport) char* MoveLocalFileContext(CFC_FileContext_t fc, GoString oldPath, GoString newPath, GoUint8 tough);
extern __declspec(dllexport) char* RenameLocalFileContext(CFC_FileContext_t fc, GoString oldPath, GoString newPath, GoUint8 tough);
extern __declspec(dllexport) char* RemoveLocalFileContext(CFC_FileContext_t fc, GoString path);
extern __declspec(dllexport) char* MkDirLocalFileContext(CFC_FileContext_t fc, GoString path);
extern __declspec(dllexport) char* ReadLocalFileContext(CFC_FileContext_t fc, GoString path, GoInt64 offset, GoInt limit);
extern __declspec(dllexport) void CloseFileContext(CFC_FileContext_t fc);
extern __declspec(dllexport) void FreeString(char* c);
extern __declspec(dllexport) char* LogListen(GoInt port);
extern __declspec(dllexport) void LogOpt(GoUint8 level, GoUint8 stack);

#ifdef __cplusplus
}
#endif
