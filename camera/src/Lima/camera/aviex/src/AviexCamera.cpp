#include <cstdlib>


#include "Exceptions.h"


#include "AviexCamera.h"

using namespace lima;
using namespace lima::Aviex;


extern "C"
{
#include "mx_util.h"
#include "mx_record.h"
#include "mx_image.h"
#include "mx_area_detector.h"
#include "mx_driver.h"
}

#define CHECK_MX_STATUS(status, origin) \
if(status.code != MXE_SUCCESS) { \
    std::ostringstream err; \
    err << "Mx returned error code " << (long) status.code << " : " << (const char*) status.message; \
    throw LIMA_HW_EXC(Error, origin) << " - "<<err.str().c_str(); \  
}

#define CHECK_MX_RECORD(record, origin) \
if(record==0) { \
    throw LIMA_HW_EXC(Error, origin) << "MX RECORD is NULL"; \              
}

//---------------------------------------------------------------------------------------
//! Camera::CameraThread::CameraThread()
//---------------------------------------------------------------------------------------
Camera::CameraThread::CameraThread(Camera& cam)
:m_cam(&cam)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "CameraThread::CameraThread - BEGIN";
	m_force_stop = false;
	DEB_TRACE() << "CameraThread::CameraThread - END";
}

//---------------------------------------------------------------------------------------
//! Camera::CameraThread::start()
//---------------------------------------------------------------------------------------
void Camera::CameraThread::start()
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "CameraThread::start - BEGIN";
	CmdThread::start();
	waitStatus(Ready);
	DEB_TRACE() << "CameraThread::start - END";
}

//---------------------------------------------------------------------------------------
//! Camera::CameraThread::init()
//---------------------------------------------------------------------------------------
void Camera::CameraThread::init()
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "CameraThread::init - BEGIN";
	setStatus(Ready);
	DEB_TRACE() << "CameraThread::init - END";
}

//---------------------------------------------------------------------------------------
//! Camera::CameraThread::execCmd()
//---------------------------------------------------------------------------------------
void Camera::CameraThread::execCmd(int cmd)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "CameraThread::execCmd - BEGIN";
	int status = getStatus();
	switch(cmd)
	{
		case StartAcq:
			if(status != Ready)
				throw LIMA_HW_EXC(InvalidValue,"Not Ready to StartAcq");
			execStartAcq();
			break;
		case StartMeasureDark:
			if(status != Ready)
				throw LIMA_HW_EXC(InvalidValue,"Not Ready to StartMeasureDark");
			execStartMeasureCorrection(MXFT_AD_DARK_CURRENT_FRAME);
			break;
		case StartMeasureFloodField:
			if(status != Ready)
				throw LIMA_HW_EXC(InvalidValue,"Not Ready to StartMeasureFloodField");
			execStartMeasureCorrection(MXFT_AD_FLOOD_FIELD_FRAME);
			break;
	}
	DEB_TRACE() << "CameraThread::execCmd - END";
}

//---------------------------------------------------------------------------------------
//! Camera::CameraThread::execStartAcq()
//---------------------------------------------------------------------------------------
void Camera::CameraThread::execStartAcq()
{

	DEB_MEMBER_FUNCT();

	DEB_TRACE() << "CameraThread::execStartAcq - BEGIN";
	bool bTraceAlreadyDone = false;
	mx_status_type mx_status;
	setStatus(Exposure);

	StdBufferCbMgr& buffer_mgr = m_cam->m_buffer_ctrl_obj.getBuffer();
	buffer_mgr.setStartTimestamp(Timestamp::now());

	// Start acquisition on aviex detector
	m_cam->_armDetector();

	/////////////////////////////////////////////////////////////////////////////////////////
	// Loop while :
	//				m_nb_frames is not reached OR 
	//				stop() is requested OR 
	//				to infinity in "live mode"
	/////////////////////////////////////////////////////////////////////////////////////////
	DEB_TRACE() << "CameraThread::execStartAcq - Loop while 'Nb. acquired frames' < " << (m_cam->m_nb_frames) << " ...";
	bool continueAcq = true;
	while(continueAcq && (!m_cam->m_nb_frames || m_cam->m_acq_frame_nb < (m_cam->m_nb_frames - 1)))
	{
		//Force quit the thread if command stop() is launched by client
		if(m_force_stop)
		{
			//abort the current acquisition et set internal driver state to IDLE
			continueAcq = false;
			m_force_stop = false;
			goto ForceTheStop;
		}

		//To prevent flooding of traces
		if(!bTraceAlreadyDone)
		{
			DEB_TRACE() << "\n";
			DEB_TRACE() << "Waiting for the Mx Image Acquisition ...";
			bTraceAlreadyDone = true;
		}

		mx_status_type mx_status;
		long last_frame_num = -1;
		mx_status = mx_area_detector_get_last_frame_number(m_cam->m_mx_record,&last_frame_num);
		CHECK_MX_STATUS(mx_status,"Camera::execStartAcq()");

		//Wait while new Frame is not ready 
		if(last_frame_num <= m_cam->m_acq_frame_nb)
		{
			mx_msleep(1);//sleep 1ms
			continue;
		}

		//New image(s) is ready
		while(last_frame_num > m_cam->m_acq_frame_nb)
		{
			int current_frame_nb = m_cam->m_acq_frame_nb + 1;
			//Prepare frame Lima Ptr ...
			bTraceAlreadyDone = false;
			DEB_TRACE() << "\t- Prepare the Lima Frame ptr - " << DEB_VAR1(current_frame_nb);
			setStatus(Readout);
			void *ptr = buffer_mgr.getFrameBufferPtr(current_frame_nb);

			//Prepare frame Mx Ptr ...
			DEB_TRACE() << "\t- Prepare the Mx Frame ptr - " << DEB_VAR1(current_frame_nb);
			MX_IMAGE_FRAME *image_frame = NULL;

			//Send corrections flags
			DEB_TRACE() << "\t- Send the list of corrections flags to Mx library - correction_flags = " << m_cam->m_correction_flags;
			mx_area_detector_set_correction_flags(m_cam->m_mx_record,m_cam->m_correction_flags);

			//Get the last image
			DEB_TRACE() << "\t- Get the last Frame From Mx - " << DEB_VAR1(current_frame_nb);
			//utility function that make : setup_frame() + readout_frame() + correct_frame() + transfer_frame().
			MX_AREA_DETECTOR* ad = (MX_AREA_DETECTOR*)(m_cam->m_mx_record->record_class_struct);
			mx_status = mx_area_detector_get_frame(m_cam->m_mx_record,current_frame_nb,&(ad->image_frame));
			CHECK_MX_STATUS(mx_status,"Camera::execStartAcq()");
			image_frame = ad->image_frame;

			//copy from the Mx buffer to the Lima buffer
			DEB_TRACE() << "\t- Copy Frame From Mx Ptr into the Lima ptr :";
			size_t nb_bytes_to_copy = m_cam->m_frame_size.getWidth() * m_cam->m_frame_size.getHeight() * sizeof(unsigned short);

			size_t nb_bytes_copied;
			mx_status = mx_image_copy_1d_pixel_array(image_frame,
													(unsigned short *)ptr,
													nb_bytes_to_copy,
													&nb_bytes_copied);
			CHECK_MX_STATUS(mx_status,"Camera::execStartAcq()");

			DEB_TRACE() << "\t- Frame size  = "			<< m_cam->m_frame_size;
			DEB_TRACE() << "\t- NB. Bytes to Copy = "	<< nb_bytes_to_copy;
			DEB_TRACE() << "\t- NB. Copied Bytes  = "	<< nb_bytes_copied;


			buffer_mgr.setStartTimestamp(Timestamp::now());

			//Push the image buffer through Lima 
			DEB_TRACE() << "\t- Declare a Lima new Frame Ready (" << current_frame_nb << ")";
			HwFrameInfoType frame_info;
			frame_info.acq_frame_nb = current_frame_nb;
			buffer_mgr.newFrameReady(frame_info);
			m_cam->m_acq_frame_nb = current_frame_nb;
			DEB_TRACE() << "\n";
		}

	} /* End while */

ForceTheStop:{
			  // stop acquisition
			  DEB_TRACE() << "\n";
			  DEB_TRACE() << "Stop the Acquisition.";
			  CHECK_MX_RECORD(m_cam->m_mx_record,"Camera::execStartAcq()")

			  mx_status = mx_area_detector_stop(m_cam->m_mx_record);
			  CHECK_MX_STATUS(mx_status,"Camera::execStartAcq()")

			  setStatus(Ready);
			  DEB_TRACE() << "CameraThread::execStartAcq - END";
	}
}


//---------------------------------------------------------------------------------------
//! Camera::CameraThread::execStartMeasureCorrection()
//---------------------------------------------------------------------------------------
void Camera::CameraThread::execStartMeasureCorrection(unsigned measure_type)
{

	DEB_MEMBER_FUNCT();

	DEB_TRACE() << "CameraThread::execStartMeasureCorrection - BEGIN";
	mx_status_type mx_status;
	setStatus(Exposure);

	StdBufferCbMgr& buffer_mgr = m_cam->m_buffer_ctrl_obj.getBuffer();
	buffer_mgr.setStartTimestamp(Timestamp::now());

	/////////////////////////////////////////////////////////////////////////////////////////
	// Loop while detector is busy : i.e acuiring Dark and made some corrections
	/////////////////////////////////////////////////////////////////////////////////////////
	DEB_TRACE() << "CameraThread::execStartMeasureCorrection - Loop while 'Detector is Busy ...";
	DEB_TRACE() << "Waiting for the Mx Image Acquisition ...";
	while (m_cam->isBusy())//	See if the acqusition OR computation is still in progress.
	{
		//Force quit the thread if command stop() is launched by client
		if(m_force_stop)
		{
			//abort the current acquisition et set internal driver state to IDLE
			m_force_stop = false;
			goto ForceTheStop;
		}

		/* Sleep for a millisecond */
		mx_msleep(1);
	}

	{
		//New image Dark is ready
		DEB_TRACE() << "\t- Prepare the Lima Frame ptr - " << "The unique et only frame in this Mode";
		setStatus(Readout);
		void *ptr = buffer_mgr.getFrameBufferPtr((m_cam->m_nb_frames - 1));

		// Because, Mx library does not allow transfer_frame for FLOOD_FIELD correctly (size = 4096 instead of 1024 in binnig 4)		
		// -> in case of DARK,  we get the real image from Mx Library (setup_frame+transfer_frame) and publish it through newFrameReady()
		// -> in case of FLOOD, we take a black image (*ptr initialized with 0) and publish it through newFrameReady()
		if(measure_type == MXFT_AD_DARK_CURRENT_FRAME)
		{
			//Prepare frame Mx Ptr ...
			DEB_TRACE() << "\t- Prepare the Mx Frame ptr - " << "The unique et only frame in this Mode";
			MX_IMAGE_FRAME *image_frame = NULL;

			// Get the DARK/FLOOD image
			mx_status = mx_area_detector_setup_frame( m_cam->m_mx_record, &image_frame);
			CHECK_MX_STATUS(mx_status,"Camera::execStartMeasureCorrection()");

			mx_status = mx_area_detector_transfer_frame(m_cam->m_mx_record, measure_type, image_frame);
			CHECK_MX_STATUS(mx_status,"Camera::execStartMeasureCorrection()");

			//copy from the Mx buffer to the Lima buffer
			DEB_TRACE() << "\t- Copy Frame From Mx Ptr into the Lima ptr :";
			size_t nb_bytes_to_copy = m_cam->m_frame_size.getWidth() * m_cam->m_frame_size.getHeight() * sizeof(unsigned short);

			size_t nb_bytes_copied;
			mx_status = mx_image_copy_1d_pixel_array(image_frame,
													(unsigned short *)ptr,
													nb_bytes_to_copy,
													&nb_bytes_copied);
			CHECK_MX_STATUS(mx_status,"Camera::execStartMeasureCorrection()");

			DEB_TRACE() << "\t- Frame size  = "			<< m_cam->m_frame_size;
			DEB_TRACE() << "\t- NB. Bytes to Copy = "	<< nb_bytes_to_copy;
			DEB_TRACE() << "\t- NB. Copied Bytes  = "	<< nb_bytes_copied;
		}

		buffer_mgr.setStartTimestamp(Timestamp::now());

		//@@TODO Push m_nb_frame images buffer through Lima, in fact we need to put ONLY ONE DARK image : but Lima don't allow that !
		for(int i = 0; i < m_cam->m_nb_frames;i++)
		{
			DEB_TRACE() << "\t- Declare a Lima new Frame Ready (" << i << ")";
			HwFrameInfoType frame_info;
			frame_info.acq_frame_nb = i;
			buffer_mgr.newFrameReady(frame_info);
			m_cam->m_acq_frame_nb = i ;
		}
	}

ForceTheStop:{
			  // stop acquisition
			  DEB_TRACE() << "\n";
			  DEB_TRACE() << "Stop the Acquisition.";
			  CHECK_MX_RECORD(m_cam->m_mx_record,"Camera::execStartMeasureCorrection()")

			  mx_status = mx_area_detector_stop(m_cam->m_mx_record);
			  CHECK_MX_STATUS(mx_status,"Camera::execStartMeasureCorrection()")

			  setStatus(Ready);
			  DEB_TRACE() << "CameraThread::execStartMeasureCorrection - END";
	}
}

//---------------------------------------------------------------------------------------
//! Camera::CameraThread::getNbHwAcquiredFrames()
//---------------------------------------------------------------------------------------
int Camera::CameraThread::getNbHwAcquiredFrames()
{
	return (m_cam->m_acq_frame_nb == -1) ? 0 : (m_cam->m_acq_frame_nb + 1);
}

//---------------------------------------------------------------------------------------
//! Camera::Camera()
//---------------------------------------------------------------------------------------
Camera::Camera(const std::string& camName,const std::string& databaseFile):
m_thread(*this),
m_name(camName),
m_database_file_name(databaseFile),
m_trigger_mode(0),
m_max_width(4096),
m_max_height(4096),
m_depth(16),
m_nb_frames(1),
m_mx_record(0),
m_status(""),
m_frame_size(4096,4096),
m_acq_frame_nb(-1),
m_state(Ready),
m_exposure_time(1.0),
m_exposure_multiplier(1.0),
m_latency_time(1.0),
m_gap_multiplier(1.0),
m_initial_delay_time(0.0),
m_readout_delay_time(0.0),
m_readout_speed(false),
m_correction_flags(0),
m_acq_mode_name("ONESHOT")
{
	DEB_CONSTRUCTOR();
	DEB_TRACE() << "Camera::Camera";

	//Open Mx database/Mx Record/ ...
	_open();
	m_thread.start();

}

//---------------------------------------------------------------------------------------
//! Camera::~Camera()
//---------------------------------------------------------------------------------------
Camera::~Camera()
{
	DEB_DESTRUCTOR();
	DEB_TRACE() << "Camera::~Camera";
	//Close Mx database/Mx Record/ ...
	_close();
}

//---------------------------------------------------------------------------------------
//! Camera::getStatus()
//---------------------------------------------------------------------------------------
Camera::Status Camera::getStatus()
{
	DEB_MEMBER_FUNCT();

	int thread_status = m_thread.getStatus();

	DEB_RETURN() << DEB_VAR1(thread_status);

	switch(thread_status)
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
			throw LIMA_HW_EXC(Error,"Invalid thread status");
	}
}

//---------------------------------------------------------------------------------------
//! Camera::setNbFrames()
//---------------------------------------------------------------------------------------
void Camera::setNbFrames(int nb_frames)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "Camera::setNbFrames - " << DEB_VAR1(nb_frames);
	if(nb_frames < 0)
		throw LIMA_HW_EXC(InvalidValue,"Invalid nb of frames");

	m_nb_frames = nb_frames;
}

//---------------------------------------------------------------------------------------
//! Camera::getNbFrames()
//---------------------------------------------------------------------------------------
void Camera::getNbFrames(int& nb_frames)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "Camera::getNbFrames";

	nb_frames = m_nb_frames;
}

//---------------------------------------------------------------------------------------
//! Camera::CameraThread::getNbHwAcquiredFrames()
//---------------------------------------------------------------------------------------
int Camera::getNbHwAcquiredFrames()
{
	DEB_MEMBER_FUNCT();
	//DEB_TRACE()<<"Camera::getNbHwAcquiredFrames";			
	return (m_acq_frame_nb == -1) ? 0 : (m_acq_frame_nb + 1);
}

//---------------------------------------------------------------------------------------
//! Camera::setTrigMode()
//---------------------------------------------------------------------------------------
void Camera::setTrigMode(TrigMode mode)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "Camera::setTrigMode - " << DEB_VAR1(mode);
	DEB_PARAM() << DEB_VAR1(mode);

	CHECK_MX_RECORD(m_mx_record,"Camera::setTrigMode()");
	mx_status_type mx_status;
	switch(mode)
	{
		case IntTrig:
			m_trigger_mode = 0x1;// 0x1 (int. trigger)
			break;

		case ExtTrigSingle:
			m_trigger_mode = 0x2;// 0x2  (ext. trigger)                
			break;

		default:
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
	switch(m_trigger_mode)
	{
		case 0x1:		//(int. trigger)
			mode = IntTrig;
			break;

		case 0x2:		//(ext. trigger)         
			mode = ExtTrigSingle;
			break;
	}
}
//---------------------------------------------------------------------------------------
//! Camera::prepareAcq()
//---------------------------------------------------------------------------------------
void Camera::prepareAcq()
{
	DEB_MEMBER_FUNCT();
	try
	{
		CHECK_MX_RECORD(m_mx_record,"Camera::prepareAcq()");
		mx_status_type mx_status;

		// dh_readout_delay_time
		DEB_TRACE() << "Set register 'dh_readout_delay_time' = " << m_readout_delay_time << " (ms)";
		mx_status = mx_area_detector_set_register(m_mx_record, "dh_readout_delay_time", m_readout_delay_time);
		CHECK_MX_STATUS(mx_status,"Camera::prepareAcq()");

		// dh_initial_delay_time
		DEB_TRACE() << "Set register 'dh_initial_delay_time' = " << m_initial_delay_time << " (ms)";
		mx_status = mx_area_detector_set_register(m_mx_record, "dh_initial_delay_time", m_initial_delay_time);
		CHECK_MX_STATUS(mx_status,"Camera::prepareAcq()");

		// dh_readout_speed
		DEB_TRACE() << "Set register 'dh_readout_speed' = " << m_readout_speed;
		mx_status = mx_area_detector_set_register(m_mx_record, "dh_readout_speed", m_readout_speed);
		CHECK_MX_STATUS(mx_status,"Camera::prepareAcq()");

		DEB_TRACE() << "Get the Frame Size.";
		long xsize = 0, ysize = 0;
		mx_status = mx_area_detector_get_framesize(m_mx_record,&xsize,&ysize);
		CHECK_MX_STATUS(mx_status,"Camera::prepareAcq()");
		m_frame_size = Size(xsize,ysize);

		DEB_TRACE() << "Prepare the the acquisition according to the acquisition mode.";
		if(m_acq_mode_name == "ONESHOT")
		{
			DEB_TRACE() << "Acquisition mode : ONESHOT";
			m_status = "Prepare Acquisition mode : ONESHOT";
			setNbFrames(1);//@@@@ TODO Check this : force nbFrames to 1
			mx_status = mx_area_detector_set_one_shot_mode(	m_mx_record,
														m_exposure_time);
			CHECK_MX_STATUS(mx_status,"Camera::prepareAcq()")
		}
		else if(m_acq_mode_name == "CONTINUOUS")
		{
			DEB_TRACE() << "Acquisition mode : CONTINUOUS";
			m_status = "Prepare Acquisition mode : CONTINUOUS";
			mx_status = mx_area_detector_set_continuous_mode(m_mx_record,
															m_exposure_time);
			CHECK_MX_STATUS(mx_status,"Camera::prepareAcq()")
		}
		else if(m_acq_mode_name == "MULTIFRAME")
		{
			DEB_TRACE() << "Acquisition mode : MULTIFRAME.";
			double frame_time = m_exposure_time + m_latency_time - m_readout_delay_time;
			m_status = "Prepare Acquisition mode : MULTIFRAME";
			mx_status = mx_area_detector_set_multiframe_mode(m_mx_record,
															m_nb_frames,
															m_exposure_time,
															frame_time);
			CHECK_MX_STATUS(mx_status,"Camera::prepareAcq()")
		}
		else if(m_acq_mode_name == "GEOMETRICAL")
		{
			DEB_TRACE() << "Acquisition mode : GEOMETRICAL";
			double frame_time = m_exposure_time + m_latency_time - m_readout_delay_time;
			m_status = "Prepare Acquisition mode : GEOMETRICAL";
			mx_status = mx_area_detector_set_geometrical_mode(m_mx_record,
															m_nb_frames,
															m_exposure_time,
															frame_time,
															m_exposure_multiplier,
															m_gap_multiplier);
			CHECK_MX_STATUS(mx_status,"Camera::prepareAcq()")
		}
		else if(m_acq_mode_name == "MEASURE_DARK")
		{
			DEB_TRACE() << "Acquisition mode : MEASURE_DARK";
			m_status = "Prepare Acquisition mode : MEASURE_DARK";
			mx_status = mx_area_detector_measure_dark_current_frame(m_mx_record,
																	m_exposure_time,
																	m_nb_frames);
			CHECK_MX_STATUS(mx_status,"Camera::prepareAcq()")
		}
		else if(m_acq_mode_name == "MEASURE_FLOOD_FIELD")
		{
			DEB_TRACE() << "Acquisition mode : MEASURE_FLOOD_FIELD";
			m_status = "Prepare Acquisition mode : MEASURE_FLOOD_FIELD";
			mx_status = mx_area_detector_measure_flood_field_frame(	m_mx_record,
																m_exposure_time,
																m_nb_frames);
			CHECK_MX_STATUS(mx_status,"Camera::prepareAcq()")
		}

		// set the trigger mode here, because Mx requires that it be done once the user has been defined acquisition mode
		mx_status = mx_area_detector_set_trigger_mode(m_mx_record,m_trigger_mode);
		CHECK_MX_STATUS(mx_status,"Camera::prepareAcq()");

	}
	catch(std::exception& e)
	{
		DEB_TRACE() << "Allocating memory is FAIL.";
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
	m_acq_frame_nb = -1;
	m_status = "Start Acquisition ...";

	if(m_acq_mode_name == "MEASURE_DARK")
		m_thread.sendCmd(CameraThread::StartMeasureDark);
	else if(m_acq_mode_name == "MEASURE_FLOOD_FIELD")
		m_thread.sendCmd(CameraThread::StartMeasureFloodField);
	else
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

	m_status = "Stop Acquisition ...";
	m_thread.sendCmd(CameraThread::StopAcq);
	m_thread.waitStatus(CameraThread::Ready);
}

//---------------------------------------------------------------------------------------
//! Camera::isBusy()
//---------------------------------------------------------------------------------------
bool Camera::isBusy()
{
	CHECK_MX_RECORD(m_mx_record,"Camera::isBusy()");
	mx_status_type mx_status;
	unsigned long status_flag;
	mx_status = mx_area_detector_get_status(m_mx_record,&status_flag);
	CHECK_MX_STATUS(mx_status,"Camera::isBusy()");
	if(status_flag/* & MXSF_AD_IS_BUSY*/)
	{
		return true;
	}
	return false;
}

//---------------------------------------------------------------------------------------
//! Camera::getExpTime()
//---------------------------------------------------------------------------------------
void Camera::getExpTime(double& exp_time)
{
	DEB_MEMBER_FUNCT();
	//DEB_TRACE() << "Camera::getExpTime";
	exp_time = m_exposure_time;
}

//---------------------------------------------------------------------------------------
//! Camera::setExpTime()
//---------------------------------------------------------------------------------------
void Camera::setExpTime(double exp_time)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "Camera::setExpTime - " << DEB_VAR1(exp_time) << " (s)";

	m_exposure_time = exp_time ;//default detector unit is sec	
}

//---------------------------------------------------------------------------------------
//! Camera::getExpMultiplier()
//---------------------------------------------------------------------------------------
void Camera::getExpMultiplier(double& exp_mult)
{
	DEB_MEMBER_FUNCT();
	//DEB_TRACE() << "Camera::getExpMultiplier";
	exp_mult = m_exposure_multiplier;
}

//---------------------------------------------------------------------------------------
//! Camera::setExpMultiplier()
//---------------------------------------------------------------------------------------	
void Camera::setExpMultiplier(double exp_mult)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "Camera::setExpMultiplier - " << DEB_VAR1(exp_mult);

	m_exposure_multiplier = exp_mult;
}

//---------------------------------------------------------------------------------------
//! Camera::getLatencyTime()
//---------------------------------------------------------------------------------------		
void Camera::getLatencyTime(double& latency_time)
{
	DEB_MEMBER_FUNCT();
	//DEB_TRACE() << "Camera::getLatencyTime";
	latency_time = m_latency_time;
}

//---------------------------------------------------------------------------------------
//! Camera::setLatencyTime()
//---------------------------------------------------------------------------------------		
void Camera::setLatencyTime(double latency_time)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "Camera::setLatencyTime - " << DEB_VAR1(latency_time) << " (s)";

	m_latency_time = latency_time ;//default detector unit is in sec	
}

//---------------------------------------------------------------------------------------
//! Camera::getGapMultiplier()
//---------------------------------------------------------------------------------------		
void Camera::getGapMultiplier(double& gap_mult)
{
	DEB_MEMBER_FUNCT();
	//DEB_TRACE() << "Camera::getGapMultiplier";
	gap_mult = m_gap_multiplier;
}

//---------------------------------------------------------------------------------------
//! Camera::setGapMultiplier()
//---------------------------------------------------------------------------------------		
void Camera::setGapMultiplier(double gap_mult)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "Camera::setGapMultiplier - " << DEB_VAR1(gap_mult);

	m_gap_multiplier = gap_mult;
}

//---------------------------------------------------------------------------------------
//! Camera::getReadoutDelayTime()
//---------------------------------------------------------------------------------------		
void Camera::getReadoutDelayTime(double& readout_delay)
{
	DEB_MEMBER_FUNCT();
	//DEB_TRACE() << "Camera::getReadoutDelayTime";
	readout_delay = m_readout_delay_time / 1E5; //unit is in 10 탎
}

//---------------------------------------------------------------------------------------
//! Camera::setReadoutDelayTime()
//---------------------------------------------------------------------------------------		
void Camera::setReadoutDelayTime(double readout_delay)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "Camera::setReadoutDelayTime - " << DEB_VAR1(readout_delay) << " (s)";

	m_readout_delay_time = readout_delay * 1E5;//unit is in 10 탎
}


//---------------------------------------------------------------------------------------
//! Camera::getInitialDelayTime()
//---------------------------------------------------------------------------------------		
void Camera::getInitialDelayTime(double& initial_delay)
{
	DEB_MEMBER_FUNCT();
	//DEB_TRACE() << "Camera::getInitialDelayTime";
	initial_delay = m_initial_delay_time / 1E5; //unit is in 10 탎
}

//---------------------------------------------------------------------------------------
//! Camera::setInitialDelayTime()
//---------------------------------------------------------------------------------------		
void Camera::setInitialDelayTime(double initial_delay)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "Camera::setInitialDelayTime - " << DEB_VAR1(initial_delay) << " (s)";
	m_initial_delay_time = initial_delay * 1E5; //unit is in 10 탎
}

//---------------------------------------------------------------------------------------
//! Camera::getReadoutSpeed()
//---------------------------------------------------------------------------------------		
void Camera::getReadoutSpeed(bool& readout_speed)
{
	DEB_MEMBER_FUNCT();
	//DEB_TRACE() << "Camera::getReadoutSpeed";
	readout_speed = m_readout_speed;
}

//---------------------------------------------------------------------------------------
//! Camera::setReadoutSpeed()
//---------------------------------------------------------------------------------------		
void Camera::setReadoutSpeed(bool readout_speed)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "Camera::setReadoutSpeed - " << DEB_VAR1(readout_speed);

	m_readout_speed = readout_speed;
}

//---------------------------------------------------------------------------------------
//! Camera::getOffsetCorrection()
//---------------------------------------------------------------------------------------		
void Camera::getOffsetCorrection(bool& offset_correction)
{
	DEB_MEMBER_FUNCT();
	//DEB_TRACE() << "Camera::getOffsetCorrection";
	offset_correction = m_offset_correction;
}

//---------------------------------------------------------------------------------------
//! Camera::setOffsetCorrection()
//---------------------------------------------------------------------------------------		
void Camera::setOffsetCorrection(bool offset_correction)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "Camera::setOffsetCorrection - " << DEB_VAR1(offset_correction);

	m_offset_correction = offset_correction;
}

//---------------------------------------------------------------------------------------
//! Camera::getLinearization()
//---------------------------------------------------------------------------------------		
void Camera::getLinearization(bool& linearization)
{
	DEB_MEMBER_FUNCT();
	//DEB_TRACE() << "Camera::getLinearization";
	linearization = m_linearization;
}

//---------------------------------------------------------------------------------------
//! Camera::setLinearization()
//---------------------------------------------------------------------------------------		
void Camera::setLinearization(bool linearization)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "Camera::setLinearization - " << DEB_VAR1(linearization);

	m_linearization = linearization;
}

//---------------------------------------------------------------------------------------
//! Camera::setCorrectionFlags()
//---------------------------------------------------------------------------------------		
void Camera::setCorrectionFlags(unsigned long flags)
{
	DEB_MEMBER_FUNCT();	
	DEB_TRACE() << "Camera::setCorrectionFlags - 0x"<<std::uppercase<<std::hex<<flags;	
	m_correction_flags = flags;
}

//---------------------------------------------------------------------------------------
//! Camera::getMxLibraryVersion()
//---------------------------------------------------------------------------------------
void Camera::getMxLibraryVersion(std::string& version)
{
	DEB_MEMBER_FUNCT();
	version = mx_get_version_string();
}

//---------------------------------------------------------------------------------------
//! Camera::getInternalAcqMode()
//---------------------------------------------------------------------------------------
void Camera::getInternalAcqMode(std::string& acq_mode)
{
	DEB_MEMBER_FUNCT();
	acq_mode = m_acq_mode_name;
}

//---------------------------------------------------------------------------------------
//! Camera::setInternalAcqMode()
//---------------------------------------------------------------------------------------
void Camera::setInternalAcqMode(const std::string& mode)
{
	DEB_MEMBER_FUNCT();
	DEB_PARAM() << DEB_VAR1(mode);

	if(	mode == "ONESHOT"				||
		mode == "CONTINUOUS"			||
		mode == "MULTIFRAME"			||
		mode == "GEOMETRICAL"			||
		mode == "MEASURE_DARK"			||
		mode == "MEASURE_FLOOD_FIELD")
	{
		m_acq_mode_name = mode;
	}
	else
	{
		THROW_HW_ERROR(Error) << "Incorrect Internal Acquisition mode !";
	}
}

//---------------------------------------------------------------------------------------
//! Camera::getDetectorModel()
//---------------------------------------------------------------------------------------
void Camera::getDetectorModel(std::string& model)
{
	DEB_MEMBER_FUNCT();
	DEB_RETURN() << "PCCD-170170";
	model = "PCCD-170170";
}

//---------------------------------------------------------------------------------------
//! Camera::getMaxWidth()
//---------------------------------------------------------------------------------------
unsigned short Camera::getMaxWidth()
{
	DEB_MEMBER_FUNCT();
	DEB_RETURN() << DEB_VAR1(m_max_width);
	return m_max_width;
}

//---------------------------------------------------------------------------------------
//! Camera::getMaxHeight()
//---------------------------------------------------------------------------------------
unsigned short Camera::getMaxHeight()
{
	DEB_MEMBER_FUNCT();
	DEB_RETURN() << DEB_VAR1(m_max_height);
	return m_max_height;
}

//-----------------------------------------------------
//! Camera::setImageType()
//-----------------------------------------------------
void Camera::setImageType(ImageType type)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "Camera::setImageType - " << DEB_VAR1(type);
	switch(type)
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
		case 16: type = Bpp16;
			break;
		default:
			THROW_HW_ERROR(Error) << "This pixel format of the camera is not managed, only 16 bits cameras are already managed!";
			break;
	}
	return;
}

//---------------------------------------------------------------------------------------
//! Camera::setBin()
//---------------------------------------------------------------------------------------
void Camera::setBin(const Bin& bin)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "Camera::setBin";
	DEB_PARAM() << DEB_VAR1(bin);
	if(m_name == "NONE") return;//simu mode : used to start LimaDetector even when detector is not available

	// Define the binning.
	CHECK_MX_RECORD(m_mx_record,"Camera::setBin()");
	mx_status_type mx_status;
	mx_status = mx_area_detector_set_binsize(m_mx_record,bin.getX(),bin.getY());
	CHECK_MX_STATUS(mx_status,"Camera::setBin()")

}

//---------------------------------------------------------------------------------------
//! Camera::getBin()
//---------------------------------------------------------------------------------------
void Camera::getBin(Bin& bin)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "Camera::getBin";

	// Get the current Bin 
	long xbin = 1, ybin = 1;
	CHECK_MX_RECORD(m_mx_record,"Camera::getBin()");
	mx_status_type mx_status;
	mx_status = mx_area_detector_get_binsize(m_mx_record,&xbin,&ybin);
	CHECK_MX_STATUS(mx_status,"Camera::getBin()")
	bin = Bin(xbin,ybin);
	DEB_RETURN() << DEB_VAR1(bin);
}

//---------------------------------------------------------------------------------------
//! Camera::checkBin()
//---------------------------------------------------------------------------------------
void Camera::checkBin(Bin& bin)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "Camera::checkBin";
	DEB_PARAM() << DEB_VAR1(bin);

	//All standards Binning are supported !! @TODO TO CONFIRM 
	if(bin.getX() <= 0 || bin.getY() <= 0)
	{
		THROW_HW_ERROR(Error) << "checkBin : Invalid Bin = " << DEB_VAR1(bin);
	}
}

//-----------------------------------------------------
//! Camera::checkRoi()
//-----------------------------------------------------
void Camera::checkRoi(const Roi& set_roi,Roi& hw_roi)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "Camera::checkRoi";
	DEB_PARAM() << DEB_VAR1(set_roi);
	//@@ #TODO implement this    
	hw_roi = set_roi;

	DEB_RETURN() << DEB_VAR1(hw_roi);
}

//---------------------------------------------------------------------------------------
//! Camera::getRoi()
//---------------------------------------------------------------------------------------
void Camera::getRoi(Roi& hw_roi)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "Camera::getRoi";
	//@@ #TODO implement this
	hw_roi = Roi(0,0,m_max_width,m_max_height);

	DEB_RETURN() << DEB_VAR1(hw_roi);
}

//---------------------------------------------------------------------------------------
//! Camera::setRoi()
//---------------------------------------------------------------------------------------
void Camera::setRoi(const Roi& set_roi)
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "Camera::setRoi";
	DEB_PARAM() << DEB_VAR1(set_roi);
	//@@@@ #TODO implement this
	if(!set_roi.isActive())
	{
		DEB_TRACE() << "Roi is not Enabled";
	}
	else
	{
		DEB_TRACE() << "Roi is Enabled";
	}
}

//---------------------------------------------------------------------------------------
//! Camera::_open()
//---------------------------------------------------------------------------------------
void Camera::_open()
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "Camera::_open";
	if(m_name == "NONE") return;//simu mode : used to start LimaDetector even when detector is not available

	//@@@@ #TODO close if previously open
	//    close();

	// open mx database  
	mx_status_type mx_status;
	AutoMutex mx_lock(m_cond.mutex());
	DEB_TRACE() << "Open Mx DataBase";
	MX_RECORD* database;
	mx_status = mx_setup_database(&database,(char*)m_database_file_name.c_str());
	CHECK_MX_STATUS(mx_status,"Camera::_open()")

	// open mx record
	DEB_TRACE() << "Open Mx Record";
	m_mx_record = mx_get_record(database,(char*)m_name.c_str());
	CHECK_MX_RECORD(m_mx_record,"Camera::_open() - The MX server is either not running or is not working correctly. ")

	if(m_mx_record->mx_type != MXT_AD_NETWORK)
	{
		std::ostringstream err;
		err << "Mx Record '" << m_name << "' is not a network area detector record." << std::endl;
		throw LIMA_HW_EXC(Error,"Camera::_open()") << " - " << err.str().c_str();
	}

	// get ccd size
	DEB_TRACE() << "Get CCD Size";
	long width = 0;
	long height = 0;
	mx_status = mx_area_detector_get_maximum_framesize(m_mx_record,&width,&height);
	CHECK_MX_STATUS(mx_status,"Camera::_open()")
	m_max_width = width;
	m_max_height = height;
	DEB_TRACE() << "m_max_width = " << m_max_width;
	DEB_TRACE() << "m_max_height = " << m_max_height;

	// get ccd depth
	DEB_TRACE() << "Get CCD Depth";
	m_depth = 0;
	long bits_per_pixel = 16;
	mx_status = mx_area_detector_get_bits_per_pixel(m_mx_record,&bits_per_pixel);
	CHECK_MX_STATUS(mx_status,"Camera::_open()")
	m_depth = bits_per_pixel;
	DEB_TRACE() << "m_depth = " << m_depth;

	// update device state
	////mState = Tango::OPEN;
	m_status = "The camera is Open and Ready.";
}

//---------------------------------------------------------------------------------------
//! Camera::_close()
//---------------------------------------------------------------------------------------
void Camera::_close()
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "Camera::_close";
	if(m_name == "NONE") return;//simu mode : used to start LimaDetector even when detector is not available

	//@@@@ #TODO stop possibly running acquisition
	// stop();

	AutoMutex mx_lock(m_cond.mutex());
	if(m_mx_record)
	{
		// #TODO : find a way to close the connection with mx server
		mx_close_hardware(m_mx_record);
		mx_delete_record(m_mx_record);
	}
	m_mx_record = 0;

	////mState = Tango::CLOSE;
	m_status = "The camera was Closed.";
}

//---------------------------------------------------------------------------------------
//! Camera::_armDetector()
//---------------------------------------------------------------------------------------
void Camera::_armDetector()
{
	DEB_MEMBER_FUNCT();
	DEB_TRACE() << "Camera::_armDetector";
	// Prepare the detector aviex in order to make an acqusition : arm/trigger
	CHECK_MX_RECORD(m_mx_record,"CameraThread::_armDetector()");

	mx_status_type mx_status;
	DEB_TRACE() << "\t- arm Detector.";
	mx_status = mx_area_detector_arm(m_mx_record);
	CHECK_MX_STATUS(mx_status,"Camera::_armDetector()");

	//in internal trigger, we need to call this function to really start the acquisition!!
	DEB_TRACE() << "\t- is internal trigger ?";
	if(m_trigger_mode == 1)
	{
		DEB_TRACE() << "\t- internal detector trigger.";
		mx_status = mx_area_detector_trigger(m_mx_record);
		CHECK_MX_STATUS(mx_status,"Camera::_armDetector()");
	}
}

//========================================================================================
