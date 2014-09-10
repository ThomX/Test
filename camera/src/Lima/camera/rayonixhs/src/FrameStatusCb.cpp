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
#include "RayonixHsCamera.h"

#include "FrameStatusCb.h"

using namespace lima;
using namespace lima::RayonixHs;

//-----------------------------------------------------
// @brief Ctor
//-----------------------------------------------------
FrameStatusCb::FrameStatusCb(Camera *cam, volatile bool &acquiring)
  : m_cam(cam),
    mRawFramesRcvd(0),
    mBgFramesRcvd(0),
    mCorrFramesRcvd(0),
    m_acquiring(acquiring) {

        DEB_CONSTRUCTOR();
}

//void registerCallbackAcqComplete(void(*func)()) {
//   acquistionComplete = func;
//}

//-----------------------------------------------------
// @brief Ctor
//-----------------------------------------------------
FrameStatusCb::~FrameStatusCb() {
        DEB_DESTRUCTOR();
}

//-----------------------------------------------------
// @brief callback on new raw frame, increase raw counter
//-----------------------------------------------------
void FrameStatusCb::RawFrameReady(int frame_number, const craydl::RxFrame *rx_frame) {
	DEB_MEMBER_FUNCT();
        ++mRawFramesRcvd; 
        DEB_TRACE() << "Have received " << mRawFramesRcvd << " raw frames.";
}

//-----------------------------------------------------
// @brief callback on new background frame, increase the bkg counter
//-----------------------------------------------------
void FrameStatusCb::BackgroundFrameReady(const craydl::RxFrame *frame_p) {
	DEB_MEMBER_FUNCT();
        ++mBgFramesRcvd; 
        DEB_TRACE() << "Have received " << mBgFramesRcvd << " background frames."; 
}

//-----------------------------------------------------
// @brief callback on new corrected frame, increase the corr counter
//-----------------------------------------------------
void FrameStatusCb::FrameReady(int frame_number, const craydl::RxFrame *rx_frame) {
	DEB_MEMBER_FUNCT();
        ++mCorrFramesRcvd; 
        DEB_TRACE() << "Have received " << mCorrFramesRcvd << " corrected frames."; 
        m_cam->frameReady(rx_frame); 
}

//-----------------------------------------------------
// @brief callback on acquisition aborted (stop)
//-----------------------------------------------------
void FrameStatusCb::FrameAborted(int frame_number) {
	DEB_MEMBER_FUNCT();
        DEB_TRACE() << "Frame #" << frame_number << " aborted!";
}

//-----------------------------------------------------
// @brief callback on a acquired frame error
//-----------------------------------------------------
void FrameStatusCb::FrameError(int frame_number, int error_code, const std::string& error_string) { 
	DEB_MEMBER_FUNCT();
        DEB_ERROR() << "Frame error with frame #" << frame_number << ":"; 
	DEB_ERROR() << error_string;
}

//-----------------------------------------------------
// @brief callback on sequence started
//-----------------------------------------------------
void FrameStatusCb::SequenceStarted() { 
	DEB_MEMBER_FUNCT();
        m_acquiring = true; 
        DEB_TRACE() << "Sequence started."; 
}

//-----------------------------------------------------
// @brief callback on sequence ended
//-----------------------------------------------------
void FrameStatusCb::SequenceEnded() {
	DEB_MEMBER_FUNCT();
        m_acquiring = false;
        std::cout << "Sequence ended."; 
        m_cam->setStatus(DETECTOR_STATUS_IDLE);
}

//-----------------------------------------------------
// @brief callback on exposure started
//-----------------------------------------------------
void FrameStatusCb::ExposureStarted(int frame) { 
	DEB_MEMBER_FUNCT();
        DEB_TRACE() << "Exposure started."; 
}
//-----------------------------------------------------
// @brief callback on exposure ended
//-----------------------------------------------------
void FrameStatusCb::ExposureEnded(int frame) { 
	DEB_MEMBER_FUNCT();
        DEB_TRACE() << "Exposure ended."; 
}

//-----------------------------------------------------
// @brief callback on readout started
//-----------------------------------------------------
void FrameStatusCb::ReadoutStarted(int frame) {
	DEB_MEMBER_FUNCT();
}
//-----------------------------------------------------
// @brief callback on readout ended
//-----------------------------------------------------
void FrameStatusCb::ReadoutEnded(int frame) {
	DEB_MEMBER_FUNCT();
}

//-----------------------------------------------------
// @brief callback on a frame completed, set the camera status to IDLE
//-----------------------------------------------------
void FrameStatusCb::FrameCompleted(int frame) {
	DEB_MEMBER_FUNCT();
	// in case of IntTrigMult detector must be set to Idle after a frame is completed
	m_cam->setStatus(DETECTOR_STATUS_IDLE);
}

//-----------------------------------------------------
// @brief return the raw frame count
//-----------------------------------------------------
int FrameStatusCb::frameCountRaw() const { 
	DEB_MEMBER_FUNCT();
	return mRawFramesRcvd; 
}
//-----------------------------------------------------
// @brief return the background frame count
//-----------------------------------------------------
int FrameStatusCb::frameCountBackground() const { 
	DEB_MEMBER_FUNCT();
	return mBgFramesRcvd; 
}
//-----------------------------------------------------
// @brief return the corrected frame count
//-----------------------------------------------------
int FrameStatusCb::frameCountCorrected() const {
	DEB_MEMBER_FUNCT();
	return mCorrFramesRcvd; 
}

//-----------------------------------------------------
// @brief reset all the frame counts: raw, bkg and corrected
//-----------------------------------------------------
void FrameStatusCb::resetFrameCounts() { 
	DEB_MEMBER_FUNCT();
	mRawFramesRcvd = mBgFramesRcvd = mCorrFramesRcvd = 0; 
}
