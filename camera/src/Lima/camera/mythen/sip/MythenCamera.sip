namespace Mythen
{
  class Camera
  {
%TypeHeaderCode
#include <MythenCamera.h>
%End
  public:
    Camera(std::string, int, int);
    ~Camera();
      
    bool isMonochrome() const;

    int getNbAcquiredFrames() const;

    VideoMode getVideoMode() const;
    void 	setVideoMode(VideoMode);

    void setNbFrames(int );
    void getNbFrames(int&);

	      
    void 	startAcq();	
    void 	stopAcq();
    void        prepareAcq();
    void	reset();

    std::string  getInternalAcqMode();
    void         setInternalAcqMode(std::string );

    std::string getPositions();	
    void        setPositions(std::string); 
 
    std::string getTrimEnergies();	
    void        setTrimEnergies(std::string);
 
    short       getThreshold();	
    void        setThreshold(short);

    void        Refresh();

    int getIntParameter(std::string);
    void setIntParameter(std::string, int);
    std::string getIntParameterList();

    float getFloatParameter(std::string);
    void setFloatParameter(std::string, float);
    std::string getFloatParameterList();

    std::string getStringParameter(std::string);
    void setStringParameter(std::string, std::string);
    std::string getStringParameterList();	


  };
};
