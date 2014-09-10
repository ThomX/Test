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

#include <string>
#include <unistd.h>
#include <cmath>

using namespace lima;
using namespace lima::Adsc;

//-----------------------------------------------------
//
//-----------------------------------------------------
Camera::AdscThread::AdscThread(Camera& adsc) :
		m_adsc(&adsc)
{
	m_acq_frame_nb = 0;
	m_force_stop = false;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::AdscThread::start()
{
	CmdThread::start();
	waitStatus(Ready);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::AdscThread::init()
{
	setStatus(Ready);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::AdscThread::execCmd(int cmd)
{
	int status = getStatus();
	switch (cmd)
	{
		case StartAcq:
			if (status != Ready)
				throw LIMA_HW_EXC(InvalidValue, "Not Ready to StartAcq");
			execStartAcq();
		break;
	}
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::AdscThread::execStartAcq()
{
	DEB_MEMBER_FUNCT();

	AdscApi& adsc_api = m_adsc->m_adsc_api;
	adsc_api.resetFrameNr();

	int nb_frames = m_adsc->m_nb_frames;
	int& frame_nb = m_acq_frame_nb;
	for (frame_nb = 0; (frame_nb < nb_frames) || (nb_frames == 0); frame_nb++)
	{
		double req_time;
		if (m_force_stop)
		{
			m_force_stop = false;
			setStatus(Ready);
			return;
		}
		req_time = m_adsc->m_exp_time;
		if (req_time > 0)
		{
			setStatus(Exposure);
			usleep(long(req_time * 1e6));
		}

		setStatus(Readout);

		req_time = m_adsc->m_lat_time;
		if (req_time > 0)
		{
			setStatus(Latency);
			usleep(long(req_time * 1e6));
		}
	}
	setStatus(Ready);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
int Camera::AdscThread::getNbAcquiredFrames()
{
	DEB_MEMBER_FUNCT();
	return m_acq_frame_nb;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
Camera::Camera() :	m_thread(*this)
{
	DEB_CONSTRUCTOR();

	init();

	m_thread.start();
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::init()
{
	DEB_MEMBER_FUNCT();
	m_exp_time = 1.0;
	m_lat_time = 0.0;
	m_nb_frames = 1;
	m_flp_kind = 5; //
	m_bin_used = 2;
	m_adc_used = 1;
	m_output_raws = 0;
	m_no_transform = 0;
	m_use_stored_dark = 1; //
	m_last_image = 0;
	m_path = "/927bis/ccd/limatest/";
	m_filename = "default_1_001.img";

	CCDInitialize();

}

//-----------------------------------------------------
//
//-----------------------------------------------------
Camera::~Camera()
{
	DEB_DESTRUCTOR();
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getMaxImageSize(Size& max_image_size)
{
	DEB_MEMBER_FUNCT();
	m_adsc_api.getMaxImageSize(max_image_size);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::setNbFrames(int nb_frames)
{
	DEB_MEMBER_FUNCT();
	if (nb_frames < 0)
		throw LIMA_HW_EXC(InvalidValue, "Invalid nb of frames");

	m_nb_frames = nb_frames;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getNbFrames(int& nb_frames)
{
	DEB_MEMBER_FUNCT();
	nb_frames = m_nb_frames;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::setExpTime(double exp_time)
{
	DEB_MEMBER_FUNCT();
	if (exp_time <= 0)
		throw LIMA_HW_EXC(InvalidValue, "Invalid exposure time");

	m_exp_time = exp_time;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getExpTime(double& exp_time)
{
	DEB_MEMBER_FUNCT();
	exp_time = m_exp_time;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::setLatTime(double lat_time)
{
	DEB_MEMBER_FUNCT();
	if (lat_time < 0)
		throw LIMA_HW_EXC(InvalidValue, "Invalid latency time");

	m_lat_time = lat_time;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getLatTime(double& lat_time)
{
	DEB_MEMBER_FUNCT();
	lat_time = m_lat_time;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::setBin(const Bin& bin)
{
	DEB_MEMBER_FUNCT();
	m_adsc_api.setBin(bin);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getBin(Bin& bin)
{
	DEB_MEMBER_FUNCT();
	m_adsc_api.getBin(bin);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::checkBin(Bin& bin)
{
	DEB_MEMBER_FUNCT();
	m_adsc_api.checkBin(bin);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::setFrameDim(const FrameDim& frame_dim)
{
	DEB_MEMBER_FUNCT();
	m_adsc_api.setFrameDim(frame_dim);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getFrameDim(FrameDim& frame_dim)
{
	DEB_MEMBER_FUNCT();
	m_adsc_api.getFrameDim(frame_dim);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::reset()
{
	DEB_MEMBER_FUNCT();
	stopAcq();

	init();
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::setStoredImageDark(bool value)
{
	DEB_MEMBER_FUNCT();
	m_use_stored_dark = value ? 1 : 0;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
bool Camera::getStoredImageDark(void)
{
	return m_use_stored_dark;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::setImageKind(int image_kind)
{
	DEB_MEMBER_FUNCT();
	m_flp_kind = image_kind;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
int Camera::getImageKind(void)
{
	DEB_MEMBER_FUNCT();
	return m_flp_kind;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::setLastImage(int last_image)
{
	DEB_MEMBER_FUNCT();
	m_last_image = last_image;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
int Camera::getLastImage(void)
{
	DEB_MEMBER_FUNCT();
	return m_last_image;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::setFileName(const std::string& name)
{
	DEB_MEMBER_FUNCT();
	m_filename = name;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
const std::string& Camera::getFileName(void)
{
	DEB_MEMBER_FUNCT();
	return m_filename;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::setImagePath(const std::string& path)
{
	DEB_MEMBER_FUNCT();
	m_path = path;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
const std::string& Camera::getImagePath(void)
{
	DEB_MEMBER_FUNCT();
	return m_path;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::setHeaderParameters(const std::string& header)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "setHeaderParameters [" << const_cast<char*>(header.c_str()) << "]";
	CCDSetFilePar(FLP_HEADERPARAMS, const_cast<char*>(header.c_str()));}

//-----------------------------------------------------
//
//-----------------------------------------------------
Camera::Status Camera::getStatus()
{
	DEB_MEMBER_FUNCT();
	int thread_status = m_thread.getStatus();
	switch (thread_status)
	{
		case AdscThread::Ready:
			return Camera::Ready;
		case AdscThread::Exposure:
			return Camera::Exposure;
		case AdscThread::Readout:
			return Camera::Readout;
		case AdscThread::Latency:
			return Camera::Latency;
		default:
			throw LIMA_HW_EXC(Error, "Invalid thread status");
	}
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::startAcq()
{

	DEB_MEMBER_FUNCT();

	float f_stat_time;
	float f_stat_wavelength = 1.0; // need at least a default value

	m_thread.m_force_stop = false; //ugly but works

//
//	interface to the ccd library, starting image.
//
//	These items will change from image to image
//
	f_stat_time = m_exp_time;
	std::string full_file_name = m_path + m_filename;
	CCDSetFilePar(FLP_TIME, (char *) &f_stat_time);
	CCDSetFilePar(FLP_FILENAME, (char *) full_file_name.c_str());
	CCDSetFilePar(FLP_KIND, (char *) &m_flp_kind);
	CCDSetFilePar(FLP_WAVELENGTH, (char *) &f_stat_wavelength);
//
//	These items do not change (at the present time)
//
	CCDSetHwPar(HWP_BIN, (char *) &m_bin_used);
	CCDSetHwPar(HWP_ADC, (char *) &m_adc_used);
	CCDSetHwPar(HWP_SAVE_RAW, (char *) &m_output_raws);
	CCDSetHwPar(HWP_NO_XFORM, (char *) &m_no_transform);
	CCDSetHwPar(HWP_STORED_DARK, (char *) &m_use_stored_dark);

	CCDStartExposure();
	while (DTC_STATE_EXPOSING != CCDState())
	{
		if (DTC_STATE_ERROR == CCDState())
		{
			DEB_TRACE() << "Error returned from CCDStartExposure()";
			throw LIMA_HW_EXC(Error, "Error returned from CCDStartExposure()");
			return;
		}
	}

	m_thread.sendCmd(AdscThread::StartAcq);
	m_thread.waitNotStatus(AdscThread::Ready);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::stopAcq()
{
	DEB_MEMBER_FUNCT();
	m_thread.m_force_stop = true; //ugly but works
//
//	Interface to CCD library here.
//
	CCDStopExposure();
	while (DTC_STATE_IDLE != CCDState())
	{
		if (DTC_STATE_ERROR == CCDState())
		{
			DEB_TRACE() << "Error returned from CCDStopExposure()";
			throw LIMA_HW_EXC(Error, "Error returned from CCDStopExposure()");
			return;
		}
	}
	if (5 == m_flp_kind)
		m_last_image = 1;
	else
		m_last_image = 0;

	CCDSetFilePar(FLP_LASTIMAGE, (char *) &m_last_image);
	CCDGetImage();

	m_thread.sendCmd(AdscThread::StopAcq);
	m_thread.waitStatus(AdscThread::Ready);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
int Camera::getNbAcquiredFrames()
{
	DEB_MEMBER_FUNCT();
	return m_thread.getNbAcquiredFrames();
}

//-----------------------------------------------------
