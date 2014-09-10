//###########################################################################
// This file is part of LImA, a Library for Image Acquisition
//
// Copyright (C) : 2009-2011
// European Synchrotron Radiation Facility
// BP 220, Grenoble 38043
// FRANCE
//
// This is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//###########################################################################
/*
 * UltraNet.cpp
 * Created on: Apr 10, 2013
 * Author: gm
 */

#include <sstream>
#include <iostream>
#include <string>
#include <iomanip>
#include <cmath>
#include <cstring>

#include <stdarg.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/select.h>
#include <signal.h>

#include "UltraNet.h"
#include "ThreadUtils.h"
#include "Exceptions.h"
#include "Debug.h"

using namespace lima::Ultra;

//const int CR = '\15';				// carriage return
//const int LF = '\12';				// line feed
//const int MAX_ERRMSG = 1024;
//const char QUIT[] = "quit\n";		// sent using 'send'
const string terminator = "\r\n";

using namespace std;
using namespace lima;
UltraNet::UltraNet() {
	DEB_CONSTRUCTOR();
	// Ignore the sigpipe we get we try to send quit to
	// dead server in disconnect, just use error codes
	struct sigaction pipe_act;
	sigemptyset(&pipe_act.sa_mask);
	pipe_act.sa_flags = 0;
	pipe_act.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &pipe_act, 0);
	m_valid = 0;
	m_data_port = -1;
	firstFrame = true;
	lastFrameNo = 0;
}

UltraNet::~UltraNet() {
	DEB_DESTRUCTOR();
}

/*
 * Connect to remote server
 */
void UltraNet::connectToServer(const string hostname, int port) {
	DEB_MEMBER_FUNCT();
	struct hostent *host;
	struct protoent *protocol;
	int opt;

	if (m_valid) {
		THROW_HW_ERROR(Error) << "UltraNet::connectToServer(): Already connected to server";
	}
	if ((host = gethostbyname(hostname.c_str())) == 0) {
		endhostent();
		THROW_HW_ERROR(Error) << "UltraNet::connectToServer(): Can't get gethostbyname";
	}
	if ((m_skt = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		endhostent();
		THROW_HW_ERROR(Error) << "UltraNet::connectToServer(): Can't create socket";
	}
	m_remote_addr.sin_family = host->h_addrtype;
	m_remote_addr.sin_port = htons (port);
	size_t len = host->h_length;
	memcpy(&m_remote_addr.sin_addr.s_addr, host->h_addr, len);
	endhostent();
	if (connect(m_skt, (struct sockaddr *) &m_remote_addr, sizeof(struct sockaddr_in)) == -1) {
		close(m_skt);
		THROW_HW_ERROR(Error) << "UltraNet::connectToServer(): Connection to server refused. Is the server running?";
	}
	protocol = getprotobyname("tcp");
	if (protocol == 0) {
		THROW_HW_ERROR(Error) << "UltraNet::connectToServer(): Can't get protocol TCP";
	} else {
		opt = 1;
		if (setsockopt(m_skt, protocol->p_proto, TCP_NODELAY, (char *) &opt, 4) < 0) {
			THROW_HW_ERROR(Error) << "UltraNet::connectToServer(): Can't set socket options";
		}
	}
	endprotoent();
	m_valid = 1;
}

void UltraNet::disconnectFromServer() {
	DEB_MEMBER_FUNCT();
	if (m_valid) {
		shutdown(m_skt, 2);
		close(m_skt);
		m_valid = 0;
	}
}

void UltraNet::sendWait(string cmd, string& value) {
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "sendWait(" << cmd << ")";
	AutoMutex aLock(m_cond.mutex());
	int count;
	char recvBuf[41];
	string command = cmd + terminator;

	if (write(m_skt, command.c_str(), strlen(command.c_str())) <= 0) {
		THROW_HW_ERROR(Error) << "UltraNet::sendWait(): write to socket error";
	}
	if ((count = read(m_skt, recvBuf, 40)) <= 0) {
		THROW_HW_ERROR(Error) << "UltraNet::sendWait(): read from socket error";
	}
	recvBuf[count] = 0;
	stringstream ss;
	ss << recvBuf;
	value = ss.str();
}

void UltraNet::initServerDataPort(const string hostname, int port) {
	DEB_MEMBER_FUNCT();
	struct sockaddr_in data_addr;
//	int len = sizeof(struct sockaddr_in);
//	int one = 0x7FFFFFFF;

	if (m_data_port == -1) {
		if ((m_data_listen_skt = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
			THROW_HW_ERROR(Error) << "UltraNet::initServerDataPort(): create data socket error";
		}
		int optval = 32000000;
		if (setsockopt(m_data_listen_skt, SOL_SOCKET, SO_RCVBUF,(const char *)&optval, sizeof(optval)) < 0) {
			THROW_HW_ERROR(Error) << "UltraNet::initServerDataPort(): setsocketopt error";
		}
		// Bind the listening socket so that the server may connect to it.
		// Create a unique port number for the server to connect to.
		// Assign a port number at random. Allow anyone to connect.
		//		EthPort.sin_port = htons(HOSTPORT); //5005);
		//		EthPort.sin_addr.s_addr = inet_addr(cIPHost); //"192.168.1.103");
		data_addr.sin_family = AF_INET;
		data_addr.sin_addr.s_addr = inet_addr(hostname.c_str());
		data_addr.sin_port = htons(port);
		if (bind(m_data_listen_skt, (struct sockaddr *) &data_addr, sizeof(struct sockaddr_in)) == -1) {
			THROW_HW_ERROR(Error) << "UltraNet::initServerDataPort(): bind to socket error";
		}
//		if (listen(m_data_listen_skt, 1) == -1) {
//			THROW_HW_ERROR(Error) << "UltraNet::sendWait(): write to socket error";
//			close(m_data_listen_skt);
//			THROW_HW_ERROR(Error) << "UltraNet::sendWait(): write to socket error";
//		}
//		/* Find out which port was used */
//		if (getsockname(m_data_listen_skt, (struct sockaddr *) &data_addr, (socklen_t*)&len) == -1) {
//			close(m_data_listen_skt);
//			THROW_HW_ERROR(Error) << "UltraNet::sendWait(): write to socket error";
//		}
//		m_data_port = ntohs (data_addr.sin_port);
	}
}

void UltraNet::getData(void* bptr, int numBytes) {
	DEB_MEMBER_FUNCT();
	unsigned char buffer[numBytes+6];
	unsigned char* cptr = buffer;
	int frameNo;
//	int frameType;
	if (recv(m_data_listen_skt, buffer, sizeof(buffer), 0) != -1) {
		frameNo = (((unsigned int) cptr[0]) << 24) + (((unsigned int) cptr[1]) << 16) + (((unsigned int) cptr[2]) << 8)
				+ (unsigned int) cptr[3];
		// no used at the moment
//		frameType = (((unsigned int) cptr[4]) << 8) + (unsigned int) cptr[5];
		// check for missing frames
		if (!firstFrame && frameNo != lastFrameNo + 1) {
			THROW_HW_ERROR(Error) << "UltraNet::getData(): frame sequence error: Lost data";
		}
		DEB_TRACE() << "UltraNet::getData()" << DEB_VAR3(firstFrame, frameNo, lastFrameNo);
		lastFrameNo = frameNo;
		firstFrame = false;
		memcpy(bptr, cptr+6, numBytes);
		unsigned short *dptr = (unsigned short*) bptr;
		for (int i=0; i<numBytes/2; i++) {
			if (dptr[i] != 0)
				DEB_TRACE() << "UltraNet::getData()" << DEB_VAR2(i, dptr[i]);
		}
	}
	return;
}
