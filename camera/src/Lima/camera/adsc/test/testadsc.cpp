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
#include "AdscCamera.h"
#include "HwBufferSave.h"
#include "PoolThreadMgr.h"

#include <iostream>
#include <cstdlib>

#include <stdlib.h>


using namespace lima;
using namespace std;

class TestFrameCallback : public HwFrameCallback
{
public:
	TestFrameCallback(AdscCamera& adsc, HwBufferSave& buffer_save) 
		: m_adsc(adsc), m_buffer_save(buffer_save) {}
protected:
	virtual bool newFrameReady(const HwFrameInfoType& frame_info);
private:
	AdscCamera& m_adsc;
	HwBufferSave& m_buffer_save;
};

bool TestFrameCallback::newFrameReady(const HwFrameInfoType& frame_info)
{
	cout << "acq_frame_nb=" << frame_info.acq_frame_nb  << ", "
	     << "ts=" << frame_info.frame_timestamp << ", "
	     << "adsc=" << m_adsc  << endl;
	cout << "skipping buffer write..." << endl;
	// m_buffer_save.writeFrame(frame_info);
	return true;
}

/*
 *	Similar test to detcon_test_th
 *
 *	testadsc prefix start_frame end_frame time
 */

int main(int argc, char *argv[])
{
	AdscCamera adsc;
	HwBufferSave buffer_save(HwBufferSave::EDF);
	TestFrameCallback cb(adsc, buffer_save);

	FrameDim frame_dim;
	adsc.getFrameDim(frame_dim);
	Size size = frame_dim.getSize();

	HwBufferCtrlObj *buffer_mgr = adsc.getBufferMgr();
	buffer_mgr->setFrameDim(frame_dim);
	buffer_mgr->setNbBuffers(10);
	buffer_mgr->registerFrameCallback(cb);

	if(argc < 5)
	{
		cout << "Usage: detcon_test_th prefix start end time\n" << endl;
		exit(0);
	}
	double	exp_time;
	int	start_frame, end_frame;
	char	*prefix;

	int	n;

	exp_time = atof(argv[4]);
	start_frame = atoi(argv[2]);
	end_frame = atoi(argv[3]);
	prefix = argv[1];

	usleep(1000000);

	//
	//	Do two darks and then one image as a test.
	//

	for(n = 0 ; n < 2; n++)
	{
		char	imno[10];
		std::string str_imno;
		std::string full_filename;

		sprintf(imno, "_%03d", start_frame);
		str_imno = imno;
		full_filename = prefix;
		full_filename = full_filename + str_imno;

		adsc.setFilename(full_filename);
		adsc.setImageKind(n);
		adsc.setExpTime(exp_time);
		cout << "Dark 0: adsc=" << adsc << endl;
		adsc.startAcq();
		cout << "Dark 0: adsc=" << adsc << endl;
		adsc.stopAcq();
		cout << "Dark 0: adsc=" << adsc << endl;
	}

	for(n = start_frame ; n <= end_frame; n++)
	{
		char	imno[10];
		std::string str_imno;
		std::string full_filename;

		sprintf(imno, "_%03d", n);
		str_imno = imno;
		full_filename = prefix;
		full_filename = full_filename + str_imno;
		
		adsc.setImageKind(5);
		adsc.setExpTime(2.0);
		adsc.setFilename(full_filename);
		cout << "Image 1: adsc=" << adsc << endl;
		adsc.startAcq();
		cout << "Image 1: adsc=" << adsc << endl;
		adsc.stopAcq();
		cout << "Image 1: adsc=" << adsc << endl;
	}

	cout << "Sleep 2 seconds to enable all data to be flushed" << endl;
	usleep(2000000);

	PoolThreadMgr &pMgr = PoolThreadMgr::get();
	pMgr.quit();

	exit(0);
}

//
//	Original main.
//

int main_previous(int argc, char *argv[])
{
	AdscCamera adsc;
	HwBufferSave buffer_save(HwBufferSave::EDF);
	TestFrameCallback cb(adsc, buffer_save);

	FrameDim frame_dim;
	adsc.getFrameDim(frame_dim);
	Size size = frame_dim.getSize();

	HwBufferCtrlObj *buffer_mgr = adsc.getBufferMgr();
	buffer_mgr->setFrameDim(frame_dim);
	buffer_mgr->setNbBuffers(10);
	buffer_mgr->registerFrameCallback(cb);

	cout << "ImageSet 1: adsc=" << adsc << endl;
	adsc.startAcq();
	cout << "ImageSet 1: adsc=" << adsc << endl;
	adsc.stopAcq();
	cout << "ImageSet 1: adsc=" << adsc << endl;

	adsc.setExpTime(5);

	cout << "ImageSet 2: adsc=" << adsc << endl;
	adsc.startAcq();
	cout << "ImageSet 2: adsc=" << adsc << endl;
	adsc.stopAcq();
	cout << "ImageSet 2: adsc=" << adsc << endl;

	adsc.setExpTime(2);
//	adsc.setNbFrames(3);
	adsc.setNbFrames(1);

	cout << "ImageSet 3: adsc=" << adsc << endl;
	adsc.startAcq();
	cout << "ImageSet 3: adsc=" << adsc << endl;
	adsc.stopAcq();
	cout << "ImageSet 3: adsc=" << adsc << endl;

	PoolThreadMgr &pMgr = PoolThreadMgr::get();
	pMgr.quit();
	exit(0);
}
