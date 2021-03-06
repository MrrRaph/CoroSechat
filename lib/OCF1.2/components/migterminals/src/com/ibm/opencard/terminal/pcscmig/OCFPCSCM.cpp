/*
 * Copyright ? 1997 - 1999 IBM Corporation.
 * 
 * Redistribution and use in source (source code) and binary (object code)
 * forms, with or without modification, are permitted provided that the
 * following conditions are met:
 * 1. Redistributed source code must retain the above copyright notice, this
 * list of conditions and the disclaimer below.
 * 2. Redistributed object code must reproduce the above copyright notice,
 * this list of conditions and the disclaimer below in the documentation
 * and/or other materials provided with the distribution.
 * 3. The name of IBM may not be used to endorse or promote products derived
 * from this software or in any other form without specific prior written
 * permission from IBM.
 * 4. Redistribution of any modified code must be labeled "Code derived from
 * the original OpenCard Framework".
 * 
 * THIS SOFTWARE IS PROVIDED BY IBM "AS IS" FREE OF CHARGE. IBM SHALL NOT BE
 * LIABLE FOR INFRINGEMENTS OF THIRD PARTIES RIGHTS BASED ON THIS SOFTWARE.  ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IBM DOES NOT WARRANT THAT THE FUNCTIONS CONTAINED IN THIS
 * SOFTWARE WILL MEET THE USER'S REQUIREMENTS OR THAT THE OPERATION OF IT WILL
 * BE UNINTERRUPTED OR ERROR-FREE.  IN NO EVENT, UNLESS REQUIRED BY APPLICABLE
 * LAW, SHALL IBM BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  ALSO, IBM IS UNDER NO OBLIGATION
 * TO MAINTAIN, CORRECT, UPDATE, CHANGE, MODIFY, OR OTHERWISE SUPPORT THIS
 * SOFTWARE.
 */

/*
 * Author:  Stephan Breideneich (sbreiden@de.ibm.com)
 * Version: $Id: OCFPCSCM.cpp,v 1.5 1998/09/21 16:14:43 cvsusers Exp $
 */

#include <stdio.h>
#include <windows.h>

#include "Tracer.h"
#include "PcscExceptions.h"
#include "PcscContexts.h"
#include "fieldIO.h"

#include "OCFPCSCM.h"

#include <res_mgr.h>


/*
 * Class:     com_ibm_opencard_terminal_pcscmig_OCFPCSC1
 * Method:    initTrace
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_ibm_opencard_terminal_pcscmig_OCFPCSCM_initTrace
    (JNIEnv *env, jobject obj) {

    initTrace(env, obj);
}

/*
 * Class:     com_ibm_opencard_terminal_pcscmig_OCFPCSC1
 * Method:    SCardEstablishContext
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_ibm_opencard_terminal_pcscmig_OCFPCSCM_SCardEstablishContext
  (JNIEnv *env, jobject obj, jint scope) {

    long context;
    long returnCode;

    returnCode = SCardEstablishContext((DWORD)scope, NULL, NULL, (SCARDCONTEXT *)&context);
    if (returnCode != SCARD_S_SUCCESS) {
  throwPcscException(env, obj, "SCardEstablishContext", "PC/SC Error SCardEstablishContext", returnCode);
  return 0;
    }

    /* add this context to the internal table
     * it's useful in the case the layer above didn't release the context
     * the Dll_Main is able to release all established contexts
     */
    addContext(context);

    return (jint)context;
}

/*
 * Class:     com_ibm_opencard_terminal_pcscmig_OCFPCSC1
 * Method:    SCardReleaseContext
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_ibm_opencard_terminal_pcscmig_OCFPCSCM_SCardReleaseContext
  (JNIEnv *env, jobject obj, jint context) {

    long returnCode;

    if (isContextAvailable((long)context) < 0) {
  throwPcscException(env, obj, "SCardReleaseContext", "tried to release a non-existing context",0);
  return;
    }

    /* delete the context from the internal table */
    removeContext((long)context);

    returnCode = SCardReleaseContext((SCARDCONTEXT)context);
    if (returnCode != SCARD_S_SUCCESS) {
  throwPcscException(env, obj, "SCardReleaseContext", "PC/SC Error SCardReleaseContext", returnCode);
  return;
    }
    return;
}

/*
 * Class:     com_ibm_opencard_terminal_pcscmig_OCFPCSC1
 * Method:    SCardConnect
 * Signature: (ILjava/lang/String;IILjava/lang/Integer;)I
 */
// jActiveProtocol not yet implemented
JNIEXPORT jint JNICALL Java_com_ibm_opencard_terminal_pcscmig_OCFPCSCM_SCardConnect
  (JNIEnv *env, jobject obj, jint context, jstring jReader,
   jint jShareMode, jint jPreferredProtocol, jobject jActiveProtocol) {

    const char  *readerUTF;
    long  cardHandle;
    DWORD       activeProtocol;

    long  returnCode;

    /* get the readers friendly name as 8bit code */
    readerUTF = env->GetStringUTFChars(jReader, NULL);

    /* get a connection to the card */
    returnCode = SCardConnect(  (SCARDCONTEXT)context,
        readerUTF,
        (DWORD)jShareMode,
        (DWORD)jPreferredProtocol,
        (LPSCARDHANDLE)&cardHandle,
        (DWORD *)&activeProtocol);

    /* release the readers friendly name */
    env->ReleaseStringUTFChars(jReader, readerUTF);

    if (returnCode != SCARD_S_SUCCESS) {
  throwPcscException(env, obj, "SCardConnect", "PC/SC Error SCardConnect", returnCode);
  return 0;
    }

    return cardHandle;
}

/*
 * Class:     com_ibm_opencard_terminal_pcscmig_OCFPCSC1
 * Method:    SCardReconnect
 * Signature: (IIIILjava/lang/Integer;)V
 */
// jActiveProtocol not yet implemented
JNIEXPORT void JNICALL Java_com_ibm_opencard_terminal_pcscmig_OCFPCSCM_SCardReconnect
  (JNIEnv *env, jobject obj, jint card, jint shareMode,
   jint preferredProtocols, jint initialization, jobject jActiveProtocoll) {

    long    returnCode;
    DWORD   activeProtocol;

    returnCode = SCardReconnect((SCARDHANDLE)card,
        (DWORD)shareMode,
        (DWORD)preferredProtocols,
        (DWORD)initialization,
        (LPDWORD)&activeProtocol);

    if (returnCode != SCARD_S_SUCCESS) {
  throwPcscException(env, obj, "SCardReconnect", "PC/SC Error SCardReconnect", returnCode);
  return;
    }

    return;
}

/*
 * Class:     com_ibm_opencard_terminal_pcscmig_OCFPCSC1
 * Method:    SCardDisconnect
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_com_ibm_opencard_terminal_pcscmig_OCFPCSCM_SCardDisconnect
  (JNIEnv *env, jobject obj, jint card, jint disposition) {

    long returnCode;

    returnCode = SCardDisconnect((SCARDHANDLE)card, (DWORD)disposition);

    if ((returnCode != SCARD_S_SUCCESS) && (returnCode != SCARD_W_REMOVED_CARD)) {
  throwPcscException(env, obj, "SCardDisconnect", "PC/SC Error SCardDisconnect", returnCode);
  return;
    }

    return;
}

/*
 * Class:     com_ibm_opencard_terminal_pcscmig_OCFPCSC1
 * Method:    SCardGetStatusChange
 * Signature: (II[Lcom/ibm/opencard/terminal/pcsc10/PcscReaderState;)V
 */
JNIEXPORT void JNICALL Java_com_ibm_opencard_terminal_pcscmig_OCFPCSCM_SCardGetStatusChange
  (JNIEnv *env, jobject obj, jint context, jint timeout, jobjectArray jReaderState) {

    SCARD_READERSTATE   *readerState;
    int     numReaderState;
    int     ii;
    long    returnCode;
    jobject   objReaderState;

    /* First access the PcscReaderState structure to initialize the return       */
    /* value. Allocate a reader state array for each java ReaderState structure. */

    numReaderState = env->GetArrayLength(jReaderState);
    if (numReaderState < 1) {
  throwPcscException(env, obj, "SCardGetStatusChange",
          "size of ReaderState array must be greater than 0 elements", 0);
  return;
    }

    readerState = (SCARD_READERSTATE *)malloc(numReaderState * sizeof(SCARD_READERSTATE));
    if (readerState == NULL) {
  throwPcscException(env, obj, "SCardGetStatusChange",
          "error allocating memory for the readerState buffer", 0);
  return;
    }

    /* clear the allocated memory */
    memset(readerState, 0x00, numReaderState * sizeof(SCARD_READERSTATE));

    /* Now get each Java reader state structure and translate it into C++ */
    for (ii=0; ii<numReaderState; ii++) {
  objReaderState = env->GetObjectArrayElement(jReaderState, ii);
  if (env->ExceptionOccurred() != NULL) {
      free(readerState);
      throwPcscException(env, obj, "SCardGetStatusChange",
          "error getting elements from the readerState array", 0);
      return;
  }

  // ToDo: in case of errors release the array elements

  returnCode = getIntField(env, objReaderState, "CurrentState", (long *)&readerState[ii].dwCurrentState);
  if (returnCode) {
      free(readerState);
      throwPcscException(env, obj, "SCardGetStatusChange",
          "error getting CurrentState field from the readerState record", 0);
      return;
  }

  returnCode = getIntField(env, objReaderState, "EventState", (long*)&readerState[ii].dwEventState);
  if (returnCode) {
      free(readerState);
      throwPcscException(env, obj, "SCardGetStatusChange",
          "error getting EventState field from the readerState record", 0);
      return;
  }


  readerState[ii].szReader = (char *)accessStringField(env, objReaderState, "Reader");
  if (readerState[ii].szReader == NULL) {
      free(readerState);
      throwPcscException(env, obj, "SCardGetStatusChange",
          "error getting Reader field from readerState record", 0);
      return;
  }

  int maxSize;
  returnCode = accessByteArray(env, objReaderState, "UserData", (unsigned char **)&readerState[ii].pvUserData, &maxSize);
  if (returnCode) {
      free(readerState);
      throwPcscException(env, obj, "SCardGetStatusChange",
          "error getting UserData field from readerState record", 0);
      return;
  }
    }



    /* set the response timeout to 1000ms */
    returnCode =  SCardGetStatusChange((SCARDCONTEXT)context, 1000, readerState, numReaderState);
    if (returnCode != SCARD_S_SUCCESS) {
  free(readerState);
  throwPcscException(env, obj, "SCardGetStatusChange",
          "error getting UserData field from readerState record", returnCode);
  return;
    }

    /* write back the informations from the readerStatus to the java structures */
    for (ii=0; ii<numReaderState; ii++) {

  objReaderState = env->GetObjectArrayElement(jReaderState, ii);
  if (env->ExceptionOccurred() != NULL) {
      free(readerState);
      throwPcscException(env, obj, "SCardGetStatusChange",
        "error getting array elements", returnCode);
      return;
  }

  returnCode = setIntField(env, objReaderState, "EventState", readerState[ii].dwEventState);
  if (returnCode) {
      free(readerState);
      throwPcscException(env, obj, "SCardGetStatusChange",
        "error setting the EventState field", returnCode);
      return;
  }

  returnCode = releaseStringField(env, objReaderState, "Reader", (const char *)readerState[ii].szReader);
  if (returnCode) {
      free(readerState);
      throwPcscException(env, obj, "SCardGetStatusChange",
        "error setting the Reader field", returnCode);
      return;
  }

  returnCode = releaseByteArray(env, objReaderState, "UserData", (unsigned char *)readerState[ii].pvUserData);
  if (returnCode) {
      free(readerState);
      throwPcscException(env, obj, "SCardGetStatusChange",
        "error setting the UserData", returnCode);
      return;
  }
    }

    free(readerState);

    return;
}

/*
 * Class:     com_ibm_opencard_terminal_pcscmig_OCFPCSC1
 * Method:    SCardGetAttrib
 * Signature: (II)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_ibm_opencard_terminal_pcscmig_OCFPCSCM_SCardGetAttrib
  (JNIEnv *env, jobject obj, jint card, jint attrId) {

    long  returnCode;
    DWORD       lenAttr;
    jbyte       attrArray[36];
    jbyteArray  jAttrArray;


    /* length of temp buffer */
    lenAttr = 36;

    /* get the attribute information from the reader */
    returnCode = SCardGetAttrib((SCARDHANDLE)card,
        (DWORD)attrId,
        (LPBYTE)attrArray,
        (LPDWORD)&lenAttr);
    if (returnCode != SCARD_S_SUCCESS) {
  throwPcscException(env, obj, "SCardGetAttrib",
         "error retrieving attribute data from reader", returnCode);
  return NULL;
    }

    /* allocate the jAttrArray with the returned length of the attribute data */
    jAttrArray = env->NewByteArray((jsize)lenAttr);
    if (jAttrArray == NULL) {
  throwPcscException(env, obj, "SCardGetAttrib",
         "error allocating the java bytearray for the attribute information", 0);
  return NULL;
    }

    /* copy the temp buffer into the java attribute array */
    env->SetByteArrayRegion(jAttrArray, (jsize)0, (jsize)lenAttr, attrArray);
    if (env->ExceptionOccurred() != NULL)
  return NULL;

    return jAttrArray;
}


/*
 * Class:     com_ibm_opencard_terminal_pcscmig_OCFPCSC1
 * Method:    SCardControl
 * Signature: (II[B)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_ibm_opencard_terminal_pcscmig_OCFPCSCM_SCardControl
  (JNIEnv *env, jobject obj, jint jCardHandle, jint jControlCode, jbyteArray jInBuffer) {

    LONG    returnCode;
    DWORD   lenInBuffer;
    DWORD   lenOutBuffer;
    DWORD   bytesReturned = 0;
    jbyte   *tmpInBuffer;     /* points to the java array*/
    jbyte   tmpOutBuffer[255];


    /* set the length of the buffers */
    lenInBuffer = env->GetArrayLength(jInBuffer);
    lenOutBuffer = 255;

    /* get the pointer to the internal buffer of the jInBuffer */
    if (lenInBuffer > 0)
  tmpInBuffer = env->GetByteArrayElements(jInBuffer, NULL);
    else
  tmpInBuffer = NULL;

    returnCode = SCardControl((SCARDHANDLE)jCardHandle,
            (DWORD)jControlCode,
            (LPCVOID)tmpInBuffer,
            (DWORD)lenInBuffer,
            (LPVOID)tmpOutBuffer,
            (DWORD)lenOutBuffer,
            &bytesReturned);

    if (returnCode != SCARD_S_SUCCESS) {
  throwPcscException(env, obj, "SCardControl",
         "error occured", returnCode);
  return NULL;
    }

    /* return buffer control to java vm without internal update */
    env->ReleaseByteArrayElements(jInBuffer, tmpInBuffer, JNI_ABORT);

    /* create java byte array for the returned data */
    jbyteArray jOutBuffer = env->NewByteArray((jsize)bytesReturned);
    if (jOutBuffer == NULL) {
  throwPcscException(env, obj, "SCardControl",
        "panic: couldn't create java byte array", returnCode);
  return NULL;
    }

    /* get the pointer to the internal byte buffer */
    jbyte   *ptr;
    ptr = env->GetByteArrayElements(jOutBuffer, NULL);
    if (ptr == NULL) {
  throwPcscException(env, obj, "SCardControl",
    "panic: couldn't get pointer to the internal buffer of the java byte array", returnCode);
  return NULL;
    }

    /* copy the read data to the new java byte array */
    memcpy((void *)ptr, (void *)tmpOutBuffer, (size_t)bytesReturned);

    /* return buffer control to java vm */
    env->ReleaseByteArrayElements(jOutBuffer, ptr, 0);

    /* return the data */
    return jOutBuffer;
}

/*
 * Class:     com_ibm_opencard_terminal_pcscmig_OCFPCSC1
 * Method:    SCardListReaders
 * Signature: (Ljava/lang/String;)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_com_ibm_opencard_terminal_pcscmig_OCFPCSCM_SCardListReaders
  (JNIEnv *env, jobject obj, jstring groups) {
    long      returnCode;
    DWORD     lenReaderList;
    char      *readerList;
    jobjectArray    readerArray;

    /* get a pointer to the converted 8bit-version of the groups string */
    const char *groupsUTF;
    groupsUTF = (groups != NULL) ? env->GetStringUTFChars(groups, NULL) : NULL;

    /* first retrieve the length of the readerlist */
    /* the first parameter <context> is not needed. query is not directed to a specific context */
    returnCode = SCardListReaders(NULL, (LPCTSTR)groupsUTF, NULL, &lenReaderList);
    // got the right length of the ReaderStr?
    if (returnCode != SCARD_S_SUCCESS) {
  env->ReleaseStringUTFChars(groups, groupsUTF);
  throwPcscException(env, obj, "SCardListReaders", "error getting length of reader list",returnCode);
  return NULL;
    }

    /* allocate space for the reader list */
    readerList = (char *)malloc((size_t)lenReaderList + 1);

    /* the first parameter <context> is not needed. query is not directed to a specific context */
    returnCode = SCardListReaders(NULL, (LPCTSTR)groupsUTF, readerList, &lenReaderList);
    if (returnCode != SCARD_S_SUCCESS) {
  free(readerList);
  throwPcscException(env, obj, "SCardListReaders", "error getting length of readerlist",returnCode);
  return NULL;
    }

    // The reader names are null terminated strings packed one
    // after another into the buffer. Separate them, making each into
    // a Java string. pack the strings into an array and return the array.

    // count names ....

    int ii;
    int jj;
    int numNames;
    for (ii=0, numNames=0; ii<lenReaderList; ) {
  numNames++;
  ii += strlen(&readerList[ii])+1;
  if (strlen(&readerList[ii]) == 0) ii++;    // series ended by 2 NULLs
    }                 /* endfor*/
    if (numNames == 0) return NULL;

    // make object array of proper size .....

    readerArray = env->NewObjectArray(numNames,env->FindClass("java/lang/String"),NULL);
    if (readerArray==NULL) {
  free(readerList);
  throwPcscException(env, obj, "SCardListReaders", "error converting reader list to array", 0);
  return NULL;
    }

    // make java strings and put them in array ...

    for (ii=0, jj=0; jj<numNames; jj++) {
  char       *name;
  jstring    jname;

  name = &readerList[ii];
  jname = env->NewStringUTF(name);
  if (jname==NULL) {
  free(readerList);
  throwPcscException(env, obj, "SCardListReaders", "error converting reader list to array", 0);
  return NULL;
    }

  env->SetObjectArrayElement(readerArray, jj, jname);
  if (env->ExceptionOccurred() != NULL) {
      free(readerList);
      throwPcscException(env, obj, "SCardListReaders", "error writing to reader array", 0);
      return NULL;
  }

  ii += strlen(name)+1;
    }                 /* endfor*/

    free(readerList);

    /* 8bit-string not longer needed */
    env->ReleaseStringUTFChars(groups, groupsUTF);

    return readerArray;
}

/*
 * Class:     com_ibm_opencard_terminal_pcscmig_OCFPCSC1
 * Method:    SCardTransmit
 * Signature: (I[B)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_ibm_opencard_terminal_pcscmig_OCFPCSCM_SCardTransmit
  (JNIEnv *env, jobject obj, jint jCard, jbyteArray jSendBuf) {

  SCARD_IO_REQUEST    sendPci = {SCARD_PROTOCOL_T1,sizeof(SCARD_IO_REQUEST)};
  SCARD_IO_REQUEST    recvPci = {SCARD_PROTOCOL_T1,sizeof(SCARD_IO_REQUEST)};
  long    returnCode;

  DWORD   lenSendBuf;
  jbyte   *ptrSendBuf;

  jbyteArray    jRecvBuf;
  DWORD   lenRecvBuf;
  jbyte   tmpRecvBuf[512];


  /* get the size of the sendbuf */
  lenSendBuf = env->GetArrayLength(jSendBuf);

  if (lenSendBuf > 0) {
    /* get the pointer to the send buf */
    ptrSendBuf = env->GetByteArrayElements(jSendBuf, NULL);
    if (ptrSendBuf == NULL) {
      throwPcscException(env, obj, "SCardTransmit", "error getting ptr to java sendbuffer", 0);
      return NULL;
    }
  } else
    ptrSendBuf = NULL;

  lenRecvBuf = sizeof(tmpRecvBuf);

  /* transmit the data */
  returnCode = SCardTransmit((SCARDHANDLE)jCard,
                             (LPSCARD_IO_REQUEST)&sendPci,
                             (LPCBYTE)ptrSendBuf, 
                             lenSendBuf,
                             (LPSCARD_IO_REQUEST)&recvPci,
                             (LPBYTE)tmpRecvBuf,
                             &lenRecvBuf);

  if (returnCode != SCARD_S_SUCCESS) {
    printf("******** ERROR CODE: %8.8x\n", returnCode);
    throwPcscException(env, obj, "SCardTransmit", "error occurred with SCardTransmit", returnCode);
    return NULL;
  }

  /* create the java receiveBuffer and copy the data into the new array */
  jRecvBuf = env->NewByteArray((jsize)lenRecvBuf);
  env->SetByteArrayRegion(jRecvBuf, (jsize)0, (jsize)lenRecvBuf, tmpRecvBuf);
  if (env->ExceptionOccurred() != NULL)
    return NULL;

  return jRecvBuf;
}



BOOL APIENTRY DllMain(HANDLE hModule,
          DWORD  ul_reason_for_call,
          LPVOID lpReserved)
{
    switch( ul_reason_for_call ) {
      case DLL_PROCESS_ATTACH:
  initContextTable();        /* clears the internal context table*/
  break;

      case DLL_THREAD_ATTACH:
  break;
      case DLL_THREAD_DETACH:
  break;
      case DLL_PROCESS_DETACH:
  removeAllContexts();
  break;
    }
    return TRUE;
}

// $Log: OCFPCSCM.cpp,v $
// Revision 1.5  1998/09/21 16:14:43  cvsusers
// clear text error messages implemented
//
// Revision 1.4  1998/07/02 11:47:37  breid
// updated for Migration Level mig.1.2.OCF.SCT
//
// Revision 1.3  1998/04/23 09:14:10  breid
// SCardTransmit: receiveBuffer enlarged
//
// Revision 1.2  1998/04/20 14:34:53  breid
// SCardDisconnect: ErrorHandling modified (ignode SCARD_W_CARD_REMOVED)
//
// Revision 1.1  1998/04/09 18:28:48  breid
// initial version
//
