//###########################################################################
// This file is part of LImA, a Library for Image Acquisition
//
// Copyright (C) : 2009-2014
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
#ifndef RayonixHsINTERFACE_H
#define RayonixHsINTERFACE_H

#include "HwInterface.h"
#include "RayonixHsCamera.h"

namespace lima {
namespace RayonixHs {

class DetInfoCtrlObj;
class SyncCtrlObj;
class BinCtrlObj;
class ShutterCtrlObj;

class Interface: public HwInterface {
	DEB_CLASS_NAMESPC(DebModCamera, "Interface", "RayonixHs");

	public:
		Interface(Camera*);
		virtual ~Interface();

		virtual void getCapList(CapList &) const;

		virtual void reset(ResetLevel reset_level);
		virtual void prepareAcq();
		virtual void startAcq();
		virtual void stopAcq();
		virtual void getStatus(StatusType& status);

		virtual int getNbAcquiredFrames();
		virtual int getNbHwAcquiredFrames();

		// Rayonix specific Hw Interface
		void setFrameMode(FrameMode mode);
		void getFrameMode(FrameMode &mode);
		void setFrameTriggerSignalType(SignalType signal_type);
		void getFrameTriggerSignalType(SignalType &signal_type);
		void setSequenceGateSignalType(SignalType signal_type);
		void getSequenceGateSignalType(SignalType &signal_type);

		void setOutputSignalType(OutputChannel output, SignalType signal_type);
		void getOutputSignalType(OutputChannel output, SignalType &signal_type);
		void setOutputSignalID(OutputChannel output, SignalID signal_id);
		void getOutputSignalID(OutputChannel output, SignalID &signal_id);

		void setElectronicShutterEnabled(bool enable);
		void getElectronicShutterEnabled(bool &enable);
		
		void getCoolerTemperatureSetpoint(double &temperature);
		void setCoolerTemperatureSetpoint(double temperature);
		void getSensorTemperatureSetpoint(double &temperature);
		void setSensorTemperatureSetpoint(double temperature);
		void getSensorTemperature(double &temperature);
		void setCooler(bool start);
		void getCooler(bool &start);
		void setVacuumValve(bool open);
		void getVacuumValve(bool &open);
		void getNewBackgroundNeeded(bool& needed);
		void acquireNewBackground(bool block = false, int n_backgrounds = 0);		
	private:
		Camera* m_cam;
		DetInfoCtrlObj* m_det_info;
		HwBufferCtrlObj* m_buffer;
		SyncCtrlObj* m_sync;
		BinCtrlObj* m_bin;
		ShutterCtrlObj* m_shutter;
};

} // namespace RayonixHs
} // namespace lima

#endif // RayonixHsINTERFACE_H
