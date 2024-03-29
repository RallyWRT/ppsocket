/*****************************************************************************
Copyright (c) 2001 - 2008, The Board of Trustees of the University of Illinois.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the
  above copyright notice, this list of conditions
  and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the University of Illinois
  nor the names of its contributors may be used to
  endorse or promote products derived from this
  software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

/*****************************************************************************
written by
   Yunhong Gu, last updated 12/04/2008
*****************************************************************************/

#ifndef __UDT_API_H__
#define __UDT_API_H__


#include <map>
#include <vector>
#include "udt.h"
#include "packet.h"
#include "queue.h"
#include "co-op.h"


class CUDT;

struct CUDTSocket
{
   CUDTSocket();
   ~CUDTSocket();

   enum UDTSTATUS {INIT = 1, OPENED, LISTENING, CONNECTED, BROKEN, CLOSED};
   UDTSTATUS m_Status;                       // current socket state

   uint64_t m_TimeStamp;                     // time when the socket is closed

   int m_iIPversion;                         // IP version
   sockaddr* m_pSelfAddr;                    // pointer to the local address of the socket
   sockaddr* m_pPeerAddr;                    // pointer to the peer address of the socket

   UDTSOCKET m_SocketID;                     // socket ID
   UDTSOCKET m_ListenSocket;                 // ID of the listener socket; 0 means this is an independent socket

   UDTSOCKET m_PeerID;                       // peer socket ID
   int32_t m_iISN;                           // initial sequence number, used to tell different connection from same IP:port

   CUDT* m_pUDT;                             // pointer to the UDT entity

   std::set<UDTSOCKET>* m_pQueuedSockets;    // set of connections waiting for accept()
   std::set<UDTSOCKET>* m_pAcceptSockets;    // set of accept()ed connections

   pthread_cond_t m_AcceptCond;              // used to block "accept" call
   pthread_mutex_t m_AcceptLock;             // mutex associated to m_AcceptCond

   unsigned int m_uiBackLog;                 // maximum number of connections in queue
};

////////////////////////////////////////////////////////////////////////////////

class CUDTUnited
{
friend class CUDT;

public:
   CUDTUnited();
   ~CUDTUnited();

public:

      // Functionality:
      //    initialize the UDT library.
      // Parameters:
      //    None.
      // Returned value:
      //    0 if success, otherwise -1 is returned.

   int startup();

      // Functionality:
      //    release the UDT library.
      // Parameters:
      //    None.
      // Returned value:
      //    0 if success, otherwise -1 is returned.

   int cleanup();

      // Functionality:
      //    Create a new UDT socket.
      // Parameters:
      //    0) [in] af: IP version, IPv4 (AF_INET) or IPv6 (AF_INET6).
      //    1) [in] type: socket type, SOCK_STREAM or SOCK_DGRAM
      // Returned value:
      //    The new UDT socket ID, or INVALID_SOCK.

   UDTSOCKET newSocket(const int& af, const int& type);

      // Functionality:
      //    Create a new UDT connection.
      // Parameters:
      //    0) [in] listen: the listening UDT socket;
      //    1) [in] peer: peer address.
      //    2) [in/out] hs: handshake information from peer side (in), negotiated value (out);
      // Returned value:
      //    If the new connection is successfully created: 1 success, 0 already exist, -1 error.

   int newConnection(const UDTSOCKET listen, const sockaddr* peer, CHandShake* hs);

      // Functionality:
      //    look up the UDT entity according to its ID.
      // Parameters:
      //    0) [in] u: the UDT socket ID.
      // Returned value:
      //    Pointer to the UDT entity.

   CUDT* lookup(const UDTSOCKET u);

      // Functionality:
      //    Check the status of the UDT socket.
      // Parameters:
      //    0) [in] u: the UDT socket ID.
      // Returned value:
      //    UDT socket status, or INIT if not found.

   CUDTSocket::UDTSTATUS getStatus(const UDTSOCKET u);

      // socket APIs

   int bind(const UDTSOCKET u, const sockaddr* name, const int& namelen);
   int bind(const UDTSOCKET u, UDPSOCKET udpsock);
   int listen(const UDTSOCKET u, const int& backlog);
   UDTSOCKET accept(const UDTSOCKET listen, sockaddr* addr, int* addrlen);
   int connect(const UDTSOCKET u, const sockaddr* name, const int& namelen);
   int close(const UDTSOCKET u);
   int getpeername(const UDTSOCKET u, sockaddr* name, int* namelen);
   int getsockname(const UDTSOCKET u, sockaddr* name, int* namelen);
   int select(ud_set* readfds, ud_set* writefds, ud_set* exceptfds, const timeval* timeout);

      // Functionality:
      //    record the UDT exception.
      // Parameters:
      //    0) [in] e: pointer to a UDT exception instance.
      // Returned value:
      //    None.

   void setError(CUDTException* e);

      // Functionality:
      //    look up the most recent UDT exception.
      // Parameters:
      //    None.
      // Returned value:
      //    pointer to a UDT exception instance.

   CUDTException* getError();

private:
   std::map<UDTSOCKET, CUDTSocket*> m_Sockets;       // stores all the socket structures

   pthread_mutex_t m_ControlLock;                    // used to synchronize UDT API

   pthread_mutex_t m_IDLock;                         // used to synchronize ID generation
   UDTSOCKET m_SocketID;                             // seed to generate a new unique socket ID

private:
   pthread_key_t m_TLSError;                         // thread local error record (last error)
   #ifndef WIN32
      static void TLSDestroy(void* e) {if (NULL != e) delete (CUDTException*)e;}
   #else
      std::map<DWORD, CUDTException*> m_mTLSRecord;
      void checkTLSValue();
      pthread_mutex_t m_TLSLock;
   #endif

private:
   CUDTSocket* locate(const UDTSOCKET u);
   CUDTSocket* locate(const UDTSOCKET u, const sockaddr* peer, const UDTSOCKET& id, const int32_t& isn);
   void updateMux(CUDT* u, const sockaddr* addr = NULL, const UDPSOCKET* = NULL);
   void updateMux(CUDT* u, const CUDTSocket* ls);

private:
   std::vector<CMultiplexer> m_vMultiplexer;		// UDP multiplexer
   pthread_mutex_t m_MultiplexerLock;

private:
   CControl* m_pController;				// UDT congestion control manager

private:
   volatile bool m_bClosing;
   pthread_mutex_t m_GCStopLock;
   pthread_cond_t m_GCStopCond;

   pthread_mutex_t m_InitLock;
   bool m_bGCStatus;					// if the GC thread is working (true)

   pthread_t m_GCThread;
   #ifndef WIN32
      static void* garbageCollect(void*);
   #else
      static DWORD WINAPI garbageCollect(LPVOID);
   #endif

   std::map<UDTSOCKET, CUDTSocket*> m_ClosedSockets;   // temporarily store closed sockets

   void checkBrokenSockets();
   void removeSocket(const UDTSOCKET u);
};

#endif
