#ifndef AVIEXCAMERA_H
#define AVIEXCAMERA_H

#include "Debug.h"
#include "AviexCompatibility.h"
#include "Constants.h"
#include "HwBufferMgr.h"
#include "ThreadUtils.h"



extern "C"
{
#include "mx_util.h"
#include "mx_record.h"
#include "mx_image.h"
#include "mx_area_detector.h"
#include "mx_driver.h"
#include "mx_version.h"
}

namespace lima
{
namespace Aviex
{
//    class Camera
class LIBAVIEX_API Camera
{
	DEB_CLASS_NAMESPC(DebModCamera,"Camera","Aviex");
	friend class Interface;
public:
	enum Status
	{
		Ready, Exposure, Readout, Latency, Fault
	} ;

	Camera(const std::string& camName, const std::string& databaseFile);
	~Camera();

	//-- Related to Aviex specific features
	void getExpMultiplier(double& exp_mult);
	void setExpMultiplier(double exp_mult);
	void getLatencyTime(double& period_time);
	void setLatencyTime(double  period_time);
	void getGapMultiplier(double& gap_mult);
	void setGapMultiplier(double  gap_mult);
	void getMxLibraryVersion(std::string& version);
	void getInternalAcqMode(std::string& acq_mode);
	void setInternalAcqMode(const std::string& mode);
	void getReadoutDelayTime(double& readout_delay);
	void setReadoutDelayTime(double readout_delay);
	void getReadoutSpeed(bool& readout_speed);
	void setReadoutSpeed(bool readout_speed);
	void getOffsetCorrection(bool& offset_correction);
	void setOffsetCorrection(bool offset_correction);		
	void getLinearization(bool& linearization);
	void setLinearization(bool linearization);	
	
	void getInitialDelayTime(double& initial_delay);
	void setInitialDelayTime(double initial_delay);
	void setCorrectionFlags(unsigned long);

	//-- Related to Acquisition 
	void startAcq();
	void stopAcq();
	void prepareAcq();
	Status getStatus();
	bool isBusy();				// return the state of the detector : true if detector is processing/running

	//-- Related to Synch control object
	void setTrigMode(TrigMode  mode);
	void getTrigMode(TrigMode& mode);
	void getExpTime(double& exp_time);
	void setExpTime(double  exp_time);
	int  getNbHwAcquiredFrames();
	void setNbFrames(int nb_frames);
	void getNbFrames(int& nb_frames);

	//-- Related to Bin control object
	void setBin(const Bin& bin);
	void getBin(Bin& bin);
	void checkBin(Bin& bin);

	//-- Related to Roi control object    
	void checkRoi(const Roi& set_roi, Roi& hw_roi);
	void setRoi(const Roi& set_roi);
	void getRoi(Roi& hw_roi);

	//-- Related to BufferCtrl	object
	HwBufferCtrlObj* getBufferCtrlObj()
	{
		return &m_buffer_ctrl_obj;
	};

	//-- Related to DetInfo object
	void getDetectorModel(std::string& model);
	unsigned short getMaxWidth();
	unsigned short getMaxHeight();
	void setImageType(ImageType type);
	void getImageType(ImageType& type);

private:
	void _open();							// open (reserves) the camera
	void _close();							// close (release) the camera
	void _armDetector();					// do arm/trigger on detector

	//CmdThread class, used to handle some specific tasks (startAcq, stopAcq, ...)
	class CameraThread: public CmdThread
	{
		DEB_CLASS_NAMESPC(DebModCamera, "CameraThread", "Aviex");
	public:
		enum
		{ // Status
			Ready = MaxThreadStatus, Exposure, Readout, Latency,
		} ;
		enum
		{ // Cmd
			StartAcq = MaxThreadCmd, StopAcq,StartMeasureDark, StartMeasureFloodField,
		} ;

		CameraThread(Camera& cam);

		virtual void start();

		int getNbHwAcquiredFrames();
		bool m_force_stop;

	protected:
		virtual void init();
		virtual void execCmd(int cmd);
	private:
		void execStartAcq();
		void execStartMeasureCorrection(unsigned measure_type);
		Camera*		m_cam;
	} ;
	friend class CameraThread;

	/* Lima buffer control object */
	SoftBufferCtrlObj	m_buffer_ctrl_obj;
	unsigned short*		m_frame;
	unsigned short*		m_pr_buffer;

	/*camera stuff*/
	Camera::Status  m_state;
	std::string     m_name;
	std::string     m_database_file_name;
	double          m_exposure_time;
	double          m_exposure_multiplier;
	double          m_latency_time;
	double          m_gap_multiplier;
	double			m_initial_delay_time;
	double			m_readout_delay_time;
	bool			m_readout_speed;
	bool			m_offset_correction;
	bool			m_linearization;	
	unsigned long   m_correction_flags;
	int				m_nb_frames;
	int             m_trigger_mode;
	long			m_max_width;
	long			m_max_height;
	long			m_depth;
	std::string     m_acq_mode_name;
	std::string     m_status;
	Size			m_frame_size;

	/* main acquisition thread*/
	CameraThread 	m_thread;
	int 			m_acq_frame_nb;

	/*Mx stuff*/
	struct mx_record_type* m_mx_record;

	/*mutex stuff*/
	mutable Cond           m_cond;

} ;
}
}
#endif
