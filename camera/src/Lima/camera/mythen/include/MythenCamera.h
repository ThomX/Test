#ifndef MYTHENCAMERA_H
#define MYTHENCAMERA_H
#include "Mythen.h"
#include "Debug.h"
#include "Constants.h"
#include "HwMaxImageSizeCallback.h"
#include "HwBufferMgr.h"
#include "ThreadUtils.h"

namespace lima
{
  namespace Mythen
  {
    class Camera : public HwMaxImageSizeCallbackGen   
    //    class Camera 
    {
      friend class Interface;
      DEB_CLASS_NAMESPC(DebModCamera,"Camera","Mythen");


    public:
      
      enum Status {
	Ready, Exposure, Readout, Latency,
      };

      Camera(std::string HostName, int PortNb = 1952, int DetectorId = 0);
      ~Camera();
     
      double exposure() const;
      void setExposure(double expo);

      bool isMonochrome() const;
	
      Status getStatus();
      int    getNbAcquiredFrames() const {return m_acq_frame_nb;}

      VideoMode getVideoMode() const;
      void 	setVideoMode(VideoMode);
 
	
      void 	startAcq();	
      void 	stopAcq();
      void      prepareAcq();
      void	reset();

      void setNbFrames(int  nb_frames);
      void getNbFrames(int& nb_frames);

      int       cam_sim_mode;

      HwBufferCtrlObj* getBufferCtrlObj();


      int       getTrigMode();
      void      setTrigMode(int mode);

      std::string  getInternalAcqMode();
      void         setInternalAcqMode(std::string mode);

      int       getMaxWidth();
      int       getMaxHeight();

      void setBin(const Bin& bin);
      void getBin(Bin& bin);
      void checkBin(Bin& bin);


      void new_mythen_detector();
      void mythen_set(std::string);
      std::string mythen_get(mythenDetector myDetector, char *parameter);
      void refresh_fname();
      bool isGoodString(std::string);

      
      int probeHost(); // Probe that hostname:port accepts connections
      
      bool getBusyFlag(); // 'true' if Mythen is busy
      void setBusyFlag(bool); // set Mythen busy flag
      
      void SetRefreshFlag(); // Set RefreshFlag=true, which will cause refresh() to be called
            
      std::string MythenExecuteLine(int);
      std::string MythenExecuteLine(std::string, int);
      std::string MythenExecuteLine(std::string, std::string, int);
 
      void        setPositions(std::string positions);
      std::string getPositions();

      void        setTrimEnergies(std::string trimenergies);
      std::string getTrimEnergies();

      void        setThreshold(short threshold);
      short       getThreshold();

      void        Refresh();

      int getIntParameter(std::string parameter);
      void setIntParameter(std::string parameter, int value);
      std::string getIntParameterList(void);

      float getFloatParameter(std::string parameter);
      void setFloatParameter(std::string parameter, float value);
      std::string getFloatParameterList(void);

      std::string getStringParameter(std::string parameter);
      void setStringParameter(std::string parameter, std::string value);
      std::string getStringParameterList(void);

    private:
      class CameraThread : public CmdThread{
      public:
	enum { // Status
	  Ready = MaxThreadStatus, Exposure, Readout, Latency,
	};
	
	enum { // Cmd 
	  StartAcq = MaxThreadCmd, StopAcq, StartStartAcq, Refresh
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
	void execStartStartAcq();
	void execRefresh();

	Camera* m_cam;
	
      };
      friend class CameraThread;

      void 		_allocBuffer();

      SoftBufferCtrlObj m_buffer_ctrl_obj;


      bool 		m_cam_connected;
      char		m_sensor_type[64];
      int		m_maxwidth, m_maxheight;
      
      //     unsigned long         *m_frame;
     
      unsigned long m_data[MAXSTR]; // image data (max 1280 strips * 24 modules)
      

      VideoMode		m_video_mode;
      int		m_acq_frame_nb;
      bool		m_continue_acq;
      double 		m_exposure; 

      char              m_error_msg[200];
      int               m_camera_busy;
      int               m_error_code;
      int               m_nb_frames;

      mythenDetector *m_detector;

      bool ACBusyFlag; // Acquire Busy Flag
      bool BTBusyFlag; // Bus Test Busy Flag
      bool DTBusyFlag; // Digital Test Busy Flag
      bool MStopFlag; // Merge Stop Flag (if true, will force MergeThread to stop)
      bool SBusyFlag; // Settings Busy Flag
      bool RBusyFlag; // Refresh Busy Flag

      bool m_refresh_flag; // If true, will force refresh() to be run 
      

      bool check_input(std::string, const std::string*, int);

      
      unsigned short  m_size;

      std::string m_host_name;
      int m_port_nb;
      int m_detector_id;

      int m_trigger_mode;
      int m_int_acq_mode;
 
      int m_max_width;
      int m_max_height;

      CameraThread m_thread;
      CameraThread m_start_thread;
      CameraThread m_refresh_thread;
      
      int m_file_in_progress;

      std::string m_positions;
      std::string m_trimenergies;
      short m_threshold;
      bool m_flat_field_enabled;
      std::string m_flat_field_filename;
      
 
      float m_rate_corr_deadtime;

      std::string m_bad_channels_filename;

      std::string m_ang_conv_filename;
      
      float m_global_offset;
      float m_fine_offset;
      float m_bin_size;
      
      std::string m_calibration_dir;
      
      int m_threaded;
      int m_n_modules;
      int m_n_max_modules;

      std::string m_module_serial_number;
      int m_module_firmware_version;
      int m_detector_serial_number;
      int m_detector_firmware_version;
      int m_detector_software_version;
      int m_mythen_software_version;
      
      std::string m_settings;
      float m_frame_period;
      float m_delay_after_trigger;
      int   m_gates_per_frame;
      int   m_frames_per_cycle;
      int   m_n_cycles;
      int   m_n_probes;
      int   m_dynamic_range;
      int   m_online;

      std::string m_out_dir;
      std::string m_out_filename;
      int         m_out_fileindex;
     

      std::string m_flags;
      std::string m_trim_dir;

    };
  }
}
#endif
