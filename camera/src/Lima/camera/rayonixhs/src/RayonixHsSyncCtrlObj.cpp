//###########################################################################
//// This file is part of LImA, a Library for Image Acquisition
////
//// Copyright (C) : 2009-2014
//// European Synchrotron Radiation Facility
//// BP 220, Grenoble 38043
//// FRANCE
////
//// This is free software; you can redistribute it and/or modify
//// it under the terms of the GNU General Public License as published by
//// the Free Software Foundation; either version 3 of the License, or
//// (at your option) any later version.
////
//// This software is distributed in the hope that it will be useful,
//// but WITHOUT ANY WARRANTY; without even the implied warranty of
//// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//// GNU General Public License for more details.
////
//// You should have received a copy of the GNU General Public License
//// along with this program; if not, see <http://www.gnu.org/licenses/>.
////###########################################################################
#include <sstream>

#include "RayonixHsSyncCtrlObj.h"
#include "RayonixHsCamera.h"

using namespace lima;
using namespace lima::RayonixHs;

//-----------------------------------------------------
// @brief Ctor
//-----------------------------------------------------
SyncCtrlObj::SyncCtrlObj(Camera *cam)
	: m_cam(cam) {

	DEB_CONSTRUCTOR();
}

//-----------------------------------------------------
// @brief Dtor
//-----------------------------------------------------
SyncCtrlObj::~SyncCtrlObj() {
	DEB_DESTRUCTOR();
}

//-----------------------------------------------------
// @brief return True if the mode is supported
//-----------------------------------------------------
bool SyncCtrlObj::checkTrigMode(TrigMode trig_mode) {
	DEB_MEMBER_FUNCT();
	DEB_PARAM() << DEB_VAR1(trig_mode);

	DEB_RETURN() << DEB_VAR1(m_cam->checkTrigMode(trig_mode));
	return m_cam->checkTrigMode(trig_mode);
}

//-----------------------------------------------------
// @brief set the trigger mode
//-----------------------------------------------------
void SyncCtrlObj::setTrigMode(TrigMode trig_mode) {
	DEB_MEMBER_FUNCT();
	DEB_PARAM() << DEB_VAR1(trig_mode);

	if (checkTrigMode(trig_mode)) {
		m_cam->setTrigMode(trig_mode);
	}
	else
		throw LIMA_HW_EXC(NotSupported,"Trigger type not supported");
}

//-----------------------------------------------------
// @brief return the trigger mode
//-----------------------------------------------------
void SyncCtrlObj::getTrigMode(TrigMode &trig_mode) {
        DEB_MEMBER_FUNCT();    
	m_cam->getTrigMode(trig_mode);
}

//-----------------------------------------------------
// @brief set the exposure time
//-----------------------------------------------------
void SyncCtrlObj::setExpTime(double exp_time) {
	DEB_MEMBER_FUNCT();
	DEB_PARAM() << DEB_VAR1(exp_time);

        m_cam->setExpTime(exp_time);
}

//-----------------------------------------------------
// @brief return the exposure time
//-----------------------------------------------------
void SyncCtrlObj::getExpTime(double &exp_time) {
	DEB_MEMBER_FUNCT();

	m_cam->getExpTime(exp_time);

	DEB_RETURN() << DEB_VAR1(exp_time);
}

//-----------------------------------------------------
// @brief set the latency time
//-----------------------------------------------------
void SyncCtrlObj::setLatTime(double lat_time) {
        DEB_MEMBER_FUNCT();    
	m_cam->setLatTime(lat_time);
}
//-----------------------------------------------------
// @brief return the latency time
//-----------------------------------------------------
void SyncCtrlObj::getLatTime(double& lat_time) {
        DEB_MEMBER_FUNCT();    
	m_cam->getLatTime(lat_time);
}

//-----------------------------------------------------
// @brief set the number for frames
//-----------------------------------------------------
void SyncCtrlObj::setNbFrames(int nb_frames) {
	DEB_MEMBER_FUNCT();
	DEB_PARAM() << DEB_VAR1(nb_frames);

	setNbHwFrames(nb_frames);
}

//-----------------------------------------------------
// @brief return the number of frames
//-----------------------------------------------------
void SyncCtrlObj::getNbFrames(int& nb_frames) {
        DEB_MEMBER_FUNCT();    
	getNbHwFrames(nb_frames);
}

//-----------------------------------------------------
// @brief
//-----------------------------------------------------
void SyncCtrlObj::setNbHwFrames(int nb_frames) {
        DEB_MEMBER_FUNCT();    
	m_cam->setNbFrames(nb_frames);
}

//-----------------------------------------------------
// @brief
//-----------------------------------------------------
void SyncCtrlObj::getNbHwFrames(int& nb_frames) {
        DEB_MEMBER_FUNCT();    
	m_cam->getNbFrames(nb_frames);
}

//-----------------------------------------------------
// @brief return the valid timing ranges
//-----------------------------------------------------
void SyncCtrlObj::getValidRanges(ValidRangesType& valid_ranges) {
        DEB_MEMBER_FUNCT();    
	valid_ranges.min_exp_time = 1e-3;
	valid_ranges.max_exp_time = 86400;
	valid_ranges.min_lat_time = 1e-3;
	valid_ranges.max_lat_time = 100;

	DEB_RETURN() << DEB_VAR2(valid_ranges.min_exp_time, valid_ranges.max_exp_time);
	DEB_RETURN() << DEB_VAR2(valid_ranges.min_lat_time, valid_ranges.max_lat_time);
}
