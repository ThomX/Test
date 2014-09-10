namespace Aviex
{
  //    class Camera
  class Camera
  {
%TypeHeaderCode
#include <AviexCamera.h>
%End
  public:
    enum Status
    {
      Ready, Exposure, Readout, Latency, Fault
    } ;

    Camera(const std::string& camName, const std::string& databaseFile);
    ~Camera();

    //-- Related to Aviex specific features
    void getExpMultiplier(double& exp_mult /Out/);
    void setExpMultiplier(double exp_mult);
    void getLatencyTime(double& period_time /Out/);
    void setLatencyTime(double  period_time);
    void getGapMultiplier(double& gap_mult /Out/);
    void setGapMultiplier(double  gap_mult);
    void getMxLibraryVersion(std::string& version /Out/);
    void getInternalAcqMode(std::string& acq_mode /Out/);
    void setInternalAcqMode(const std::string& mode);
    void getReadoutDelayTime(double& readout_delay /Out/);
    void setReadoutDelayTime(double readout_delay);
    void getReadoutSpeed(bool& readout_speed /Out/);
    void setReadoutSpeed(bool readout_speed);
    void getOffsetCorrection(bool& offset_correction /Out/);
    void setOffsetCorrection(bool offset_correction);		
    void getLinearization(bool& linearization /Out/);
    void setLinearization(bool linearization);	
	
    void getInitialDelayTime(double& initial_delay /Out/);
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
    void getTrigMode(TrigMode& mode /Out/);
    void getExpTime(double& exp_time /Out/);
    void setExpTime(double  exp_time);
    int  getNbHwAcquiredFrames();
    void setNbFrames(int nb_frames);
    void getNbFrames(int& nb_frames /Out/);

    //-- Related to Bin control object
    void setBin(const Bin& bin);
    void getBin(Bin& bin /Out/);
    void checkBin(Bin& bin /In,Out/);

    //-- Related to Roi control object    
    void checkRoi(const Roi& set_roi, Roi& hw_roi /Out/);
    void setRoi(const Roi& set_roi);
    void getRoi(Roi& hw_roi);

    //-- Related to DetInfo object
    void getDetectorModel(std::string& model /Out/);
    unsigned short getMaxWidth();
    unsigned short getMaxHeight();
    void setImageType(ImageType type);
    void getImageType(ImageType& type /Out/);
  } ;
};
