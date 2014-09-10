#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <iostream>

#include "server.h"
#include "client.h"

enum
{
  SOCKET_EVENT_CONNECT    = 0x0,
  SOCKET_EVENT_DISCONNECT = 0x1,
  SOCKET_EVENT_DATA       = 0x2
};

enum
{
  CMD_0 = 0x0,
  CMD_1 = 0x1,
  CMD_2 = 0x2,
  CMD_3 = 0x3
};

#define PORT        5001
#define MAX_CLIENTS 1
#define BUF_SIZE	  128

typedef SOCKET          ClientSocket;
typedef int             SocketEvent;
typedef unsigned char   CmdIdentifier;
typedef unsigned long   CmdDataSize;
typedef unsigned long   CmdDataLength;
typedef unsigned char * CmdData;
typedef char            CmdStatus;

const int           kERROR          = -1; 
const int           kNO_ERROR       =  0;
const CmdIdentifier INVALID_CMD     =  0xFF;

const CmdDataSize CMD_HEADER_SIZE = sizeof(CmdIdentifier) 
                                  + sizeof(CmdDataSize) 
                                  + sizeof(CmdDataLength);

typedef struct 
{
  ClientSocket  cs; //- client socket (for reply)
  CmdIdentifier id; //- cmd identifier
  CmdDataSize   ds; //- cmd data size in bytes
  CmdDataSize   rd; //- total number of bytes received for the cmd (expecting {CMD_HEADER_SIZE + <ds>} bytes in one or more buffers)
  CmdDataLength dl; //- cmd data length in number of elements
  CmdData       db; //- cmd data buffer (will be reinterpreted by cmd processing function) 
} CmdInfo;

//- the current processed cmd (cmds are processed one at a time - means one and only one connected client)
static CmdInfo current_cmd;

//- initializes the cmd info (MUST be called at startup)
static void init_cmd (void);

//- clears the cmd info (in order to be able to process the next one)
static void clear_cmd (void);

//- cmd processor - does the job and sends reply to client
static int process_cmd (void);

//- sends reply to client
static int send_reply_txt (CmdStatus stt, const char * txt);
static int send_reply_data (CmdStatus stt, CmdDataSize nbt, CmdDataLength nbe, unsigned char * buf);

//- simulate SPI board TCP/IP callback
void socket_callback (ClientSocket client_socket,
                      SocketEvent socket_event, 
                      CmdDataSize num_bytes_from_client, 
                      CmdData client_socket_buffer);

//-----------------------------------------------------------------------
// init_cmd
//-----------------------------------------------------------------------
static void init_cmd (void)
{
  //- be sure each byte of <current_cmd> is set to zero at init
  memset(&current_cmd, 0, sizeof(CmdInfo));

  //- be sure <current_cmd.id> is set to INVALID_CMD
  current_cmd.id = INVALID_CMD;
}

//-----------------------------------------------------------------------
// clear_cmd
//-----------------------------------------------------------------------
static void clear_cmd (void)
{
  //- release memory (leak otherwise)
  if ( current_cmd.db ) 
  {
    free(current_cmd.db);
    current_cmd.db = 0;
  }
  
  //- clear the whole structure
  memset(&current_cmd, 0, sizeof(CmdInfo));

  //- be sure <current_cmd.id> is set to INVALID_CMD
  current_cmd.id = INVALID_CMD;
}

//-----------------------------------------------------------------------
// socket_callback (simulates SPI callback)
//-----------------------------------------------------------------------
void socket_callback (SOCKET client_socket,
                      int socket_event, 
                      size_t num_bytes_from_client, 
                      char * client_socket_buffer)
{
  unsigned long elem_size = 0;
  unsigned long n_bytes_to_push = 0;
  
  //- here we simulate the SPI callback
  switch ( socket_event )
  {
    case SOCKET_EVENT_CONNECT:
      std::cout << "socket_callback::SOCKET_EVENT_CONNECT" << std::endl;  
      init_cmd ();
      break;
    case SOCKET_EVENT_DISCONNECT:
      std::cout << "socket_callback::SOCKET_EVENT_DISCONNECT" << std::endl;  
      clear_cmd ();
      break;
    default:
      break;
  }
  
  //- any data to process?
  if ( num_bytes_from_client == 0 || client_socket_buffer == 0)
    return;
    
  //- offset to data in <buffer>
  size_t buffer_offset = 0;

  //- are we already processing a cmd? (cmd data can spread over several buffers)
  if ( current_cmd.id == INVALID_CMD )
  {
    //- client socket
    current_cmd.cs = client_socket;
    //- cmd identifier
    current_cmd.id = (CmdIdentifier)client_socket_buffer[0];
    //- number of bytes in the cmd data
    current_cmd.ds = *((CmdDataSize*)&client_socket_buffer[1]);
    //- number of elements in the cmd data
    current_cmd.dl = *((CmdDataLength*)&client_socket_buffer[5]);
    //- size of each element in cmd data
    if ( current_cmd.dl )
      elem_size = current_cmd.ds / current_cmd.dl;
    else
      elem_size = 0;
    //- log
    std::cout << "processing 'new' cmd-" << (unsigned int)current_cmd.id << std::endl;
    std::cout << "expecting " << current_cmd.dl << " elements of " << elem_size << " byte(s) each" << std::endl;
    std::cout << "expecting " << current_cmd.ds << " bytes of data (excluding header)" << std::endl;
    //- allocate cmd data buffer (will be released/freed by the <clear_cmd> function) 
    current_cmd.db = (CmdData)::malloc(current_cmd.ds);
    //- check allocated memory
    if ( ! current_cmd.db )
    {
      //-TODO: send error to client, but... replying also requires memory allocation so... ????
    }
    //- clear allocated buffer
    memset(current_cmd.db, 0, current_cmd.ds);
    //- be sure the 'received data' counter is set to 0 for 'new' cmd
    current_cmd.rd = 0;
    //- offset to cmd data in socket input <buffer>
    buffer_offset = CMD_HEADER_SIZE;
  }

  std::cout << "processing cmd-" 
           <<  (unsigned int)current_cmd.id 
           << " - till now we received " 
           << current_cmd.rd
           << " bytes for this cmd" 
           << std::endl;

  //- push cmd data into cmd buffer...
  n_bytes_to_push = current_cmd.ds - current_cmd.rd;

  if ( n_bytes_to_push > num_bytes_from_client)
    n_bytes_to_push = num_bytes_from_client - buffer_offset;

  std::cout << "processing cmd-" 
           <<  (unsigned int)current_cmd.id 
           << " - pushing " 
           << n_bytes_to_push
           << " bytes into cmd data buffer" 
           << std::endl;

  memcpy(current_cmd.db + current_cmd.rd, 
         client_socket_buffer + buffer_offset, 
         n_bytes_to_push);

  //- increment the "received data for the current" counter  
  current_cmd.rd += n_bytes_to_push;

  //- received all expected data for the current cmd?
  if ( current_cmd.rd == current_cmd.ds )
  {
    std::cout << "received expected amount of data for cmd-" 
             <<  (unsigned int)current_cmd.id 
             << " - calling cmd processor..." 
             << std::endl;

    //- call cmd processor (will also send reply to client)
    process_cmd();

    //- clear the "cmd info" to process next cmd
    clear_cmd();
  }
}

//-----------------------------------------------------------------------
// process_cmd
//-----------------------------------------------------------------------
static int process_cmd (void)
{
  double * p = 0;
  
  std::cout << "processing cmd-" << (unsigned int)current_cmd.id << std::endl;

  switch ( current_cmd.id )
  {
    case CMD_0:
    {
     p = (double *)current_cmd.db;
     std::cout << "processing cmd-" 
               << (unsigned int)current_cmd.id 
               << " cmd-data["
               << 0
               << "] = "
               << *p
               << std::endl;
     std::cout << "processing cmd-" 
               << (unsigned int)current_cmd.id 
               << " cmd-data["
               << current_cmd.dl - 1
               << "] = "
               << *(p + current_cmd.dl - 1)
               << std::endl;
     //- let's say that CMD-0 simply returns the data it received...
     send_reply_data(kNO_ERROR, current_cmd.ds, current_cmd.dl, current_cmd.db);
    } 
    break;

   default:
     break;
  }

  return kNO_ERROR;
}

//-----------------------------------------------------------------------
// send_reply_txt
//-----------------------------------------------------------------------
static int send_reply_txt (CmdStatus stt, const char * txt)
{
  std::cout << "returning txt to client for cmd-" 
            << (unsigned int)current_cmd.id
            << ": "
            << txt
            << std::endl;
    
  return send_reply_data(stt, strlen(txt), strlen(txt), (unsigned char*)txt);
}

//-----------------------------------------------------------------------
// send_reply_data
//-----------------------------------------------------------------------
static int send_reply_data (CmdStatus     stt, //- status 
                            CmdDataSize   nbt, //- num of bytes
                            CmdDataLength nbe, //- num of elements
                            CmdData       buf) //- data
{
  CmdData db = 0;
  unsigned long offset = 0;
  
  std::cout << "returning "
            << nbt
            << " bytes of data to client for cmd-" 
            << (unsigned int)current_cmd.id
            << std::endl;
    
  size_t bs = sizeof(CmdIdentifier) 
            + sizeof(CmdStatus) 
            + sizeof(CmdDataSize) 
            + sizeof(CmdDataLength) 
            + nbt;

  db = (CmdData)malloc(bs); 
  if ( db == 0 )
  {
    //- error handling goes here...
    return kERROR;
  }

  memset(db, 0, bs);

  memcpy(db, &current_cmd.id, sizeof(CmdIdentifier));

  offset += sizeof(CmdIdentifier);

  memcpy(db + offset, &stt, sizeof(CmdStatus));

  offset += sizeof(CmdStatus);

  memcpy(db + offset, &nbt, sizeof(CmdDataSize));

  offset += sizeof(CmdDataSize);

  memcpy(db + offset, &nbe, sizeof(CmdDataLength));

  offset += sizeof(CmdDataLength);
  
  memcpy(db + offset, buf, nbt);

  int result = kNO_ERROR;

  if ( send(current_cmd.cs, db, bs , 0) < 0 )
  {
    //- oops... error handling goes here...
    result = kERROR;
  }
   
  //- release allocated memory...
  free(db);

  return result;
}

//-----------------------------------------------------------------------
int main(int argc, char **argv)
{
   init();

   app(socket_callback);

   end();

   return EXIT_SUCCESS;
}

