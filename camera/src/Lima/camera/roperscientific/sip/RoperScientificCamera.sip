namespace RoperScientific
{
  class Camera
  {
%TypeHeaderCode
#include <RoperScientificCamera.h>
%End
  public:
    Camera(int);
    ~Camera();
      
    bool isMonochrome() const;

    int getNbAcquiredFrames() const;

    VideoMode getVideoMode() const;
    void 	setVideoMode(VideoMode);

    void setNbFrames(int );
    void getNbFrames(int&);

    void	getCameraName(std::string& name /Out/);
	      
    void 	startAcq();	
    void 	stopAcq();
    void        prepareAcq();
    void	reset();

    void      setGain(long);
    long      getGain();

    void      get_camera_error();

    float     getTemperature(); 
    float     getTemperatureSetPoint();
    void      setTemperatureSetPoint( float);

    std::string  getInternalAcqMode();
    void         setInternalAcqMode(std::string );
	
    int        getUseFullFrame();
    void       setUseFullFrame(int );
	
  };
};
