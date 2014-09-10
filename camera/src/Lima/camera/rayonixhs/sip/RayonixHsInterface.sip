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
namespace RayonixHs
{

  class Interface: HwInterface 
  {
%TypeHeaderCode
#include <RayonixHsInterface.h>
#include <RayonixHsCamera.h>
using namespace lima;
%End
    public:
    Interface(RayonixHs::Camera*);
    virtual ~Interface();

    //virtual void getCapList(CapList &) const;
    virtual void getCapList(std::vector<HwCap> &cap_list /Out/) const;
    virtual void reset(ResetLevel reset_level);
    virtual void prepareAcq();
    virtual void startAcq();
    virtual void stopAcq();
    virtual void getStatus(StatusType& status /Out/);

    virtual int getNbAcquiredFrames();
    virtual int getNbHwAcquiredFrames();

    // Rayonix specific Hw Interface
    void setFrameMode(RayonixHs::FrameMode mode);
    void getFrameMode(RayonixHs::FrameMode &mode /Out/);
    void setFrameTriggerSignalType(RayonixHs::SignalType signal_type);
    void getFrameTriggerSignalType(RayonixHs::SignalType &signal_type /Out/);
    void setSequenceGateSignalType(RayonixHs::SignalType signal_type);
    void getSequenceGateSignalType(RayonixHs::SignalType &signal_type /Out/);

   void setOutputSignalType(RayonixHs::OutputChannel output, RayonixHs::SignalType signal_type);
   void getOutputSignalType(RayonixHs::OutputChannel output, RayonixHs::SignalType &signal_type /Out/);
   void setOutputSignalID(RayonixHs::OutputChannel output, RayonixHs::SignalID signal_id);
   void getOutputSignalID(RayonixHs::OutputChannel output, RayonixHs::SignalID &signal_id /Out/);

    void setElectronicShutterEnabled(bool enable);
    void getElectronicShutterEnabled(bool &enable /Out/);
	
    void setCoolerTemperatureSetpoint(double temperature);
    void getCoolerTemperatureSetpoint(double &temperature /Out/);
    void setSensorTemperatureSetpoint(double temperature);
    void getSensorTemperatureSetpoint(double &temperature /Out/);
    void getSensorTemperature(double &temperature /Out/);
    void setCooler(bool start);
    void getCooler(bool &start /Out/);
    void setVacuumValve(bool open);
    void getVacuumValve(bool &open /Out/);
    void getNewBackgroundNeeded(bool& needed /Out/);
    void acquireNewBackground(bool block = false, int n_backgrounds = 0);

  };

}; // namespace RayonixHs

