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

#ifndef ULTRANET_CPP_
#define ULTRANET_CPP_

#include <netinet/in.h>
#include "Debug.h"

using namespace std;

namespace lima {
namespace Ultra {

const int RD_BUFF = 1000;	// Read buffer for more efficient recv

class UltraNet {
DEB_CLASS_NAMESPC(DebModCamera, "UltraNet", "Ultra");

public:
	UltraNet();
	~UltraNet();


	void sendWait(string cmd, string& value);

	void connectToServer (const string hostname, int port);
	void disconnectFromServer();
	void initServerDataPort(const string hostname, int udpPort);
	void getData(void* bptr, int num);

private:
	mutable Cond m_cond;
	bool m_valid;						// true if connected
	int m_skt;							// socket for commands */
	struct sockaddr_in m_remote_addr;	// address of remote server */
	int m_data_port;					// our data port
	int m_data_listen_skt;				// data socket we listen on
	bool firstFrame;
	int lastFrameNo;
};

} // namespace Ultra
} // namespace lima

#endif /* ULTRANET_CPP_ */
