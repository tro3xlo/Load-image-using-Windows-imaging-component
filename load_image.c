#ifndef LOAD_IMAGE_C
#define LOAD_IMAGE_C

#define INITGUID
#define COBJMACROS
#include <wincodec.h>//link ole32.lib
#include <shlwapi.h>//link shlwapi.lib
#include <stdio.h>//printf, can be removed
#include <stdlib.h>//calloc
#include <stdbool.h>

void load_image ( unsigned * width, unsigned * height, unsigned char * * data, char * file_name ) {
 IWICImagingFactory * factory = NULL;
 IStream * stream = NULL;
 IWICBitmapDecoder * decoder = NULL; 
 IWICBitmapFrameDecode * frame = NULL;
 IWICFormatConverter * converter = NULL;
 bool COM_in_use = false;
 * width = 0;
 * height = 0;
 ( * data ) = NULL;

 if ( ! width ) {
  printf ( "load_image:\tno width address provided\n" );
  return;
 }

 if ( ! height ) {
  printf ( "load_image:\tno height address provided\n" );
  return;
 }

 if ( ! ( * data ) ) {
  printf ( "load_image:\tno image buffer address provided\n" );
  return;
 }

 if ( ! file_name ) {
  printf ( "load_image:\tno file name provided\n" );
  return;
 }

 switch ( CoInitialize ( NULL ) ) {
  case S_OK: COM_in_use = false; break;
  case S_FALSE: COM_in_use = true; break;//prevents COM from being uninitialized if it is used elsewhere
  //case E_INVALIDARG: printf ( "load_image:\tinvalid argument for CoInitialize\n" ); return;//hard coded, nothing but NULL is passed to CoInitialize
  //case RPC_E_CHANGED_MODE: printf ( "load_image:\tCoInitialize failed\n" ); return;//??? same as S_FALSE?
  case E_OUTOFMEMORY: printf ( "load_image:\tCoInitialize failed to allocate necessary memory(E_OUTOFMEMORY)\n" ); return;
  case E_UNEXPECTED: printf ( "load_image:\tCoInitialize failed unexpectedly(E_UNEXPECTED)\n" ); return;
 }

 switch ( CoCreateInstance ( & CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, & IID_IWICImagingFactory, ( LPVOID * ) & factory ) ) {
  case REGDB_E_CLASSNOTREG: printf ( "load_image:\tCoCreateInstance failed due to unregistered class(REGDB_E_CLASSNOTREG)\n" ); goto load_image_cleanup;
  case CLASS_E_NOAGGREGATION: printf ( "load_image:\tCoCreateInstance failed as the class cannot be aggregated(CLASS_E_NOAGGREGATION)\n" ); goto load_image_cleanup;
  //case E_NOINTERFACE: printf ( "load_image:\tCoCreateInstance failed due to unimplemented interface(E_NOINTERFACE)\n" ); goto load_image_cleanup;//highly unlikely the interface is missing unless custom interface
  //case E_POINTER: printf ( "load_image:\tCoCreateInstance failed due to factory pointer being NULL(E_POINTER)\n" ); goto load_image_cleanup;//hard coded unless factory is passed as function parameter
 }

 if ( SHCreateStreamOnFileA ( file_name, STGM_READ, & stream ) ) {
  printf ( "load_image:\tSHCreateStreamOnFileA failed\n" );
  goto load_image_cleanup;
 }

 if ( IWICImagingFactory_CreateDecoderFromStream ( factory, stream, NULL, WICDecodeMetadataCacheOnDemand, & decoder ) ) {
  printf ( "load_image:\tIWICImagingFactory_CreateDecoderFromStream failed\n" );
  goto load_image_cleanup;
 }

 if ( IWICBitmapDecoder_GetFrame ( decoder, 0, & frame ) ) {
  printf ( "load_image:\tIWICBitmapDecoder_GetFrame failed\n" );
  goto load_image_cleanup;
 }

 if ( IWICImagingFactory_CreateFormatConverter ( factory, & converter ) ) {
  printf ( "load_image:\tIWICImagingFactory_CreateFormatConverter failed\n" );
  goto load_image_cleanup;
 }

 if ( IWICFormatConverter_Initialize ( converter, ( IWICBitmapSource * ) frame, & GUID_WICPixelFormat32bppBGRA, WICBitmapDitherTypeNone, NULL, 0.0d, WICBitmapPaletteTypeCustom ) ) {
  printf ( "load_image:\tIWICFormatConverter_Initialize failed\n" );
  goto load_image_cleanup;
 }

 if ( IWICBitmapFrameDecode_GetSize ( frame, width, height ) ) {
  printf ( "load_image:\tIWICBitmapFrameDecode_GetSize failed\n" );
  goto load_image_cleanup;
 }

 if ( * width == 0 ) {
  printf ( "load_image:\timage width is zero\n" );
  * height = 0;
  goto load_image_cleanup;
 }

 if ( * height == 0 ) {
  printf ( "load_image:\timage height is zero\n" );
  * width = 0;
  goto load_image_cleanup;
 }

 if ( ! ( ( * data ) = ( BYTE * ) calloc ( 1, * width * * height * 4 ) ) ) {
  printf ( "load_image:\tcalloc failed\n" );
  * height = 0;
  * width = 0;
  goto load_image_cleanup;
 }

 if ( IWICFormatConverter_CopyPixels ( converter, NULL, * width * 4, * width * * height * 4, ( * data ) ) ) {
  printf ( "load_image:\tIWICFormatConverter_CopyPixels failed\n" );
  * height = 0;
  * width = 0;
  ( * data ) = ( free ( ( * data ) ), NULL );
 }

 if ( ( * data ) ) printf ( "load_image:\tsuccess\n" );

 load_image_cleanup:
 if ( factory ) IUnknown_Release ( factory );
 if ( stream ) IUnknown_Release ( stream );
 if ( decoder ) IUnknown_Release ( decoder );
 if ( frame ) IUnknown_Release ( frame );
 if ( converter ) IUnknown_Release ( converter );
 if ( ! COM_in_use ) CoUninitialize ();
}

#endif
