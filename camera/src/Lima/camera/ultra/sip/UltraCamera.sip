//###########################################################################
// This file is part of LImA, a Library for Image Acquisition
//
// Copyright (C) : 2009-2013
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

namespace Ultra
{

  /*******************************************************************
   * \class Camera
   * \brief object controlling the ultra detector. 
   *******************************************************************/
  class Camera
  {
%TypeHeaderCode
#include <UltraCamera.h>
#include <string>
%End

  public:

	Camera(std::string headname, std::string hostname, int tcpPort, int udpPort, int npixels);
	~Camera();

	void init();
	void reset();
	void prepareAcq();
	void startAcq();
	void stopAcq();
	int getNbHwAcquiredFrames();

	// -- detector info object
	void getImageType(ImageType& type /Out/);
	void setImageType(ImageType type);

	void getDetectorType(std::string& type /Out/);
	void getDetectorModel(std::string& model /Out/);
	void getDetectorImageSize(Size& size /Out/);
	void getPixelSize(double& sizex, double& sizey /Out/);

	// -- Buffer control object
	HwBufferCtrlObj* getBufferCtrlObj();

	//-- Synch control object
	void setTrigMode(TrigMode mode);
	void getTrigMode(TrigMode& mode /Out/);

	void setExpTime(double exp_time);
	void getExpTime(double& exp_time /Out/);

	void setLatTime(double lat_time);
	void getLatTime(double& lat_time /Out/);
        
	// not implemented yet
	//void getExposureTimeRange(double& min_expo /Out/, double& max_expo /Out/) const;
	//void getLatTimeRange(double& min_lat /Out/, double& max_lat /Out/) const;

	void setNbFrames(int nb_frames);
	void getNbFrames(int& nb_frames /Out/);

	bool isAcqRunning() const;

	///////////////////////////
	// -- ultra specific functions
	///////////////////////////

	void getHeadColdTemp(float &value /Out/);
	void getHeadHotTemp(float &value /Out/);
	void getTecColdTemp(float &value /Out/);
	void getTecSupplyVolts(float &value /Out/);
	void getAdcPosSupplyVolts(float &value /Out/);
	void getAdcNegSupplyVolts(float &value /Out/);
	void getVinPosSupplyVolts(float &value /Out/);
	void getVinNegSupplyVolts(float &value /Out/);
	void getHeadADCVdd(float &value /Out/);
	void getHeadVdd(float &value /Out/);
	void setHeadVdd(float voltage);
	void getHeadVref(float &value /Out/);
	void setHeadVref(float voltage);
	void getHeadVrefc(float &value /Out/);
	void setHeadVrefc(float voltage);
	void getHeadVpupref(float &value /Out/);
	void setHeadVpupref(float voltage);
	void getHeadVclamp(float &value /Out/);
	void setHeadVclamp(float voltage);
	void getHeadVres1(float &value /Out/);
	void setHeadVres1(float voltage);
	void getHeadVres2(float &value /Out/);
	void setHeadVres2(float voltage);
	void getHeadVTrip(float &value /Out/);
	void setHeadVTrip(float voltage);
	void getFpgaXchipReg(unsigned int& reg /Out/);
	void setFpgaXchipReg(unsigned int reg);
	void getFpgaPwrReg(unsigned int &reg /Out/);
	void setFpgaPwrReg(unsigned int reg);
	void getFpgaSyncReg(unsigned int &reg /Out/);
	void setFpgaSyncReg(unsigned int reg);
	void getFpgaAdcReg(unsigned int &reg /Out/);
	void setFpgaAdcReg(unsigned int reg);
	void getFrameCount(unsigned int& reg /Out/);
	void getFrameErrorCount(unsigned int& reg /Out/);
	void getHeadPowerEnabled(bool& state /Out/);
	void setHeadPowerEnabled(bool state);
	void getTecPowerEnabled(bool& state /Out/);
	void setTecPowerEnabled(bool state);
	void getBiasEnabled(bool& state /Out/);
	void setBiasEnabled(bool state);
	void getSyncEnabled(bool& state /Out/);
	void setSyncEnabled(bool state);
	void getCalibEnabled(bool& state /Out/);
	void setCalibEnabled(bool state);
	void get8pCEnabled(bool& state /Out/);
	void set8pCEnabled(bool state);
	void getTecOverTemp(bool& state /Out/);
	void saveConfiguration();
	void restoreConfiguration();
	void getAdcOffset(unsigned int channel, float &value /Out/);
	void setAdcOffset(unsigned int channel, float value);
	void getAdcGain(unsigned int channel, float &value /Out/);
	void setAdcGain(unsigned int channel, float value);
	void getAux1(unsigned int& delay, unsigned int& width /Out/);
	void setAux1(unsigned int delay, unsigned int width);
	void getAux2(unsigned int& delay, unsigned int& width /Out/);
	void setAux2(unsigned int delay, unsigned int width);
	void getXchipTiming(unsigned int& delay /Out/, unsigned int& width /Out/, unsigned int& zeroWidth /Out/,
			unsigned int& sampleWidth /Out/, unsigned int& resetWidth /Out/, unsigned int& settlingTime /Out/, unsigned int& xClkHalfPeriod /Out/,
			unsigned int& readoutMode /Out/, unsigned int& shiftDelay /Out/);
	void setXchipTiming(unsigned int delay, unsigned int width, unsigned int zeroWidth,
			unsigned int sampleWidth, unsigned int resetWidth, unsigned int settlingTime, unsigned int xClkHalfPeriod,
			unsigned int readoutMode);
  };
};

