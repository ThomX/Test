#include <algorithm>
#include "Debug.h"
#include "MythenInterface.h"


using namespace lima;
using namespace lima::Mythen;
using namespace std;

/*******************************************************************
 * \brief DetInfoCtrlObj constructor
 *******************************************************************/
DetInfoCtrlObj::DetInfoCtrlObj(Camera& cam)
				:m_cam(cam)
{
	DEB_CONSTRUCTOR();
}

//-----------------------------------------------------
//
//-----------------------------------------------------
DetInfoCtrlObj::~DetInfoCtrlObj()
{
	DEB_DESTRUCTOR();
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void DetInfoCtrlObj::getMaxImageSize(Size& size)
{
	DEB_MEMBER_FUNCT();
	// get the max image size
	getDetectorImageSize(size);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void DetInfoCtrlObj::getDetectorImageSize(Size& size)
{
  DEB_MEMBER_FUNCT();
  // get the max image size of the detector
  
  int width;
  int height;

  width = m_cam.getMaxWidth();
  height = m_cam.getMaxHeight();
    
  size= Size(width,height);
}


//-----------------------------------------------------
//
//-----------------------------------------------------
void DetInfoCtrlObj::getDefImageType(ImageType& image_type)
{
	DEB_MEMBER_FUNCT();
	getCurrImageType(image_type);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void DetInfoCtrlObj::getCurrImageType(ImageType& image_type)
{
	DEB_MEMBER_FUNCT();
	image_type= Bpp32; // unsigned long
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void DetInfoCtrlObj::setCurrImageType(ImageType image_type)
{
	DEB_MEMBER_FUNCT();
	ImageType valid_image_type;
	getDefImageType(valid_image_type);
	if (image_type != valid_image_type)
		THROW_HW_ERROR(Error) << "Cannot change to "  << DEB_VAR2(image_type, valid_image_type);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void DetInfoCtrlObj::getPixelSize(double& x_size,double& y_size)
{
	DEB_MEMBER_FUNCT();
	x_size = y_size = 172.0;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void DetInfoCtrlObj::getDetectorType(std::string& type)
{
	DEB_MEMBER_FUNCT();
	type  = "Mythen";

}

//-----------------------------------------------------
//
//-----------------------------------------------------
void DetInfoCtrlObj::getDetectorModel(std::string& model)
{
  DEB_MEMBER_FUNCT();
  model = "Mythen";
}



void DetInfoCtrlObj::registerMaxImageSizeCallback(
						HwMaxImageSizeCallback& cb)
{
	m_mis_cb_gen.registerMaxImageSizeCallback(cb);
}

void DetInfoCtrlObj::unregisterMaxImageSizeCallback(
						HwMaxImageSizeCallback& cb)
{
	m_mis_cb_gen.unregisterMaxImageSizeCallback(cb);
}

void DetInfoCtrlObj::
     MaxImageSizeCallbackGen::setMaxImageSizeCallbackActive(bool cb_active)
{
}

/*******************************************************************
 * \brief SyncCtrlObj constructor
 *******************************************************************/

SyncCtrlObj::SyncCtrlObj(Camera& cam, DetInfoCtrlObj& det)
  : HwSyncCtrlObj(), m_cam(cam)
{
  DEB_CONSTRUCTOR();
}

//-----------------------------------------------------
//
//-----------------------------------------------------
SyncCtrlObj::~SyncCtrlObj()
{
}

//-----------------------------------------------------
//
//-----------------------------------------------------
bool SyncCtrlObj::checkTrigMode(TrigMode trig_mode)
{
	bool valid_mode = false;
	switch (trig_mode)
	{
	case IntTrig:
	case ExtTrigSingle:
	case ExtGate:
		valid_mode = true;
		break;

	default:
		valid_mode = false;
	}
	return valid_mode;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void SyncCtrlObj::setTrigMode(TrigMode trig_mode)
{
  DEB_MEMBER_FUNCT();
  
  int trig;

  switch(trig_mode)
    {
    case IntTrig        : trig = 0; // "TIMED_MODE (int. trigger)"
      break;
    case IntTrigMult    : trig = 6; // "INT_STROBE_MODE";
      break;
    case ExtTrigSingle  : trig = 3; // "TRIGGER_FIRST_MODE (ext. trigger)
      break;
    case ExtTrigMult    : trig = 1; // "STROBED_MODE (ext. trigger)
      break;
    case ExtGate        : trig = 2; // "BULB_MODE (ext. trigger)"
      break;
    case ExtStartStop   : trig = 5; // "VARIABLE_TIMED_MODE (int. trigger)"
      break;
    }
  
  m_cam.setTrigMode(trig);
  
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void SyncCtrlObj::getTrigMode(TrigMode& trig_mode)
{

  int trig = m_cam.getTrigMode();

  switch(trig)
    {
    case 0     :   trig_mode = IntTrig;
      break;
    case 6     :   trig_mode = IntTrigMult;
      break;
    case 3     :   trig_mode = ExtTrigSingle;
      break;
    case 1     :   trig_mode = ExtTrigMult;
      break;
    case 2     :   trig_mode = ExtGate;
      break;
    case 5     :   trig_mode = ExtStartStop;
      break;
    }

}

//-----------------------------------------------------
//
//-----------------------------------------------------
void SyncCtrlObj::setExpTime(double exp_time)
{

	m_cam.setExposure(exp_time);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void SyncCtrlObj::getExpTime(double& exp_time)
{
	exp_time = m_cam.exposure();
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void SyncCtrlObj::setLatTime(double lat_time)
{
	//@TODO
}

void SyncCtrlObj::getLatTime(double& lat_time)
{
	//@TODO
}


void SyncCtrlObj::setNbHwFrames(int  nb_frames)
{
  m_cam.setNbFrames(nb_frames);
}

void SyncCtrlObj::getNbHwFrames(int& nb_frames)
{
  m_cam.getNbFrames(nb_frames);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void SyncCtrlObj::getValidRanges(ValidRangesType& valid_ranges)
{
	double min_time = 10e-9;
	double max_time = 1e6;
	valid_ranges.min_exp_time = min_time;
	valid_ranges.max_exp_time = max_time;
	valid_ranges.min_lat_time = min_time;
	valid_ranges.max_lat_time = max_time;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void SyncCtrlObj:: prepareAcq()
{

  
	double exposure =  m_cam.exposure();
	cout<<"> exposure = "<<exposure<<endl;
//	double latency = m_det.getMinLatency();
	//cout<<"> latency = "<<latency<<endl;
	//	double exposure_period = exposure + latency;
	//cout<<"> exposure_period = "<<exposure_period<<endl;
	//	m_cam.setExposurePeriod(exposure_period);

	TrigMode trig_mode;
	getTrigMode(trig_mode);
	//	int nb_frames = (trig_mode == IntTrigMult)?1:m_nb_frames;
	//	m_cam.setNbImagesInSequence(nb_frames);

}

/*******************************************************************
 * \brief BinCtrlObj constructor
 *******************************************************************/

BinCtrlObj::BinCtrlObj(Camera& cam)
	: m_cam(cam)
{
  printf("Teresa: constructor binctrl \n");
}

BinCtrlObj::~BinCtrlObj()
{
}

void BinCtrlObj::setBin(const Bin& bin)
{
  printf("Teresa: Interface setBin \n");
  m_cam.setBin(bin);
}

void BinCtrlObj::getBin(Bin& bin)
{
  printf("Teresa: Interface getBin \n");
  m_cam.getBin(bin);
}

void BinCtrlObj::checkBin(Bin& bin)
{
  
  printf("Teresa: Interface checkBin \n");
  m_cam.checkBin(bin);
}



/*******************************************************************
 * \brief Hw Interface constructor
 *******************************************************************/

Interface::Interface(Camera& cam)
			: 	m_cam(cam),
				m_det_info(cam),
                                m_sync(cam, m_det_info), 
				m_bin(cam)
{
	DEB_CONSTRUCTOR();

	HwDetInfoCtrlObj *det_info = &m_det_info;
	m_cap_list.push_back(HwCap(det_info));

	HwBufferCtrlObj *buffer = cam.getBufferCtrlObj();
	m_cap_list.push_back(HwCap(buffer));

	HwSyncCtrlObj *sync = &m_sync;
	m_cap_list.push_back(HwCap(sync));

	HwBinCtrlObj *bin = &m_bin;
	m_cap_list.push_back(HwCap(bin));	

	
}

//-----------------------------------------------------
//
//-----------------------------------------------------
Interface::~Interface()
{
	DEB_DESTRUCTOR();
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Interface::getCapList(HwInterface::CapList &cap_list) const
{
	DEB_MEMBER_FUNCT();
	cap_list = m_cap_list;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Interface::reset(ResetLevel reset_level)
{
	DEB_MEMBER_FUNCT();
	DEB_PARAM() << DEB_VAR1(reset_level);

	stopAcq();

	Size image_size;
	m_det_info.getMaxImageSize(image_size);
	ImageType image_type;
	m_det_info.getDefImageType(image_type);
	FrameDim frame_dim(image_size, image_type);

	HwBufferCtrlObj *buffer = m_cam.getBufferCtrlObj();
	buffer->setFrameDim(frame_dim);

	buffer->setNbConcatFrames(1);
	buffer->setNbBuffers(1);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Interface::prepareAcq()
{
	DEB_MEMBER_FUNCT();

	m_cam.prepareAcq();     

}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Interface::startAcq()
{
	DEB_MEMBER_FUNCT();
	m_cam.startAcq();
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Interface::stopAcq()
{
	DEB_MEMBER_FUNCT();
	m_cam.stopAcq();
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Interface::getStatus(StatusType& status)
{

  Camera::Status camera_status = m_cam.getStatus();
  switch (camera_status) {
  case Camera::Ready:
    status.acq = AcqReady;
    status.det = DetIdle;
    break;
  case Camera::Exposure:
    status.det = DetExposure;
    goto Running;
  case Camera::Readout:
    status.det = DetReadout;
    goto Running;
  case Camera::Latency:
    status.det = DetLatency;
  Running:
    status.acq = AcqRunning;
    break;
  }
  status.det_mask = DetExposure | DetReadout | DetLatency;
  

  //	Communication::Status com_status = Communication::OK;
  //	com_status = m_cam.status();
  /*
	if(com_status == Camera::ERROR)
	{
		status.det = DetFault;
		status.acq = AcqFault;		
	}
	else
	{
		if(com_status != Communication::OK)
		{
			status.det = DetExposure;
			status.acq = AcqRunning;		
		}
		else
		{
			status.det = DetIdle;
            int lastAcquiredFrame = -1;//self.__buffer.getLastAcquiredFrame()
            int requestNbFrame = -1;
			m_sync.getNbHwFrames(requestNbFrame);
			if(lastAcquiredFrame >= 0 && lastAcquiredFrame == (requestNbFrame - 1))
				status.acq = AcqReady;
			else
				status.acq = AcqRunning;
		}
	}
	status.det_mask = DetExposure|DetFault;
  */
}

int Interface::getNbAcquiredFrames()
{
  DEB_MEMBER_FUNCT();

  return getNbHwAcquiredFrames();
}



//-----------------------------------------------------
//
//-----------------------------------------------------
int Interface::getNbHwAcquiredFrames()
{
  DEB_MEMBER_FUNCT(); 
  int aNbAcquiredFrames;

  aNbAcquiredFrames = m_cam.getNbAcquiredFrames();
  
  DEB_RETURN() << DEB_VAR1(aNbAcquiredFrames);
  return aNbAcquiredFrames;
}



