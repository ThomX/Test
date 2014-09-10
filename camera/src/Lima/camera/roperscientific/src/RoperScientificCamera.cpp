#include <cstdlib>

#if ! defined (WIN32)
#  include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h> 
# include <netdb.h>
#else
# include <winsock2.h>
#include <unistd.h>
#endif

#include "Exceptions.h"


#include "RoperScientificCamera.h"

using namespace lima;
using namespace lima::RoperScientific;

//---------------------------------------------------------------------------------------
//! Camera::CameraThread::CameraThread()
//---------------------------------------------------------------------------------------
Camera::CameraThread::CameraThread(Camera& cam)
		: m_cam(&cam)
{
	DEB_MEMBER_FUNCT();	
	DEB_TRACE()<<"CameraThread::CameraThread - BEGIN";	
	m_cam->m_acq_frame_nb = 0;
	m_force_stop = false;
	DEB_TRACE()<<"CameraThread::CameraThread - END";	
}

//---------------------------------------------------------------------------------------
//! Camera::CameraThread::start()
//---------------------------------------------------------------------------------------
void Camera::CameraThread::start()
{
	DEB_MEMBER_FUNCT();	
	DEB_TRACE()<<"CameraThread::start - BEGIN";	
	CmdThread::start();
	waitStatus(Ready);
	DEB_TRACE()<<"CameraThread::start - END";		
}

//---------------------------------------------------------------------------------------
//! Camera::CameraThread::init()
//---------------------------------------------------------------------------------------
void Camera::CameraThread::init()
{
	DEB_MEMBER_FUNCT();	
	DEB_TRACE()<<"CameraThread::init - BEGIN";		
	setStatus(Ready);
	DEB_TRACE()<<"CameraThread::init - END";		
}

//---------------------------------------------------------------------------------------
//! Camera::CameraThread::execCmd()
//---------------------------------------------------------------------------------------
void Camera::CameraThread::execCmd(int cmd)
{
	DEB_MEMBER_FUNCT();	
	DEB_TRACE()<<"CameraThread::execCmd - BEGIN";		
	int status = getStatus();
	switch (cmd)
	{
		case StartAcq:
			if (status != Ready)
				throw LIMA_HW_EXC(InvalidValue, "Not Ready to StartAcq");
			execStartAcq();
		break;
	}
	DEB_TRACE()<<"CameraThread::execCmd - END";		
}

//---------------------------------------------------------------------------------------
//! Camera::CameraThread::execStartAcq()
//---------------------------------------------------------------------------------------
void Camera::CameraThread::execStartAcq()
{

	DEB_MEMBER_FUNCT();
	int16 status;
	uns32 not_needed;
	int acq_frame_nb;
	void *f_address;
	DEB_TRACE()<<"CameraThread::execStartAcq - BEGIN";
	setStatus(Exposure);

	StdBufferCbMgr& buffer_mgr = m_cam->m_buffer_ctrl_obj.getBuffer();
	buffer_mgr.setStartTimestamp(Timestamp::now());

	int nb_frames = m_cam->m_nb_frames;
	int& frame_nb = m_cam->m_acq_frame_nb;

	m_cam->m_acq_frame_nb = 0;
	
	acq_frame_nb = 0;
	bool continueAcq = true;
	while(continueAcq && (!m_cam->m_nb_frames || m_cam->m_acq_frame_nb < m_cam->m_nb_frames))
	{		
		DEB_TRACE()<<"Start the sequence according to acquisition mode.";			
		if (m_cam->m_int_acq_mode == 0)
		{
			DEB_TRACE()<<"Acquisition mode : STANDARD.";

			DEB_TRACE()<<"start acquisition.";
			if(pl_exp_start_seq(m_cam->m_handle, m_cam->m_frame) == PV_FAIL)
			{
				char Err[ERROR_MSG_LEN];
				pl_error_message(pl_error_code(), Err);
				THROW_HW_ERROR(Error) << Err;				
			}

			/* wait for data or error */
			DEB_TRACE()<<"Wait for data or error ...";
			while(pl_exp_check_status(m_cam->m_handle, &status, &not_needed) && (status != READOUT_COMPLETE && status != READOUT_FAILED))
			{
				/* Ugly, but work. TODO sooner : use something like yield();*/
				usleep(9000);
				/* Ugly, but work. TODO sooner : must use something like this mWaitCond.signal(); */
				if (m_force_stop)
					goto ForceTheStop;
			}

			/* Check Error Codes */
			if (status == READOUT_FAILED)
			{
				DEB_TRACE()<<"Status is FAIL.";
				char Err[ERROR_MSG_LEN];
				pl_error_message(pl_error_code(), Err);
				setStatus(Fault);
				continueAcq = false;
				THROW_HW_ERROR(Error) << Err;
				break;
			}
		}
		else if (m_cam->m_int_acq_mode == 1)
		{
			DEB_TRACE()<<"Acquisition mode : CONTINUOUS.";
			DEB_TRACE()<<"start acquisition & Wait for data or error ....";
			while (pl_exp_check_cont_status(m_cam->m_handle, &status, &not_needed, &not_needed) && (status != READOUT_COMPLETE && status != READOUT_FAILED))
			{
				/* Ugly, but work. TODO sooner : use something like yield();*/
				usleep(9000);
				/* Ugly, but work. TODO sooner : must use something like this mWaitCond.signal(); */
				if (m_force_stop)
					goto ForceTheStop;
			}

			/* Check Error Codes */
			if (status == READOUT_FAILED)
			{
				DEB_TRACE()<<"Status is FAIL.";					
				char Err[ERROR_MSG_LEN];
				pl_error_message(pl_error_code(), Err);
				setStatus(Fault);
				continueAcq = false;					
				THROW_HW_ERROR(Error) << Err;
				break;
			}
			if (pl_exp_get_oldest_frame(m_cam->m_handle, &f_address))
			{
				m_cam->m_frame = (uns16*) f_address;
			}
		}
		else if (m_cam->m_int_acq_mode == 2)
		{

			DEB_TRACE()<<"Acquisition mode : FOCUS.";
			DEB_TRACE()<<"start acquisition & Wait for data or error ....";
			while (pl_exp_check_cont_status(m_cam->m_handle, &status, &not_needed, &not_needed) && (status != READOUT_COMPLETE && status != READOUT_FAILED))
			{
				/* Ugly, but work. TODO sooner : use something like yield();*/
				usleep(9000);
				/* Ugly, but work. TODO sooner : must use something like this mWaitCond.signal(); */
				if (m_force_stop)
					goto ForceTheStop;
			}

			/* Check Error Codes */
			if (status == READOUT_FAILED)
			{
				DEB_TRACE()<<"Status is FAIL.";					
				char Err[ERROR_MSG_LEN];
				pl_error_message(pl_error_code(), Err);
				setStatus(Fault);
				continueAcq = false;					
				THROW_HW_ERROR(Error) << Err;
				break;
			}
			if (pl_exp_get_latest_frame(m_cam->m_handle, &f_address))
			{
				m_cam->m_frame = (uns16*) f_address;
			}
		}

		ForceTheStop:
		if (m_force_stop)
		{
			//abort the current acqusition et set internal driver state to IDLE
			pl_exp_abort(m_cam->m_handle,CCS_HALT);
			continueAcq = false;
			m_force_stop = false;
			setStatus(Ready);
			break;
		}
		
		DEB_TRACE()<<"Prepare the Frame ptr - "<< DEB_VAR1(frame_nb);
		setStatus(Readout);
		
		DEB_TRACE()<<"copy data into the Frame ptr - "<< DEB_VAR1(m_cam->m_size);		
		void *ptr = buffer_mgr.getFrameBufferPtr(frame_nb);
		memcpy((unsigned short *)ptr, (unsigned short *)m_cam->m_frame, m_cam->m_size); //we need a nb of BYTES .
		buffer_mgr.setStartTimestamp(Timestamp::now());
		
		DEB_TRACE()<<"Declare a new Frame Ready.";		
		HwFrameInfoType frame_info;
		frame_info.acq_frame_nb = acq_frame_nb;
		buffer_mgr.newFrameReady(frame_info);

		acq_frame_nb++;
		nb_frames--;
		m_cam->m_acq_frame_nb = acq_frame_nb;	
		
	} /* End while */

	DEB_TRACE()<<"Stop the sequence according to acquisition mode.";
	if (m_cam->m_int_acq_mode == 0)
	{
		DEB_TRACE()<<"Acquisition mode : STANDARD.";			
		/* Finish the sequence */
		DEB_TRACE()<<"Finish the sequence.";		
		pl_exp_finish_seq(m_cam->m_handle, m_cam->m_frame, 0);
	}
	else
	{
		DEB_TRACE()<<"Acquisition mode : CONTINUOUS.";
		/* Stop the acquisition */
		DEB_TRACE()<<"Stop the acquisition.";
		pl_exp_stop_cont(m_cam->m_handle, CCS_HALT);

		/* Finish the sequence */
		DEB_TRACE()<<"Finish the sequence.";		
		pl_exp_finish_seq(m_cam->m_handle, m_cam->m_pr_buffer, 0);
		if (m_cam->m_pr_buffer)
		{
			delete[] m_cam->m_pr_buffer;
			m_cam->m_pr_buffer = 0;
		}
	}

	/*Uninit the sequence */
	DEB_TRACE()<<"Uninit the sequence.";		
	pl_exp_uninit_seq();

	if (m_cam->m_frame)
	{
		delete[] m_cam->m_frame;
		m_cam->m_frame = 0;
	}

	setStatus(Ready);
	DEB_TRACE()<<"CameraThread::execStartAcq - END";	
}

//---------------------------------------------------------------------------------------
//! Camera::CameraThread::getNbAcquiredFrames()
//---------------------------------------------------------------------------------------
int Camera::CameraThread::getNbAcquiredFrames()
{
	return m_cam->m_acq_frame_nb;
}
	
//---------------------------------------------------------------------------------------
//! Camera::Camera()
//---------------------------------------------------------------------------------------
Camera::Camera(int camNum):
	m_thread(*this),
	m_roi_s1(0),
	m_roi_s2(2047),
	m_roi_sbin(1),
	m_roi_p1(0),
	m_roi_p2(2047),
	m_roi_pbin(1),
	m_size(2048*2048*2),
	m_nb_frames(1),
	m_exposure(1.0),
	m_trigger_mode(TIMED_MODE),
	m_shutter_mode(OPEN_NEVER),
	m_int_acq_mode(0),
	m_adc_rate("")
{
	DEB_CONSTRUCTOR();
	DEB_TRACE()<<"Camera::Camera";

    union
    {
        flt64 dval;
        uns32 ulval;
        int32 lval;
        uns16 usval;
        int16 sval;
        uns8 ubval;
        int8 bval;
    } currentVal;

	/*close pvcam library in case of !! */
	DEB_TRACE()<<"close pvcam library in case of !";		
	pl_pvcam_uninit();

	/* Open pvcam library */
	DEB_TRACE()<<"Open pvcam library.";		
	if (pl_pvcam_init() == PV_FAIL)
	{
		char Err[ERROR_MSG_LEN];
		pl_error_message(pl_error_code(), Err);
		THROW_HW_ERROR(Error) << Err;
	}

	/* Get camera name */
	DEB_TRACE()<<"Get camera name.";
	if (pl_cam_get_name(camNum, m_name) == PV_FAIL)
	{
		char Err[ERROR_MSG_LEN];
		pl_error_message(pl_error_code(), Err);
		THROW_HW_ERROR(Error) << Err;
	}

	/* Open comunication with the camera */
	DEB_TRACE()<<"Open comunication with the camera.";
	if (pl_cam_open(m_name, &m_handle, OPEN_EXCLUSIVE) == PV_FAIL)
	{
		char Err[ERROR_MSG_LEN];
		pl_error_message(pl_error_code(), Err);
		THROW_HW_ERROR(Error) << Err;
	}

	/* Get camera max width */
	DEB_TRACE()<<"Get camera max width.";
	if(pl_get_param(m_handle, PARAM_SER_SIZE, ATTR_DEFAULT, (void *) &m_max_width)==PV_FAIL)
	{
		char Err[ERROR_MSG_LEN];
		pl_error_message(pl_error_code(), Err);
		THROW_HW_ERROR(Error) << Err;
	}

	/* Get camera max height */
	DEB_TRACE()<<"Get camera max height.";
	if(pl_get_param(m_handle, PARAM_PAR_SIZE, ATTR_DEFAULT, (void *) &m_max_height)==PV_FAIL)
	{
		char Err[ERROR_MSG_LEN];
		pl_error_message(pl_error_code(), Err);
		THROW_HW_ERROR(Error) << Err;
	}

	/* Get camera pixel depth */
	DEB_TRACE()<<"Get camera pixel depth.";
	if(pl_get_param(m_handle, PARAM_BIT_DEPTH, ATTR_CURRENT, &m_depth)==PV_FAIL)
	{			
		char Err[ERROR_MSG_LEN];
		pl_error_message(pl_error_code(), Err);
		THROW_HW_ERROR(Error) << Err;
	}
	
	/*default EXPOSURE unit is the microsec */
	DEB_TRACE()<<"Set EXPOSURE unit to microsec.";
	int16 param2 = EXP_RES_ONE_MICROSEC;
	if(pl_set_param(m_handle, PARAM_EXP_RES_INDEX, (void *) &param2) == PV_FAIL)
	{
		char Err[ERROR_MSG_LEN];
		pl_error_message(pl_error_code(), Err);
		THROW_HW_ERROR(Error) << Err;
	}

	/* Get shutter mode */
	DEB_TRACE()<<"Get shutter mode.";
	if(pl_get_param(m_handle, PARAM_SHTR_OPEN_MODE, ATTR_CURRENT, (void *) &m_shutter_mode)==PV_FAIL)
	{
		char Err[ERROR_MSG_LEN];
		pl_error_message(pl_error_code(), Err);
		THROW_HW_ERROR(Error) << Err;
	}	

    // read pixel time:
    if (pl_get_param(m_handle, PARAM_PIX_TIME, ATTR_CURRENT,(void *) &currentVal.usval)==PV_FAIL)
    {
        /* error occurred calling function print out error label */
		char Err[ERROR_MSG_LEN];
		pl_error_message(pl_error_code(), Err);
		THROW_HW_ERROR(Error) << Err;
    }
    long pixelTime = currentVal.usval;
    
	//refresh m_adc_rate
    std::ostringstream osRate;
    osRate << "Rate = "<<1000.0/pixelTime << " MHz ";//rate = (1000/pixelTime) is in MHz
	m_adc_rate = osRate.str();
	
	m_thread.start();
}

//---------------------------------------------------------------------------------------
//! Camera::~Camera()
//---------------------------------------------------------------------------------------
Camera::~Camera()
{
	DEB_DESTRUCTOR();
	DEB_TRACE()<<"Camera::~Camera";	
	stopAcq();

	DEB_TRACE()<<"Close communication with the camera.";
	pl_cam_close(m_handle);
	DEB_TRACE()<<"close pvcam library.";
	pl_pvcam_uninit();
}

//---------------------------------------------------------------------------------------
//! Camera::getStatus()
//---------------------------------------------------------------------------------------
Camera::Status Camera::getStatus()
{
	DEB_MEMBER_FUNCT();

	int thread_status = m_thread.getStatus();

	DEB_RETURN() << DEB_VAR1(thread_status);

	switch (thread_status)
	{
		case CameraThread::Ready:
			return Camera::Ready;
		case CameraThread::Exposure:
			return Camera::Exposure;
		case CameraThread::Readout:
			return Camera::Readout;
		case CameraThread::Latency:
			return Camera::Latency;
		default:
			throw LIMA_HW_EXC(Error, "Invalid thread status");
	}
}

//---------------------------------------------------------------------------------------
//! Camera::setNbFrames()
//---------------------------------------------------------------------------------------
void Camera::setNbFrames(int nb_frames)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE()<<"Camera::setNbFrames - "<<DEB_VAR1(nb_frames);	
	if (nb_frames < 0)
		throw LIMA_HW_EXC(InvalidValue, "Invalid nb of frames");

	m_nb_frames = nb_frames;
}

//---------------------------------------------------------------------------------------
//! Camera::getNbFrames()
//---------------------------------------------------------------------------------------
void Camera::getNbFrames(int& nb_frames)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE()<<"Camera::getNbFrames";	
	DEB_RETURN() << DEB_VAR1(m_nb_frames);

	nb_frames = m_nb_frames;
}

//---------------------------------------------------------------------------------------
//! Camera::getDetectorModel()
//---------------------------------------------------------------------------------------
void Camera::getDetectorModel(std::string& model)
{
	DEB_MEMBER_FUNCT();
	DEB_RETURN() << DEB_VAR1(m_name);
	model = m_name;
}

//---------------------------------------------------------------------------------------
//! Camera::CameraThread::getNbAcquiredFrames()
//---------------------------------------------------------------------------------------
int Camera::getNbAcquiredFrames()
{
	return m_acq_frame_nb;
}

//---------------------------------------------------------------------------------------
//! Camera::prepareAcq()
//---------------------------------------------------------------------------------------
void Camera::prepareAcq()
{
	DEB_MEMBER_FUNCT();

	rgn_type region = { 0,2047, 1, 0, 2047, 1 };

	DEB_TRACE()<<"Define Bin/Roi parameters.";	
	setBinRoiParameters(&region);


	/* Init a sequence  */
	DEB_TRACE()<<"Init the sequence according to acquisition mode.";	
	if (pl_exp_init_seq() == PV_FAIL)
	{
		char Err[ERROR_MSG_LEN];
		pl_error_message(pl_error_code(), Err);
		THROW_HW_ERROR(Error) << Err;		
	}

	if (m_int_acq_mode == 0)
	{
		DEB_TRACE()<<"Acquisition mode : STANDARD.";
		/* Set the region, exposure mode and exposure time.			  
		 Second argument (exp_total) -> Number of images to take.
		 size is the returned number of bytes in the pixel stream */
		DEB_TRACE()<<"Setup acquisition.";
		if (pl_exp_setup_seq(m_handle, 1, 1, &region, m_trigger_mode, (long) m_exposure, &m_size) == PV_FAIL)
		{
			char Err[ERROR_MSG_LEN];
			pl_error_message(pl_error_code(), Err);
			THROW_HW_ERROR(Error) << Err;
		}

		DEB_TRACE()<<"Frame size in Bytes - "<< DEB_VAR1(m_size);
	}
	else if (m_int_acq_mode == 1)
	{
		DEB_TRACE()<<"Acquisition mode : CONTINUOUS.";
		DEB_TRACE()<<"Setup acquisition.";
		if (pl_exp_setup_cont(m_handle, 1, &region, m_trigger_mode, (long) m_exposure, &m_size, CIRC_NO_OVERWRITE)==PV_FAIL)
		{
			char Err[ERROR_MSG_LEN];
			pl_error_message(pl_error_code(), Err);
			THROW_HW_ERROR(Error) << Err;
		}
	}
	else if (m_int_acq_mode == 2)
	{
		DEB_TRACE()<<"Acquisition mode : FOCUS.";
		DEB_TRACE()<<"Setup acquisition.";
		if (pl_exp_setup_cont(m_handle, 1, &region, m_trigger_mode, (long) m_exposure, &m_size, CIRC_OVERWRITE)==PV_FAIL)
		{			
			char Err[ERROR_MSG_LEN];
			pl_error_message(pl_error_code(), Err);
			THROW_HW_ERROR(Error) << Err;
		}
	}

	/*Define circular buffer for acquisitions CONTINUOUS & FOCUS*/
	if (m_int_acq_mode != 0)
	{
		DEB_TRACE()<<"Allocate a circular buffer.";	
		m_pr_buffer = new unsigned short[(m_size/2)*3]; // set up a circular buffer of 5 frames. we need a nb of pixels => factor (1/2)
		pl_exp_start_cont(m_handle, m_pr_buffer, m_size);
	}
	
	try
	{
		DEB_TRACE() << "Allocate Memory for the Frame.";
		m_frame = new unsigned short[(m_size/2)]; // we need a nb of pixels => factor (1/2)
		memset((unsigned short*)m_frame, 0, (m_size/2));
	}
	catch (std::exception& e)
	{
		DEB_TRACE()<<"Allocating memory is FAIL.";
		THROW_HW_ERROR(Error) << e.what();
	}
}

//---------------------------------------------------------------------------------------
//! Camera::startAcq()
//---------------------------------------------------------------------------------------
void Camera::startAcq()
{
	DEB_MEMBER_FUNCT();

	m_thread.m_force_stop = false;
	m_acq_frame_nb = 0;

	m_thread.sendCmd(CameraThread::StartAcq);
	m_thread.waitNotStatus(CameraThread::Ready);
}

//---------------------------------------------------------------------------------------
//! Camera::stopAcq()
//---------------------------------------------------------------------------------------
void Camera::stopAcq()
{
	DEB_MEMBER_FUNCT();
	
	m_thread.m_force_stop = true;
	
	m_thread.sendCmd(CameraThread::StopAcq);
	m_thread.waitStatus(CameraThread::Ready);

}

//---------------------------------------------------------------------------------------
//! Camera::reset()
//---------------------------------------------------------------------------------------
void Camera::reset()
{
	DEB_MEMBER_FUNCT();
	//@todo maybe something to do!
}

//---------------------------------------------------------------------------------------
//! Camera::getExpTime()
//---------------------------------------------------------------------------------------
void Camera::getExpTime(double& exp_time)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE()<<"Camera::getExpTime";	
	//	AutoMutex aLock(m_cond.mutex());
	exp_time = m_exposure/1E6;//the lima standrad unit is second AND default detector unit is fixed to microsec
	DEB_RETURN() << DEB_VAR1(exp_time);
}

//---------------------------------------------------------------------------------------
//! Camera::setExpTime()
//---------------------------------------------------------------------------------------
void Camera::setExpTime(double  exp_time)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE()<<"Camera::setExpTime - "<<DEB_VAR1(exp_time);		
	
	m_exposure = exp_time*1E6;//default detector unit is microsec	
}

//---------------------------------------------------------------------------------------
//! Camera::getGain()
//---------------------------------------------------------------------------------------
long Camera::getGain()
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE()<<"Camera::getGain";	
	int16 param;

	if(pl_get_param(m_handle, PARAM_GAIN_INDEX, ATTR_CURRENT, (void *) &param)==PV_FAIL)
	{
		char Err[ERROR_MSG_LEN];
		pl_error_message(pl_error_code(), Err);
		THROW_HW_ERROR(Error) << Err;
	}

	DEB_RETURN() << DEB_VAR1(param);
	return param;
}

//---------------------------------------------------------------------------------------
//! Camera::setGain()
//---------------------------------------------------------------------------------------
void Camera::setGain(long gain)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE()<<"Camera::setGain - "<<DEB_VAR1(gain);	

	int16 param;

	param = (int16) gain;

	if(pl_set_param(m_handle, PARAM_GAIN_INDEX, (void *) &param)==PV_FAIL)
	{
		char Err[ERROR_MSG_LEN];
		pl_error_message(pl_error_code(), Err);
		THROW_HW_ERROR(Error) << Err;
	}
}

//---------------------------------------------------------------------------------------
//! Camera::setTrigMode()
//---------------------------------------------------------------------------------------
void Camera::setTrigMode(TrigMode  mode)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE()<<"Camera::setTrigMode - "<<DEB_VAR1(mode);		
	DEB_PARAM() << DEB_VAR1(mode);
	switch (mode)
	{
		case IntTrig:
			m_trigger_mode = TIMED_MODE; 				// 0 (int. trigger)
		break;

		case ExtTrigMult:
			m_trigger_mode = STROBED_MODE; 				// 1 STROBED_MODE (ext. trigger)
		break;

		case ExtTrigSingle:
			//m_trigger_mode = TRIGGER_FIRST_MODE; 		// 3  (ext. trigger)
			THROW_HW_ERROR(Error) << "Cannot change the Trigger Mode of the camera, this mode is not managed !";
			break;
		case ExtGate:
			//m_trigger_mode = BULB_MODE; 				// 2  (ext. trigger)
			THROW_HW_ERROR(Error) << "Cannot change the Trigger Mode of the camera, this mode is not managed !";
			break;
		default :
			THROW_HW_ERROR(Error) << "Cannot change the Trigger Mode of the camera, this mode is not managed !";
		break;
	}
}

//---------------------------------------------------------------------------------------
//! Camera::setTrigMode()
//---------------------------------------------------------------------------------------
void Camera::getTrigMode(TrigMode& mode)
{
	DEB_MEMBER_FUNCT();
	//DEB_TRACE()<<"Camera::getTrigMode";		
	switch (m_trigger_mode)
	{
		case TIMED_MODE:
			mode = IntTrig;
		break;
	
		case TRIGGER_FIRST_MODE:
			mode = ExtTrigSingle;
		break;
	
		case STROBED_MODE:
			mode = ExtTrigMult;
		break;
	
		case BULB_MODE:
			mode = ExtGate;
		break;
	}
	DEB_RETURN() << DEB_VAR1(mode);	
}

//---------------------------------------------------------------------------------------
//! Camera::getADCRate()
//---------------------------------------------------------------------------------------
const std::string& Camera::getADCRate(void)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE()<<"Camera::getADCRate";	

    return m_adc_rate;
}

//---------------------------------------------------------------------------------------
//! Camera::getSpeedTableIndex()
//---------------------------------------------------------------------------------------
unsigned Camera::getSpeedTableIndex(void)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE()<<"Camera::getSpeedTableIndex";	
    
	uns16 tableIndex = 0;

	//read the current tableIndex
    if (pl_get_param(m_handle, PARAM_SPDTAB_INDEX, ATTR_CURRENT, &tableIndex)==PV_FAIL)
    {
        /* error occurred calling function print out error label */
		char Err[ERROR_MSG_LEN];
		pl_error_message(pl_error_code(), Err);
		THROW_HW_ERROR(Error) << Err;
    }
    return tableIndex;
}

//---------------------------------------------------------------------------------------
//! Camera::setSpeedTableIndex()
//---------------------------------------------------------------------------------------
void Camera::setSpeedTableIndex(unsigned idx)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE()<<"Camera::setSpeedTableIndex - "<<DEB_VAR1(idx);	
    union
    {
        flt64 dval;
        uns32 ulval;
        int32 lval;
        uns16 usval;
        int16 sval;
        uns8 ubval;
        int8 bval;
    } currentVal;

	int16 tableIndex;
    int16 spdTableMaxIndex;
    if (pl_get_param(m_handle, PARAM_SPDTAB_INDEX, ATTR_MAX, &spdTableMaxIndex) == PV_FAIL)
    {
        /* error occurred calling function print out error label */
		char Err[ERROR_MSG_LEN];
		pl_error_message(pl_error_code(), Err);
		THROW_HW_ERROR(Error) << Err;
    }

    //Only if mode is <= Speed Table Max Index, otherwise ERROR
    if (idx<=spdTableMaxIndex)
    {
        if (pl_set_param(m_handle, PARAM_SPDTAB_INDEX, &idx)==PV_FAIL)
        {
            char Err[ERROR_MSG_LEN];
            pl_error_message(pl_error_code(), Err);
            THROW_HW_ERROR(Error) << Err;            
        }
    }
    else
    {
        THROW_HW_ERROR(Error) << "Cannot change the Speed Table Index of the camera, inexistant index = "<<idx;        
    }

    // read pixel time:
    if (pl_get_param(m_handle, PARAM_PIX_TIME, ATTR_CURRENT,(void *) &currentVal.usval)==PV_FAIL)
    {
        /* error occurred calling function print out error label */
		char Err[ERROR_MSG_LEN];
		pl_error_message(pl_error_code(), Err);
		THROW_HW_ERROR(Error) << Err;
    }
    long pixelTime = currentVal.usval;
    
	//refresh m_adc_rate
    std::ostringstream osRate;
    osRate << "Rate = "<<1000.0/pixelTime << " MHz ";//rate = (1000/pixelTime) is in MHz
	m_adc_rate = osRate.str();
    return;
}

//---------------------------------------------------------------------------------------
//! Camera::setShutterMode()
//---------------------------------------------------------------------------------------
void Camera::setShutterMode(int mode)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE()<<"Camera::setShutterMode - "<<DEB_VAR1(mode);		
	DEB_PARAM() << DEB_VAR1(mode);
	switch (mode)
	{
		case OPEN_NEVER:
		case OPEN_PRE_EXPOSURE:
		case OPEN_PRE_TRIGGER:
		case OPEN_PRE_SEQUENCE:		
		case OPEN_NO_CHANGE:		
			// configure shutter mode	
			if(pl_set_param(m_handle, PARAM_SHTR_OPEN_MODE, &mode)==PV_FAIL)
			{
				char Err[ERROR_MSG_LEN];
				pl_error_message(pl_error_code(), Err);
				THROW_HW_ERROR(Error) << Err;
			}			
			//Only if everything is OK
			m_shutter_mode = mode;
		break;
		default:
			THROW_HW_ERROR(Error) << "Cannot change the Shutter Mode of the camera, this mode is not managed !";
		break;
	}
}

//---------------------------------------------------------------------------------------
//! Camera::getShutterMode()
//---------------------------------------------------------------------------------------
void Camera::getShutterMode(int& mode)
{
	DEB_MEMBER_FUNCT();
	//DEB_TRACE()<<"Camera::getShutterMode";		
	mode = m_shutter_mode;
	DEB_RETURN() << DEB_VAR1(mode);	
}


//---------------------------------------------------------------------------------------
//! Camera::getTemperature()
//---------------------------------------------------------------------------------------
double Camera::getTemperature()
{
	DEB_MEMBER_FUNCT();
	//DEB_TRACE()<<"Camera::getTemperature";	
	int16 param;
	double temperature;

	if(pl_get_param(m_handle, PARAM_TEMP, ATTR_CURRENT, (void *) &param)==PV_FAIL)
	{
		char Err[ERROR_MSG_LEN];
		pl_error_message(pl_error_code(), Err);
		THROW_HW_ERROR(Error) << Err;
	}	
	
	temperature = param / 100;

	DEB_RETURN() << DEB_VAR1(temperature);

	return temperature;

}

//---------------------------------------------------------------------------------------
//! Camera::getTemperatureSetPoint()
//---------------------------------------------------------------------------------------
double Camera::getTemperatureSetPoint()
{
	DEB_MEMBER_FUNCT();
	//DEB_TRACE()<<"Camera::getTemperatureSetPoint";	
	int16 param;
	double temperature;

	if(pl_get_param(m_handle, PARAM_TEMP_SETPOINT, ATTR_CURRENT, (void *) &param)==PV_FAIL)
	{
		char Err[ERROR_MSG_LEN];
		pl_error_message(pl_error_code(), Err);
		THROW_HW_ERROR(Error) << Err;
	}	
	temperature = param / 100;

	DEB_RETURN() << DEB_VAR1(temperature);
	return temperature;

}

//---------------------------------------------------------------------------------------
//! Camera::setTemperatureSetPoint()
//---------------------------------------------------------------------------------------
void Camera::setTemperatureSetPoint(double temperature)
{

	DEB_MEMBER_FUNCT();
	DEB_TRACE()<<"Camera::setTemperatureSetPoint - "<<DEB_VAR1(temperature);

	int16 param;
	param = (int16)(temperature * 100);
	
	if(pl_set_param(m_handle, PARAM_TEMP_SETPOINT, (void *) &param)==PV_FAIL)
	{
		char Err[ERROR_MSG_LEN];
		pl_error_message(pl_error_code(), Err);
		THROW_HW_ERROR(Error) << Err;
	}	
}

//---------------------------------------------------------------------------------------
//! Camera::getInternalAcqMode()
//---------------------------------------------------------------------------------------
std::string Camera::getInternalAcqMode()
{

	DEB_MEMBER_FUNCT();
	std::string mode = "UNKNOWN";
	if (m_int_acq_mode == 0)
	{
		DEB_RETURN() << DEB_VAR1("STANDARD");
		mode = "STANDARD";
	}
	else if (m_int_acq_mode == 1)
	{
		DEB_RETURN() << DEB_VAR1("CONTINUOUS");
		mode = "CONTINUOUS";
	}
	else if (m_int_acq_mode == 2)
	{
		DEB_RETURN() << DEB_VAR1("FOCUS");
		mode = "FOCUS";
	}
	return mode;
}

//---------------------------------------------------------------------------------------
//! Camera::setInternalAcqMode()
//---------------------------------------------------------------------------------------
void Camera::setInternalAcqMode(std::string mode)
{

	DEB_MEMBER_FUNCT();
	DEB_PARAM() << DEB_VAR1(mode);

	if (mode.compare("STANDARD") == 0)
	{
		m_int_acq_mode = 0;
	}
	else if (mode.compare("CONTINUOUS") == 0)
	{
		m_int_acq_mode = 1;
	}
	else if (mode.compare("FOCUS") == 0)
	{
		m_int_acq_mode = 2;
	}
	else
	{
		THROW_HW_ERROR(Error) << "Incorrect Internal Acquisition mode !";
	}
}

//---------------------------------------------------------------------------------------
//! Camera::getMaxWidth()
//---------------------------------------------------------------------------------------
uns16 Camera::getMaxWidth()
{
	DEB_MEMBER_FUNCT();

	DEB_RETURN() << DEB_VAR1(m_max_width);

	return m_max_width;

}
	
//---------------------------------------------------------------------------------------
//! Camera::getMaxHeight()
//---------------------------------------------------------------------------------------
uns16 Camera::getMaxHeight()
{

	DEB_MEMBER_FUNCT();

	DEB_RETURN() << DEB_VAR1(m_max_height);

	return m_max_height;

}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::setImageType(ImageType type)
{
    DEB_MEMBER_FUNCT();
	DEB_TRACE()<<"Camera::setImageType - "<<DEB_VAR1(type);		
	//Only 16 bits princeton detector are already managed !
	switch( type )
	{              
		case Bpp16:
			m_depth = 16;
		break;
		default:
			THROW_HW_ERROR(Error) << "This pixel format of the camera is not managed, only 16 bits cameras are already managed!";
		break;
	}
}

//---------------------------------------------------------------------------------------
//! Camera::getImageType()
//---------------------------------------------------------------------------------------
void Camera::getImageType(ImageType& type)
{
    DEB_MEMBER_FUNCT();	
	switch(m_depth)
	{		
		case 16 : type  = Bpp16;
		break;
		default:
			THROW_HW_ERROR(Error) << "This pixel format of the camera is not managed, only 16 bits cameras are already managed!";
		break;				
	}
	return;
}

//---------------------------------------------------------------------------------------
//! Camera::setBinRoiParameters()
//---------------------------------------------------------------------------------------
void Camera::setBinRoiParameters(rgn_type* roi)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE()<<"Camera::setBinRoiParameters";	
	roi->s1 = m_roi_s1;
	roi->s2 = m_roi_s2;
	roi->sbin = m_roi_sbin;
	roi->p1 = m_roi_p1;
	roi->p2 = m_roi_p2;
	roi->pbin = m_roi_pbin;
	
	DEB_TRACE() << DEB_VAR1(roi->s1);
	DEB_TRACE() << DEB_VAR1(roi->s2);
	DEB_TRACE() << DEB_VAR1(roi->sbin);
	DEB_TRACE() << DEB_VAR1(roi->p1);
	DEB_TRACE() << DEB_VAR1(roi->p2);
	DEB_TRACE() << DEB_VAR1(roi->pbin);
}

//---------------------------------------------------------------------------------------
//! Camera::setFullFrame()
//---------------------------------------------------------------------------------------
void Camera::setFullFrame(rgn_type* roi)
{
	uns16 param;

	DEB_MEMBER_FUNCT();
	DEB_TRACE()<<"Camera::setFullFrame";	
	roi->s1 = 0;
	pl_get_param(m_handle, PARAM_SER_SIZE, ATTR_DEFAULT, (void *) &param);
	roi->s2 = param - 1;
	roi->sbin = 1;
	roi->p1 = 0;
	pl_get_param(m_handle, PARAM_PAR_SIZE, ATTR_DEFAULT, (void *) &param);
	roi->p2 = param - 1;
	roi->pbin = 1;
}

//---------------------------------------------------------------------------------------
//! Camera::setBin()
//---------------------------------------------------------------------------------------
void Camera::setBin(const Bin& bin)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE()<<"Camera::setBin";	
	DEB_PARAM() << DEB_VAR1(bin);
	
	m_roi_sbin = bin.getX();
	m_roi_pbin = bin.getY();
}

//---------------------------------------------------------------------------------------
//! Camera::getBin()
//---------------------------------------------------------------------------------------
void Camera::getBin(Bin& bin)
{

	DEB_MEMBER_FUNCT();
	DEB_TRACE()<<"Camera::getBin";	
	Bin tmp_bin(m_roi_sbin, m_roi_pbin);
	bin = tmp_bin;

	DEB_RETURN() << DEB_VAR1(bin);	
}

//---------------------------------------------------------------------------------------
//! Camera::checkBin()
//---------------------------------------------------------------------------------------
void Camera::checkBin(Bin& bin)
{

	DEB_MEMBER_FUNCT();
	DEB_TRACE()<<"Camera::checkBin";	
	DEB_PARAM() << DEB_VAR1(bin);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::checkRoi(const Roi& set_roi, Roi& hw_roi)
{
    DEB_MEMBER_FUNCT();
	DEB_TRACE()<<"Camera::checkRoi";	
    DEB_PARAM() << DEB_VAR1(set_roi);
    hw_roi = set_roi;

    DEB_RETURN() << DEB_VAR1(hw_roi);
}

//---------------------------------------------------------------------------------------
//! Camera::getRoi()
//---------------------------------------------------------------------------------------
void Camera::getRoi(Roi& hw_roi)
{

	DEB_MEMBER_FUNCT();
	DEB_TRACE()<<"Camera::getRoi";	
	Point point1(m_roi_s1, m_roi_p1);
	Point point2(m_roi_s2, m_roi_p2);
	Roi tmp_roi(point1, point2);

	hw_roi = tmp_roi;

	DEB_RETURN() << DEB_VAR1(hw_roi);
}

//---------------------------------------------------------------------------------------
//! Camera::setRoi()
//---------------------------------------------------------------------------------------
void Camera::setRoi(const Roi& set_roi)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE()<<"Camera::setRoi";		
	DEB_PARAM() << DEB_VAR1(set_roi);
	if (!set_roi.isActive())
	{
		DEB_TRACE()<<"Roi is not Enabled";				
	}
	else
	{
		//To avoid a double binning, API pvcam apply Roi & Binning together AND Lima Too !		
		Bin aBin;
		getBin(aBin);
		Roi UnbinnedRoi = set_roi.getUnbinned(aBin);
		Point tmp_top = UnbinnedRoi.getTopLeft();
		////
		
		m_roi_s1 = tmp_top.x;
		m_roi_p1 = tmp_top.y;

		Point tmp_bottom = UnbinnedRoi.getBottomRight();

		m_roi_s2 = tmp_bottom.x;
		m_roi_p2 = tmp_bottom.y;
	}	
}


//---------------------------------------------------------------------------------------
