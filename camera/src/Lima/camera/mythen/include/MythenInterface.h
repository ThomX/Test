#ifndef MYTHENINTERFACE_H
#define MYTHENINTERFACE_H

#include "HwInterface.h"
#include "HwBufferMgr.h"
#include "Debug.h"
#include "HwInterface.h"
#include "MythenCamera.h"


using namespace lima;
using namespace std;

namespace lima
{
namespace Mythen
{
class Interface;

/*******************************************************************
 * \class DetInfoCtrlObj
 * \brief Control object providing Mythen detector info interface
 *******************************************************************/

class DetInfoCtrlObj : public HwDetInfoCtrlObj
{
	DEB_CLASS_NAMESPC(DebModCamera, "DetInfoCtrlObj", "Mythen");

public:
	DetInfoCtrlObj(Camera& cam);
	virtual ~DetInfoCtrlObj();

	virtual void getMaxImageSize(Size& max_image_size);
	virtual void getDetectorImageSize(Size& det_image_size);

	virtual void getDefImageType(ImageType& def_image_type);
	virtual void getCurrImageType(ImageType& curr_image_type);
	virtual void setCurrImageType(ImageType  curr_image_type);

	virtual void getPixelSize(double& x_size,double &y_size);
	virtual void getDetectorType(std::string& det_type);
	virtual void getDetectorModel(std::string& det_model);


	virtual void registerMaxImageSizeCallback(HwMaxImageSizeCallback& cb);
	virtual void unregisterMaxImageSizeCallback(HwMaxImageSizeCallback& cb);

private:
	class MaxImageSizeCallbackGen: public HwMaxImageSizeCallbackGen
	{
	protected:
		virtual void setMaxImageSizeCallbackActive(bool cb_active);
	};
	Camera& m_cam;

	MaxImageSizeCallbackGen m_mis_cb_gen;
};


/*******************************************************************
 * \class SyncCtrlObj
 * \brief Control object providing Mythen synchronization interface
 *******************************************************************/

class SyncCtrlObj : public HwSyncCtrlObj
{
	DEB_CLASS_NAMESPC(DebModCamera, "SyncCtrlObj", "Mythen");

public:
	SyncCtrlObj(Camera& cam, DetInfoCtrlObj& det);
	virtual ~SyncCtrlObj();

	virtual bool checkTrigMode(TrigMode trig_mode);
	virtual void setTrigMode(TrigMode  trig_mode);
	virtual void getTrigMode(TrigMode& trig_mode);

	virtual void setExpTime(double  exp_time);
	virtual void getExpTime(double& exp_time);

	virtual void setLatTime(double  lat_time);
	virtual void getLatTime(double& lat_time);

	virtual void setNbHwFrames(int  nb_frames);
	virtual void getNbHwFrames(int& nb_frames);

	virtual void getValidRanges(ValidRangesType& valid_ranges);
	
	void prepareAcq();

private:
	Camera& 		m_cam;
};

/*******************************************************************
 * \class BinCtrlObj
 * \brief Control object providing Mythen binning interface
 *******************************************************************/

class BinCtrlObj : public HwBinCtrlObj
{
 public:
	BinCtrlObj(Camera& cam);
	virtual ~BinCtrlObj();

	virtual void setBin(const Bin& bin);
	virtual void getBin(Bin& bin);
	virtual void checkBin(Bin& bin);

 private:
	Camera& m_cam;
};


/*******************************************************************
 * \class Interface
 * \brief Mythen hardware interface
 *******************************************************************/

class Interface : public HwInterface
{
	DEB_CLASS_NAMESPC(DebModCamera, "MythenInterface", "Mythen");

public:
	Interface(Camera& cam);
	virtual 		~Interface();

	//- From HwInterface
	virtual void 		getCapList(CapList&) const;
	virtual void		reset(ResetLevel reset_level);
	virtual void		prepareAcq();
	virtual void	 	startAcq();
	virtual void	 	stopAcq();
	virtual void	 	getStatus(StatusType& status);
	virtual int             getNbAcquiredFrames();
	virtual int 		getNbHwAcquiredFrames();

private:
	Camera&		m_cam;
	CapList 			m_cap_list;
	DetInfoCtrlObj		m_det_info;
	SyncCtrlObj			m_sync;
	BinCtrlObj     m_bin;
};



} // namespace Mythen
} // namespace lima

#endif // MYTHENINTERFACE_H
