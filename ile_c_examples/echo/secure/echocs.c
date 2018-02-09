/* ==========================================================================

  Clarasoft Foundation Server 400

  echoclient.c
  Echo client using SSL
  Version 1.0.0

  Compile module with:

     CRTCMOD MODULE(ECHOSC) SRCFILE(QCSRC)
              SRCMBR(ECHOSC) DBGVIEW(*SOURCE)

  Build with:

     CRTPGM PGM(ECHOSC) MODULE(ECHOSC)
            BNDSRVPGM(CFSAPI)

  Before executing, you need a server: you can use the CFS
  SSL ECHO server (see ECHOS.C source file) by calling it with:


    call clarad parm('41101' '1' '1'
                     '/QSYS.LIB/LIBANME.LIB/CLARAH.PGM' 'ECHOS')

  Then execute this program with:

     CALL ECHOSC PARM('YOUR_ECHO_SERVER_HOSTNAME' '41101')


  Distributed under the MIT license

  Copyright (c) 2013 Clarasoft I.T. Solutions Inc.

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files
  (the "Software"), to deal in the Software without restriction,
  including without limitation the rights to use, copy, modify,
  merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
  ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH
  THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

========================================================================== */


#include"QCSRC/CFSAPI.H"
#include"QCSRC/CSSTR.H"

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <gskssl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>


int main(int argc, char** argv)
{

   char inBuffer[65];
   char outBuffer[65];

   char szHostname[128];
   CFS_INSTANCE* cfsi;
   uint64_t iSize;
   int SSLResult;
   CSRESULT hResult;
   CFS_CLIENTSESSION_100 sinfo;

   CSSTRCV cvtString;

   // String Conversion object
   cvtString = CSSTRCV_Constructor();

   do
   {

       memset(&sinfo, 0, sizeof(sinfo));

       sinfo.szHostName = argv[1];
       sinfo.port = (long)atoi(argv[2]);
       sinfo.pszBuffers[CFS_OFFSET_GSK_OS400_APPLICATION_ID] = "WSD";

       cfsi = CFS_SecureConnect((void*)&sinfo,
                                CFS_CLIENTSESSION_FMT_100,
                                &SSLResult);

       if (cfsi != 0) {

          strcpy(outBuffer, "Hello World!");
          iSize = strlen(outBuffer);


          //////////////////////////////////////////////////////////////
          // The echo server expects UT8
          //////////////////////////////////////////////////////////////

          // Start a conversion from  the job CCSID to UTF8
          CSSTRCV_SetConversion(cvtString, CCSID_JOBDEFAULT, CCSID_UTF8);

          CSSTRCV_StrCpy(cvtString, outBuffer, strlen(outBuffer));

          iSize = CSSTRCV_Size(cvtString);

          // We know that outBuffer is large enough ...
          CSSTRCV_Get(cvtString, outBuffer);

          // null terminate
          outBuffer[iSize] = 0;

          CFS_SecureWriteRecord(cfsi, outBuffer, &iSize, -1, &SSLResult);

          iSize = 64; // Maximum nimber of bytes to read

          hResult = CFS_SecureRead(cfsi,
                                   inBuffer,
                                   &iSize,
                                   -1,
                                   &SSLResult);

          if (CS_SUCCEED(hResult)) {

             //////////////////////////////////////////////////////////////
             // The echo server echoes back in UT8
             //////////////////////////////////////////////////////////////

             // Start a conversion from UTF8 to the job CCSID
             CSSTRCV_SetConversion(cvtString, CCSID_UTF8, CCSID_JOBDEFAULT);

             CSSTRCV_StrCpy(cvtString, inBuffer, iSize);

             iSize = CSSTRCV_Size(cvtString);

             // We know that inBuffer is large enough ...
             CSSTRCV_Get(cvtString, inBuffer);

             // null terminate
             inBuffer[iSize] = 0;

             /* write results to screen */

             printf("Received %d bytes, here they are ...\n", (long)iSize);
             printf("%s\n", inBuffer);
          }
          else {

             printf("Failed ...\n");
          }

          CFS_SecureClose(cfsi);
       }

   } while(0);

   return 0;
}

