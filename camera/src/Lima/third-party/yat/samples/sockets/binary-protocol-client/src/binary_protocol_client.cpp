/*!
 * \file     
 * \brief    A yat::BitsStream example.
 * \author   N. Leclercq - Synchrotron SOLEIL
 */


#include <iostream>
#include <yat/bitsstream/Endianness.h>
#include <yat/threading/Thread.h>
#include <yat/network/ClientSocket.h>

#define SERVER_PORT 5001

const unsigned char cmds[] = 
{
  0,
  1,
  2,
  3
};

enum
{
  CMD_0 = 0x0,
  CMD_1 = 0x1,
  CMD_2 = 0x2,
  CMD_3 = 0x3
};

const size_t MAX_ERROR_LENGTH = 256;
const unsigned char INVALID_CMD = 0xFF;

typedef unsigned char   CmdIdentifier;
typedef unsigned long   CmdDataSize;
typedef unsigned long   CmdDataLength;
typedef unsigned char * CmdData;
typedef char            CmdStatus;

const char CMD_ERROR    = -1;
const char CMD_NO_ERROR =  0;

const CmdDataSize CMD_HEADER_SIZE = sizeof(CmdIdentifier) 
                                  + sizeof(CmdDataSize) 
                                  + sizeof(CmdDataLength);

typedef struct 
{
  CmdIdentifier   id; //- cmd identifier
  CmdStatus       st; //- cmd status: CMD_ERROR or CMD_NO_ERROR  
  CmdDataSize     ds; //- reply size in bytes (could be the size or an error string in case <st> is NOT set to CMD_NO_ERROR) 
  CmdDataLength   dl; //- the number of elements <CmdData> - each element has a size of <ds>/<dl> bytes
  CmdData         db; //- reply data size in bytes (could be an error string in case <st> is NOT set to CMD_NO_ERROR)
} CmdReply;


//-----------------------------------------------------------------------------
// MAIN
//-----------------------------------------------------------------------------
int main (int argc, char* argv[])
{
  try
  {
    //- yat internal cooking
    yat::Socket::init();

    //- instanciating the Socket (default protocol is yat::Socket::TCP_PROTOCOL)
    std::cout << "Instanciating the Client Socket" << std::endl;
    yat::ClientSocket sock;

    //- set some socket option
    std::cout << "Setting sock options" << std::endl;
    sock.set_blocking_mode();

    //- network address
    std::cout << "Instanciating network address" << std::endl;
    yat::Address addr("localhost", SERVER_PORT);

    //- connect to addr
    std::cout << "Connecting to peer..." << std::endl;
    sock.connect(addr);

    size_t n = 2;

    yat::uint32 dd_length = 65;  
    yat::Buffer<double> dd(dd_length);
    for (size_t i = 0; i < dd_length; i++)
      dd[i] = static_cast<double>(i);

    yat::Socket::Data binary_out;
    CmdDataSize ds = dd_length * sizeof(double);
    binary_out.capacity(CMD_HEADER_SIZE + ds);
    binary_out.force_length(binary_out.capacity());

    yat::Socket::Data binary_in(32 * 1024, true);
   
    do
    {      
      char * p = binary_out.base();

      size_t offset = 0;
      
      memcpy(p + offset, &cmds[0], sizeof(CmdIdentifier));
      
      offset += sizeof(CmdIdentifier);
      
      memcpy(p + offset, &ds, sizeof(CmdDataSize));
      
      offset += sizeof(CmdDataSize);
        
      memcpy(p + offset, &dd_length, sizeof(CmdDataLength));
      
      offset += sizeof(CmdDataLength);
      
      memcpy(p + offset, dd.base(), ds);

      std::cout << "Sending " << binary_out.size() << " bytes to the server..." << std::endl;

      sock << binary_out;

      std::cout << "Waiting for reply from the server..." << std::endl;

      sock >> binary_in;
      
      CmdReply reply;
      reply.id = *((CmdIdentifier*)(binary_in.base() + 0));
      reply.st = *((CmdStatus*)(binary_in.base() + 1));
      reply.ds = *((CmdDataSize*)(binary_in.base() + 2));
      reply.dl = *((CmdDataLength*)(binary_in.base() + 6));
      reply.db = (CmdData)(binary_in.base() + 10);
      
      std::cout << "reply for cmd-" 
               <<  (unsigned int)reply.id
               << " is "
               << ( reply.st == CMD_ERROR ? "ERROR" : "NO-ERROR")
               << " + "
               << reply.ds 
               << " bytes of data representing "
               << reply.dl 
               << " elements of "
               << reply.ds / reply.dl
               << " bytes each"
               << std::endl;

      --n;
    } 
    while (n);

    //- disconnect from peer
    std::cout << "Disconnecting from peer..." << std::endl;
    sock.disconnect();

    //- yat internal cooking
    yat::Socket::terminate();

  }
  catch (const yat::SocketException & se)
  {
    std::cout << "*** yat::SocketException caught ***" << std::endl;

    for (size_t err = 0; err  < se.errors.size(); err++)
    {
      std::cout << "Err-" << err << "::reason..." << se.errors[err].reason << std::endl;
      std::cout << "Err-" << err << "::desc....." << se.errors[err].desc << std::endl;
      std::cout << "Err-" << err << "::origin..." << se.errors[err].origin << std::endl;
      std::cout << "Err-" << err << "::code....." << se.errors[err].code << std::endl;
    }
  } 
  catch (...)
  {
    std::cout << "Unknown exception caught" << std::endl;
  }

	return 0;  
}
