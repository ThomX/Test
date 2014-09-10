#ifndef ROPERSCIENTIFICCAMERA_H
#define ROPERSCIENTIFICCAMERA_H

#include "Debug.h"
#include "RoperScientific.h"
#include "RoperScientificCompatibility.h"
#include "Constants.h"
#include "HwBufferMgr.h"
#include "ThreadUtils.h"

namespace lima
{
namespace RoperScientific
{
//    class Camera
class LIBROPERSCIENTIFIC_API Camera
{
	DEB_CLASS_NAMESPC(DebModCamera,"Camera","RoperScientific");
	friend class Interface;
public:
	
	enum Status
	{
		Ready, Exposure, Readout, Latency, Fault
	};

	Camera(int camNum = 0);
	~Camera();

    void getExpTime(double& exp_time);
    void setExpTime(double  exp_time);

	Status getStatus();
	int  getNbAcquiredFrames();
	
	void getDetectorModel(std::string& model);
	void startAcq();
	void stopAcq();
	void prepareAcq();
	void reset();

	void setNbFrames(int nb_frames);
	void getNbFrames(int& nb_frames);

	void setGain(long);
	long getGain();

	void setFullFrame(rgn_type* roi);
    void setBinRoiParameters(rgn_type* roi);

	HwBufferCtrlObj* getBufferCtrlObj() { return &m_buffer_ctrl_obj; };

    void setSpeedTableIndex(unsigned);
    unsigned getSpeedTableIndex(void);
	const std::string& getADCRate(void);

	double 	getTemperature();
	double 	getTemperatureSetPoint();
	void	setTemperatureSetPoint(double temperature);

    //-- Synch control object
    void setTrigMode(TrigMode  mode);
    void getTrigMode(TrigMode& mode);

	//-- Shutter managment : TODO Shutter control object
    //---------------------------------------------------------------------------------------
	void setShutterMode(int mode);
    void getShutterMode(int& mode);
	
	std::string getInternalAcqMode();
	void setInternalAcqMode(std::string mode);

	uns16 getMaxWidth();
	uns16 getMaxHeight();
	
	void setImageType(ImageType type);
	void getImageType(ImageType& type);
	
	int getUseFullFrame();
	void setUseFullFrame(int val);

	void setBin(const Bin& bin);
	void getBin(Bin& bin);
	void checkBin(Bin& bin);

    void checkRoi(const Roi& set_roi, Roi& hw_roi);
    void setRoi(const Roi& set_roi);
    void getRoi(Roi& hw_roi);   

private:
	class CameraThread: public CmdThread
	{
	    DEB_CLASS_NAMESPC(DebModCamera, "CameraThread", "RoperScientific");
	public:
		enum
		{ // Status
			Ready = MaxThreadStatus, Exposure, Readout, Latency,
		};

		enum
		{ // Cmd
			StartAcq = MaxThreadCmd, StopAcq,
		};

		CameraThread(Camera& cam);

		virtual void start();

		int getNbAcquiredFrames();
		bool m_force_stop;

	protected:
		virtual void init();
		virtual void execCmd(int cmd);
	private:
		void execStartAcq();
		Camera* m_cam;

	};
	friend class CameraThread;

	/* Lima buffer control object */
	SoftBufferCtrlObj m_buffer_ctrl_obj;
	
	/* Related to API PvCam */
	int short m_handle; 
	char m_name[128];
	double m_exposure;

	char m_error_msg[200];
	int m_error_code;
	int m_nb_frames;

	unsigned short m_roi_s1;
	unsigned short m_roi_s2;
	unsigned short m_roi_sbin;
	unsigned short m_roi_p1;
	unsigned short m_roi_p2;
	unsigned short m_roi_pbin;
	
	uns32 m_size;

	int m_trigger_mode;
	int m_shutter_mode;
	int m_int_acq_mode;	

	uns16 m_max_width;
	uns16 m_max_height;
	uns16 m_depth;

	std::string m_adc_rate;
	
	unsigned short *m_frame;
	unsigned short *m_pr_buffer;
	
	/* main acquisition thread*/
	CameraThread 	m_thread;
	int 			m_acq_frame_nb;
	
};
}
}
#endif
