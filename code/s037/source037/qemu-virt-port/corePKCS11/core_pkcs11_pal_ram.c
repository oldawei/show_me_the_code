/*
 * corePKCS11 v3.6.1
 * Copyright (C) 2024 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file core_pkcs11_pal.c
 * @brief Linux file save and read implementation
 * for PKCS #11 based on mbedTLS with for software keys. This
 * file deviates from the FreeRTOS style standard for some function names and
 * data types in order to maintain compliance with the PKCS #11 standard.
 */
/*-----------------------------------------------------------*/

/* PKCS 11 includes. */
#include "core_pkcs11_config.h"
#include "core_pkcs11_config_defaults.h"
#include "core_pkcs11.h"
#include "core_pkcs11_pal_utils.h"

/* C runtime includes. */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*-----------------------------------------------------------*/
#define pkcs11palFILE_NAME_CLIENT_CERTIFICATE    "corePKCS11_Certificate.dat"       /**< The file name of the Certificate object. */
#define pkcs11palFILE_NAME_KEY                   "corePKCS11_Key.dat"               /**< The file name of the Key object. */
#define pkcs11palFILE_NAME_PUBLIC_KEY            "corePKCS11_PubKey.dat"            /**< The file name of the Public Key object. */
#define pkcs11palFILE_CODE_SIGN_PUBLIC_KEY       "corePKCS11_CodeSignKey.dat"       /**< The file name of the Code Sign Key object. */
#define pkcs11palFILE_HMAC_SECRET_KEY            "corePKCS11_HMACKey.dat"           /**< The file name of the HMAC Secret Key object. */
#define pkcs11palFILE_CMAC_SECRET_KEY            "corePKCS11_CMACKey.dat"           /**< The file name of the CMAC Secret Key object. */
#define pkcs11palFILE_NAME_CLAIM_CERTIFICATE     "corePKCS11_Claim_Certificate.dat" /**< The file name of the Provisioning Claim Certificate object. */
#define pkcs11palFILE_NAME_CLAIM_KEY             "corePKCS11_Claim_Key.dat"         /**< The file name of the Provisioning Claim Key object. */

#define FILE_LEN    64
#define DATA_LEN    4096
typedef struct ram_file
{
    u16  file_size;
    u16  file_offt;
    char file_name[FILE_LEN];
    u8   file_data[DATA_LEN];
    /* data */
} ram_file_t;

ram_file_t ram_file_list[8] = { 0 };

FILE *ram_fopen(const char *filename, const char *mode)
{
    (void)mode;
    //LogInfo( ( "ram_fopen: filename %s", filename ) );
    for (int i = 0; i < 8; ++i) {
        if (strncmp(ram_file_list[i].file_name, filename, FILE_LEN-1) == 0) {
            ram_file_list[i].file_offt = 0;
            return (FILE *)(ram_file_list + i);
        }
    }
    return NULL;
}

size_t ram_fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    ram_file_t *pt = (ram_file_t *)stream;

    if (pt->file_offt >= DATA_LEN) {
        LogError( ( "fram_fread: ile_offt invalid" ) );
        return 0;
    }


    size_t copy_size = size * nmemb;
    size_t left_size = DATA_LEN - pt->file_offt;
    if ( left_size < copy_size )
        copy_size = left_size;

    //LogInfo( ( "fram_fread: copy_size %d", copy_size ) );
    memcpy(ptr, &pt->file_data[pt->file_offt], copy_size);
    pt->file_offt += copy_size;
    return copy_size;
}

size_t ram_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    ram_file_t *pt = (ram_file_t *)stream;

    if (pt->file_offt >= DATA_LEN) {
        LogError( ( "ram_fwrite: file_offt invalid" ) );
        return 0;
    }

    size_t copy_size = size * nmemb;
    size_t left_size = DATA_LEN - pt->file_offt;
    if ( left_size < copy_size )
        copy_size = left_size;

    //LogInfo( ( "ram_fwrite: copy_size %d", copy_size ) );
    memcpy(&pt->file_data[pt->file_offt], ptr, copy_size);
    pt->file_offt += copy_size;
    if (pt->file_offt > pt->file_size)
        pt->file_size = pt->file_offt;
    return copy_size;
}

int ram_fclose(FILE *stream)
{
    ram_file_t *pt = (ram_file_t *)stream;
    //LogInfo( ( "ram_fclose: filename %s", pt->file_name ) );
    pt->file_offt = 0;
    return 0;
}

long int ram_ftell(FILE *stream)
{
    ram_file_t *pt = (ram_file_t *)stream;
    //LogInfo( ( "ram_ftell: file_offt %d", pt->file_offt ) );
    return pt->file_offt;
}

int ram_fseek(FILE *stream, long int offset, int whence)
{
    ram_file_t *pt = (ram_file_t *)stream;

    //LogInfo( ( "ram_fseek1: file_size %d", pt->file_size ) );
    //LogInfo( ( "ram_fseek1: file_offt %d", pt->file_offt ) );
    switch (whence)
    {
    case SEEK_SET:
        pt->file_offt = offset;
        break;

    case SEEK_CUR:
        pt->file_offt += offset;
        break;
    case SEEK_END:
        pt->file_offt = pt->file_size + offset;
        break;

    default:
        LogDebug( ( "Unknow whence %d.", whence ) );
        break;
    }

    //LogInfo( ( "ram_fseek2: file_size %d", pt->file_size ) );
    //LogInfo( ( "ram_fseek2: file_offt %d", pt->file_offt ) );
    return 0;
}

int ram_remove(const char *filename)
{
    ram_file_t *pt = (ram_file_t *)ram_fopen(filename, "r");
    if (!pt)
        return 0;

    memset(pt, 0, sizeof(ram_file_t));
    return 0;
}

/**
 * @brief Checks to see if a file exists
 *
 * @param[in] pcFileName         The name of the file to check for existence.
 *
 * @returns CKR_OK if the file exists, CKR_OBJECT_HANDLE_INVALID if not.
 */
static CK_RV prvFileExists( const char * pcFileName )
{
    FILE * pxFile = NULL;
    CK_RV xReturn = CKR_OK;

    /* fopen returns NULL if the file does not exist. */
    pxFile = ram_fopen( pcFileName, "r" );

    if( pxFile == NULL )
    {
        xReturn = CKR_OBJECT_HANDLE_INVALID;
        LogDebug( ( "File %s does not exist or could not opened for reading.", pcFileName ) );
    }
    else
    {
        ( void ) ram_fclose( pxFile );
        LogDebug( ( "Found file %s and was able to open it for reading.", pcFileName ) );
    }

    return xReturn;
}

/**
 * @brief Reads object value from file system.
 *
 * @param[in] pcLabel            The PKCS #11 label to convert to a file name
 * @param[out] pcFileName        The name of the file to check for existence.
 * @param[out] pHandle           The type of the PKCS #11 object.
 *
 */
static CK_RV prvReadData( const char * pcFileName,
                          CK_BYTE_PTR * ppucData,
                          CK_ULONG_PTR pulDataSize )
{
    CK_RV xReturn = CKR_OK;
    FILE * pxFile = NULL;
    size_t lSize = 0;

    pxFile = ram_fopen( pcFileName, "r" );

    if( NULL == pxFile )
    {
        LogError( ( "PKCS #11 PAL failed to get object value. "
                    "Could not open file named %s for reading.", pcFileName ) );
        xReturn = CKR_FUNCTION_FAILED;
    }
    else
    {
        ( void ) ram_fseek( pxFile, 0, SEEK_END );
        lSize = ram_ftell( pxFile );
        ( void ) ram_fseek( pxFile, 0, SEEK_SET );

        if( lSize > 0UL )
        {
            *pulDataSize = lSize;
            *ppucData = pvPortMalloc( *pulDataSize );

            if( NULL == *ppucData )
            {
                LogError( ( "Could not get object value. Malloc failed to allocate memory." ) );
                xReturn = CKR_HOST_MEMORY;
            }
        }
        else
        {
            LogError( ( "Could not get object value. Failed to determine object size." ) );
            xReturn = CKR_FUNCTION_FAILED;
        }
    }

    if( CKR_OK == xReturn )
    {
        lSize = 0;
        lSize = ram_fread( *ppucData, sizeof( uint8_t ), *pulDataSize, pxFile );

        if( lSize != *pulDataSize )
        {
            LogError( ( "PKCS #11 PAL Failed to get object value. Expected to read %ld "
                        "from %s but received %ld", *pulDataSize, pcFileName, lSize ) );
            xReturn = CKR_FUNCTION_FAILED;
        }
    }

    if( NULL != pxFile )
    {
        ( void ) ram_fclose( pxFile );
    }

    return xReturn;
}

/*-----------------------------------------------------------*/

CK_RV PKCS11_PAL_Initialize( void )
{
    static int is_init_done = 0;
    if (is_init_done)
        return CKR_OK;

    const char *files[] = {
        pkcs11palFILE_NAME_CLIENT_CERTIFICATE,
        pkcs11palFILE_NAME_KEY,
        pkcs11palFILE_NAME_PUBLIC_KEY,
        pkcs11palFILE_CODE_SIGN_PUBLIC_KEY,
        pkcs11palFILE_HMAC_SECRET_KEY,
        pkcs11palFILE_CMAC_SECRET_KEY,
        pkcs11palFILE_NAME_CLAIM_CERTIFICATE,
        pkcs11palFILE_NAME_CLAIM_KEY
    };

    for (int i = 0; i < 8; ++i) {
        ram_file_list[i].file_size = 0;
        ram_file_list[i].file_offt = 0;
        strncpy(ram_file_list[i].file_name, files[i], FILE_LEN-1);
    }
    is_init_done = 1;

    return CKR_OK;
}

CK_OBJECT_HANDLE PKCS11_PAL_SaveObject( CK_ATTRIBUTE_PTR pxLabel,
                                        CK_BYTE_PTR pucData,
                                        CK_ULONG ulDataSize )
{
    FILE * pxFile = NULL;
    size_t ulBytesWritten;
    const char * pcFileName = NULL;
    CK_OBJECT_HANDLE xHandle = ( CK_OBJECT_HANDLE ) eInvalidHandle;

    if( ( pxLabel != NULL ) && ( pucData != NULL ) )
    {
        /* Converts a label to its respective filename and handle. */
        PAL_UTILS_LabelToFilenameHandle( pxLabel->pValue,
                                         &pcFileName,
                                         &xHandle );
    }
    else
    {
        LogError( ( "Could not save object. Received invalid parameters." ) );
    }

    if( pcFileName != NULL )
    {
        /* Overwrite the file every time it is saved. */
        pxFile = ram_fopen( pcFileName, "w" );

        if( NULL == pxFile )
        {
            LogError( ( "PKCS #11 PAL was unable to save object to file. "
                        "The PAL was unable to open a file with name %s in write mode.", pcFileName ) );
            xHandle = ( CK_OBJECT_HANDLE ) eInvalidHandle;
        }
        else
        {
            ulBytesWritten = ram_fwrite( pucData, sizeof( uint8_t ), ulDataSize, pxFile );

            if( ulBytesWritten != ulDataSize )
            {
                LogError( ( "PKCS #11 PAL was unable to save object to file. "
                            "Expected to write %lu bytes, but wrote %lu bytes.", ulDataSize, ulBytesWritten ) );
                xHandle = ( CK_OBJECT_HANDLE ) eInvalidHandle;
            }
            else
            {
                LogDebug( ( "Successfully wrote %lu to %s", ulBytesWritten, pcFileName ) );
            }
        }

        if( NULL != pxFile )
        {
            ( void ) ram_fclose( pxFile );
        }
    }
    else
    {
        LogError( ( "Could not save object. Unable to find the correct file." ) );
    }

    return xHandle;
}

/*-----------------------------------------------------------*/


CK_OBJECT_HANDLE PKCS11_PAL_FindObject( CK_BYTE_PTR pxLabel,
                                        CK_ULONG usLength )
{
    const char * pcFileName = NULL;
    CK_OBJECT_HANDLE xHandle = ( CK_OBJECT_HANDLE ) eInvalidHandle;

    ( void ) usLength;

    if( pxLabel != NULL )
    {
        PAL_UTILS_LabelToFilenameHandle( ( const char * ) pxLabel,
                                         &pcFileName,
                                         &xHandle );

        if( CKR_OK != prvFileExists( pcFileName ) )
        {
            xHandle = ( CK_OBJECT_HANDLE ) eInvalidHandle;
        }
    }
    else
    {
        LogError( ( "Could not find object. Received a NULL label." ) );
    }

    return xHandle;
}
/*-----------------------------------------------------------*/

CK_RV PKCS11_PAL_GetObjectValue( CK_OBJECT_HANDLE xHandle,
                                 CK_BYTE_PTR * ppucData,
                                 CK_ULONG_PTR pulDataSize,
                                 CK_BBOOL * pIsPrivate )
{
    CK_RV xReturn = CKR_OK;
    const char * pcFileName = NULL;


    if( ( ppucData == NULL ) || ( pulDataSize == NULL ) || ( pIsPrivate == NULL ) )
    {
        xReturn = CKR_ARGUMENTS_BAD;
        LogError( ( "Could not get object value. Received a NULL argument." ) );
    }
    else
    {
        xReturn = PAL_UTILS_HandleToFilename( xHandle, &pcFileName, pIsPrivate );
    }

    if( xReturn == CKR_OK )
    {
        xReturn = prvReadData( pcFileName, ppucData, pulDataSize );
    }

    return xReturn;
}

/*-----------------------------------------------------------*/

void PKCS11_PAL_GetObjectValueCleanup( CK_BYTE_PTR pucData,
                                       CK_ULONG ulDataSize )
{
    /* Unused parameters. */
    ( void ) ulDataSize;

    if( NULL != pucData )
    {
        vPortFree( pucData );
    }
}

/*-----------------------------------------------------------*/

CK_RV PKCS11_PAL_DestroyObject( CK_OBJECT_HANDLE xHandle )
{
    const char * pcFileName = NULL;
    CK_BBOOL xIsPrivate = CK_TRUE;
    CK_RV xResult = CKR_OBJECT_HANDLE_INVALID;
    int ret = 0;


    xResult = PAL_UTILS_HandleToFilename( xHandle,
                                          &pcFileName,
                                          &xIsPrivate );

    if( ( xResult == CKR_OK ) && ( prvFileExists( pcFileName ) == CKR_OK ) )
    {
        ret = ram_remove( pcFileName );

        if( ret != 0 )
        {
            xResult = CKR_FUNCTION_FAILED;
        }
    }

    return xResult;
}

/*-----------------------------------------------------------*/
