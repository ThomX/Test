#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/stat.h> 

#include "Exceptions.h"

#include "MythenCamera.h"

using namespace lima;
using namespace lima::Mythen;


Camera::CameraThread::CameraThread(Camera& cam)
	: m_cam(&cam)
{

  m_cam->m_acq_frame_nb = 0;
  m_force_stop = false;

}

void Camera::CameraThread::start()
{
  CmdThread::start();
  waitStatus(Ready);
  
}

void Camera::CameraThread::init()
{
  setStatus(Ready);
}

void Camera::CameraThread::execCmd(int cmd)
{
  int status = getStatus();
  switch (cmd) {
  case StartAcq:
    if (status != Ready && !m_cam->RBusyFlag)
      throw LIMA_HW_EXC(InvalidValue,  "Not Ready to StartAcq");
    execStartAcq();
    break;
  case StartStartAcq:
    if (status != Ready && !m_cam->ACBusyFlag  && !m_cam->RBusyFlag)
      throw LIMA_HW_EXC(InvalidValue,  "Not Ready to StartStartAcq");
    execStartStartAcq();
    break;
  case Refresh:
    if (status != Ready && !m_cam->ACBusyFlag && !m_cam->RBusyFlag)
      throw LIMA_HW_EXC(InvalidValue,  "Not Ready to StartStartAcq");
    execRefresh();
    break;
  }
}


void Camera::CameraThread::execStartAcq()
{
  int acq_frame_nb;

  struct stat stFileInfo;
  bool blnReturn;
  int intStat; 

  char file_to_open[20] = "\0";
  char cmd[100] = "\0";
	
  setStatus(Exposure);

  StdBufferCbMgr& buffer_mgr = m_cam->m_buffer_ctrl_obj.getBuffer();
  buffer_mgr.setStartTimestamp(Timestamp::now());
  
  int nb_frames = m_cam->m_nb_frames;


  int& frame_nb = m_cam->m_acq_frame_nb;

  m_cam->m_acq_frame_nb = 0;


  acq_frame_nb = 0;

  while( nb_frames ) {
    
    if(m_force_stop){
	m_force_stop = false;
	setStatus(Ready);
	goto stop;
    }

    setStatus(Readout);    

    sprintf(file_to_open, "%s/%s_%d.raw", m_cam->m_out_dir.c_str(),m_cam->m_out_filename.c_str(), m_cam->m_out_fileindex + acq_frame_nb);
    

    intStat = 1;
    while(intStat){
      intStat = stat(file_to_open,&stFileInfo);
    }

    printf("File %s found \n", file_to_open);

    if(m_cam->cam_sim_mode){ // que pasa en no simulation mode ???
      printf("File in progress %d acq_frame_nb + 1 %d \n", m_cam->m_file_in_progress, acq_frame_nb + 1);
      while ( !(m_cam->m_file_in_progress > acq_frame_nb + 1) ) { // wait until file is finished
	usleep(100);	
      }
    }

    ifstream myfile;
    myfile.open(file_to_open);
    
    unsigned int idx;
    int nmax;
    unsigned long tmpdat;
    char line[128];

    while (myfile.good()) {
      myfile.getline(line, 128); // Read a line (man 256 char's)
      sscanf(line, "%d %d", &idx, &tmpdat); // Get index and data from current line
      if (idx>=0 && idx<nmax) m_cam->m_data[idx]=tmpdat; // Copy data to the array
    }
    myfile.close();
 
    printf("File in progress %d \n", m_cam->m_file_in_progress);
    if( !m_cam->cam_sim_mode || (m_cam->m_file_in_progress > acq_frame_nb + 1) ){
      sprintf(cmd, "rm -rf %s", file_to_open);
      system(cmd);
      printf("File %s removed \n", file_to_open);
    }
    
    // This part is necessary for sending the data for further Lima processing (saving files ...)

    unsigned long *ptr = (unsigned long*)buffer_mgr.getFrameBufferPtr(frame_nb);
  
    memcpy(ptr,m_cam->m_data,m_cam->m_size/sizeof(unsigned long)+1);

    buffer_mgr.setStartTimestamp(Timestamp::now());
    

    HwFrameInfoType frame_info;
    // This set is necessary for telling Lima when the acquisition has finished
    frame_info.acq_frame_nb = acq_frame_nb;
    buffer_mgr.newFrameReady(frame_info); 

    m_cam->m_acq_frame_nb = acq_frame_nb;

    acq_frame_nb++; 
    nb_frames--;


  } /* End while */

 stop:
  
  //  if( m_cam->m_frame ) free( m_cam->m_frame );
  
  m_cam->setBusyFlag(false);
  m_cam->ACBusyFlag = false;

  setStatus(Ready);
}

void Camera::CameraThread::execStartStartAcq()
{


  FILE *data;   
  if( !m_cam->cam_sim_mode ){
    
    mythenDetector myDetector(m_cam->m_detector_id);
    char **myArgs;
    myDetector.executeLine(0, (char **)myArgs, READOUT_ACTION);
    
  }else{  
    int nb_frames = m_cam->m_nb_frames; 
    int save_nb_frames = m_cam->m_nb_frames;
    double req_time;
    req_time = m_cam->m_exposure;
    while(nb_frames > 0){
      char s[1000] = "\0";
      nb_frames--;
      m_cam->m_file_in_progress = save_nb_frames - nb_frames;
      sprintf(s,"%s_%d.raw",m_cam->m_out_filename.c_str(),m_cam->m_out_fileindex - 1 + save_nb_frames - nb_frames);
      if (req_time > 0) {
	usleep(long(req_time * 1e6));
      }
      for(int i = 0; i < 30720; i++){
	m_cam->m_data[i]= i;
      } 
      data = fopen(s,"w");
      for(int i = 0; i < 30720; i++){
	fprintf(data, "%d %d \n", i, m_cam->m_data[i]);
      }
      fclose( data );
      m_cam->m_file_in_progress = save_nb_frames - nb_frames + 1;
      
    }
  } 

}


void Camera::CameraThread::execRefresh()
{
  float FVar;
  string answer;
  

  m_cam->setBusyFlag(true);
  m_cam->RBusyFlag = true;

  setStatus(Readout);

  if(!m_cam->cam_sim_mode){
    mythenDetector myDetector(m_cam->m_detector_id);

    answer=m_cam->mythen_get(myDetector, "flatfield"); // TODO: crash... write PSI people
    if ( answer=="none" || answer=="" ) {
      m_cam->m_flat_field_enabled = false;
      m_cam->m_flat_field_filename = "";
    } else {
      m_cam->m_flat_field_enabled = true;
      m_cam->m_flat_field_filename = answer;
    }

    answer=m_cam->mythen_get(myDetector, "ratecorr");
    FVar=atol(answer.c_str());
    m_cam->m_rate_corr_deadtime = FVar;
   

    answer=m_cam->mythen_get(myDetector, "badchannelsfname");
    m_cam->m_bad_channels_filename = answer; 

    answer=m_cam->mythen_get(myDetector, "angconvfname");
    m_cam->m_ang_conv_filename = answer; 
      
    answer=m_cam->mythen_get(myDetector, "globaloff");
    m_cam->m_global_offset = atof(answer.c_str());
    answer=m_cam->mythen_get(myDetector, "fineoff");
    m_cam->m_fine_offset = atof(answer.c_str());
    answer=m_cam->mythen_get(myDetector, "binsize");
    m_cam->m_bin_size = atof(answer.c_str());
    
    answer=m_cam->mythen_get(myDetector, "positions");
    m_cam->m_positions = answer;

    answer=m_cam->mythen_get(myDetector, "caldir");
    m_cam->m_calibration_dir = answer;
    
    answer=m_cam->mythen_get(myDetector, "trimen");
    m_cam->m_trimenergies = answer;

    answer=m_cam->mythen_get(myDetector, "threaded");
    m_cam->m_threaded = atoi(answer.c_str());
    answer=m_cam->mythen_get(myDetector, "nmod");
    m_cam->m_n_modules = atoi(answer.c_str());
    answer=m_cam->mythen_get(myDetector, "maxmod");
    m_cam->m_n_max_modules = atoi(answer.c_str());

    char tmpstr2[256]="";
    for (int i=0; i<m_cam->m_n_modules; ++i) {
      char tmpstr1[16];
      sprintf(tmpstr1, "modulenumber:%d", i);
      answer=m_cam->mythen_get(myDetector, tmpstr1);
      if (i==0)
	sprintf(tmpstr2, "%s;", answer.c_str());
      else
	sprintf(tmpstr2, "%s %s;", tmpstr2, answer.c_str());
    }
    m_cam->m_module_serial_number = tmpstr2;
    
    answer=m_cam->mythen_get(myDetector, "moduleversion");
    m_cam->m_module_firmware_version = atoi(answer.c_str());
    answer=m_cam->mythen_get(myDetector, "detectornumber");
    m_cam->m_detector_serial_number = atoi(answer.c_str());
    answer=m_cam->mythen_get(myDetector, "detectorversion");
    m_cam->m_detector_firmware_version = atoi(answer.c_str());
    answer=m_cam->mythen_get(myDetector, "softwareversion");
    m_cam->m_detector_software_version = atoi(answer.c_str());
    answer=m_cam->mythen_get(myDetector, "thisversion");
    m_cam->m_mythen_software_version = atoi(answer.c_str());
    answer=m_cam->mythen_get(myDetector, "index");
    m_cam->m_out_fileindex = atoi(answer.c_str());
    

    answer=m_cam->mythen_get(myDetector, "settings");
    m_cam->m_settings = answer.c_str();
    
    answer=m_cam->mythen_get(myDetector, "threshold");
    m_cam->m_threshold = atoi(answer.c_str());
    answer=m_cam->mythen_get(myDetector, "exptime");
    m_cam->m_exposure = atof(answer.c_str());
    answer=m_cam->mythen_get(myDetector, "period");
    m_cam->m_frame_period = atof(answer.c_str());
    answer=m_cam->mythen_get(myDetector, "delay");
    m_cam->m_delay_after_trigger = atof(answer.c_str());
    answer=m_cam->mythen_get(myDetector, "gates");
    m_cam->m_gates_per_frame = atoi(answer.c_str());
    answer=m_cam->mythen_get(myDetector, "frames");
    m_cam->m_frames_per_cycle = atoi(answer.c_str());
    answer=m_cam->mythen_get(myDetector, "cycles");
    m_cam->m_n_cycles = atoi(answer.c_str());
    answer=m_cam->mythen_get(myDetector, "probes");
    m_cam->m_n_probes = atoi(answer.c_str());
    answer=m_cam->mythen_get(myDetector, "dynamicrange");
    m_cam->m_dynamic_range = atoi(answer.c_str());
    answer=m_cam->mythen_get(myDetector, "online");
    m_cam->m_online = atoi(answer.c_str());
    
    answer=m_cam->mythen_get(myDetector, "flags");
    m_cam->m_flags = answer;
    
    //      *m_TrimMode_read: cannot be read
    
    answer=m_cam->mythen_get(myDetector, "trimdir");
    m_cam->m_trim_dir = answer;

    answer=m_cam->mythen_get(myDetector, "outdir");
    m_cam->m_out_dir = answer;

    answer=m_cam->mythen_get(myDetector, "fname");
    m_cam->m_out_filename = answer;

  } else {
    sleep(5);
  }

  setStatus(Ready); 
  
  m_cam->setBusyFlag(false);
  m_cam->RBusyFlag = false;
}


int Camera::CameraThread::getNbAcquiredFrames()
{
  return m_cam->m_acq_frame_nb;
}



Camera::Camera(string HostName, int PortNb, int DetectorId) :
  m_thread(*this), m_start_thread(*this), m_refresh_thread(*this)
{
  DEB_CONSTRUCTOR();

  cam_sim_mode = 0;

  m_host_name = HostName;
  m_port_nb = PortNb;
  m_detector_id = DetectorId;


  if(! cam_sim_mode ){

    // Create a new mythenDetector object
    new_mythen_detector();

  }

  m_camera_busy = 0;

  m_thread.start();
  m_start_thread.start();
  m_refresh_thread.start();

  Refresh();

}

Camera::~Camera()
{
  DEB_DESTRUCTOR();

}


HwBufferCtrlObj* Camera::getBufferCtrlObj()
{
  DEB_MEMBER_FUNCT();

  return &m_buffer_ctrl_obj;
}

Camera::Status Camera::getStatus()
{
  DEB_MEMBER_FUNCT();

  int thread_status = m_thread.getStatus();

  DEB_RETURN() << DEB_VAR1(thread_status);

  if (thread_status == CameraThread::Ready){
    int refresh_status = m_refresh_thread.getStatus();
    if (refresh_status == CameraThread::Readout) return Camera::Readout;
  }

  switch (thread_status) {
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


void Camera::setNbFrames(int nb_frames)
{
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(nb_frames);
  if (nb_frames < 0)
    throw LIMA_HW_EXC(InvalidValue, "Invalid nb of frames");

  m_nb_frames = nb_frames;

}

void Camera::getNbFrames(int& nb_frames)
{
  DEB_MEMBER_FUNCT();
  DEB_RETURN() << DEB_VAR1(m_nb_frames);

  nb_frames = m_nb_frames;
}

/** @brief test if the camera is monochrome
 */
bool Camera::isMonochrome() const
{
  DEB_MEMBER_FUNCT();

  return !strcmp(m_sensor_type,"Mono");
}

VideoMode Camera::getVideoMode() const
{
  DEB_MEMBER_FUNCT();
  DEB_RETURN() << DEB_VAR1(m_video_mode);

  return m_video_mode;
}


void Camera::setVideoMode(VideoMode aMode)
{
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(aMode);

  /*
  ImageType anImageType;
  tPvErr error;
  switch(aMode)
    {
    case Y8:
      error = PvAttrEnumSet(m_handle, "PixelFormat", "Mono8");
      anImageType = Bpp8;
      break;
    case Y16:
      error = PvAttrEnumSet(m_handle, "PixelFormat", "Mono16");
      anImageType = Bpp16;
      break;
    case BAYER_RG8:
      error = PvAttrEnumSet(m_handle, "PixelFormat", "Bayer8");
      anImageType = Bpp8;
      break;
    case BAYER_RG16:
      error = PvAttrEnumSet(m_handle, "PixelFormat", "Bayer16");
      anImageType = Bpp16;
      break;
    default:
      throw LIMA_HW_EXC(InvalidValue,"This video mode is not managed!");
    }
  
  if(error)
    throw LIMA_HW_EXC(Error,"Can't change video mode");
  */
  //  m_video_mode = aMode;
  //  maxImageSizeChanged(Size(m_maxwidth,m_maxheight),anImageType);
}


void Camera::prepareAcq()
{
  DEB_MEMBER_FUNCT();
   
  string answer;

  m_size = MAXSTR * 1;

  /* Get current index */

  if(!cam_sim_mode){
      mythenDetector myDetector(m_detector_id);
      answer=mythen_get(myDetector, "index");
      m_out_fileindex = atoi(answer.c_str());
  } else {
      m_out_fileindex = 1;
  }

  // With prepare acquisition m_nb_frames takes the value from the panel
  
  if(m_frames_per_cycle > 0){
      m_n_cycles = m_nb_frames/m_frames_per_cycle;
  } else {
      m_n_cycles = m_nb_frames;
  }
    
  mythen_set("cycles");

  m_file_in_progress = 0;
}

/** @brief start the acquisition.
    must have m_video != NULL and previously call _allocBuffer
*/
void Camera::startAcq()
{
  DEB_MEMBER_FUNCT();
 
  int err = 0;

  setBusyFlag(true);
  ACBusyFlag = true;
 
  if(!cam_sim_mode){
    int err = probeHost(); // check that host in online
  } 

  if(!err){
    m_thread.sendCmd(CameraThread::StartAcq);
    m_thread.waitNotStatus(CameraThread::Ready);
    m_start_thread.sendCmd(CameraThread::StartStartAcq);
  }

}


void   Camera::stopAcq(){

  DEB_MEMBER_FUNCT();

  m_thread.m_force_stop = true;

  mythenDetector myDetector(m_detector_id);
  char *myArgs[2];
  myArgs[0] = new char[16];
  myArgs[1] = new char[16];
  strcpy(myArgs[0], "status");
  strcpy(myArgs[1], "stop");
  myDetector.executeLine(2, (char **)myArgs, PUT_ACTION);

/*
  if( m_frame ) free( m_frame );
*/
}

void Camera::reset()
{
  DEB_MEMBER_FUNCT();
  //@todo maybe something to do!
}


//-----------------------------------------------------
//
//-----------------------------------------------------
double Camera::exposure() const
{
  DEB_MEMBER_FUNCT();
  DEB_RETURN() << DEB_VAR1(m_exposure);
  //	AutoMutex aLock(m_cond.mutex());
  return m_exposure;
}


void Camera::setExposure(double val)
{

  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(val);

  m_exposure = val;

  mythen_set("exptime");

}


void Camera::setTrigMode(int mode)
{
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(mode);
  
  m_trigger_mode = mode;

}

int Camera::getTrigMode()
{
  DEB_MEMBER_FUNCT();  
  DEB_RETURN() << DEB_VAR1(m_trigger_mode);

  return m_trigger_mode;

}


std::string Camera::getInternalAcqMode(){

  DEB_MEMBER_FUNCT();

  if(m_int_acq_mode == 0){
    DEB_RETURN() << DEB_VAR1("STANDARD");
    return "STANDARD";
  } else if(m_int_acq_mode == 1){
    DEB_RETURN() << DEB_VAR1("CONTINUOUS");
    return "CONTINUOUS";
  } else if(m_int_acq_mode == 2){
    DEB_RETURN() << DEB_VAR1("FOCUS");
    return "FOCUS";
  }
}

void  Camera::setInternalAcqMode(std::string mode){

  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(mode);

  if(mode.compare("STANDARD") == 0){
    m_int_acq_mode = 0;
  } else if (mode.compare("CONTINUOUS") == 0){
    m_int_acq_mode = 1;
  } else if (mode.compare("FOCUS") == 0){
    m_int_acq_mode = 2;
  } else {
    m_int_acq_mode = 0;
  } 
}
    
int Camera::getMaxWidth(){
 
  DEB_MEMBER_FUNCT();
    
  int width =  MAXSTR;
  m_max_width = width;

  DEB_RETURN() << DEB_VAR1(width);

  return width;

}

int Camera::getMaxHeight(){
 
  DEB_MEMBER_FUNCT();
 
  int   height;
  
  height = 1;
  m_max_height = height;

  DEB_RETURN() << DEB_VAR1(height);
        
  return height;

}


void Camera::setBin(const Bin& bin)
{
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(bin);
  
  // m_bin_size is actually a float ...

  m_bin_size = bin.getX();

  mythen_set("binsize");

}

void Camera::getBin(Bin& bin)
{

  DEB_MEMBER_FUNCT();

  // m_bin_size is actually a float ...

  Bin tmp_bin((int)m_bin_size,1);

  bin = tmp_bin;

  DEB_RETURN() << DEB_VAR1(bin);
}

void Camera::checkBin(Bin& bin)
{
  
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(bin);
}

/* Create a new mythenDetector object*/
void Camera::new_mythen_detector()
{
  
  DEB_MEMBER_FUNCT();
  
  static bool first = true;
  static int DetId = 0;
  int err;
  
// An acquisition is running... return to avoid interferences
  if (getBusyFlag()) return;

// Probe that hostname:port accepts connections
  err = probeHost();

  if (err!=0) {
    return;
  }
  
  if (!m_detector || first || DetId!=m_detector_id) {
    // If m_detector does not exist, or first function call, or the detector ID did change...
    
    // 1. delete first m_detector (if already existing)
    if (m_detector) {
      try {
	delete m_detector;
	m_detector = NULL;
      } catch (exception& e) {
	cout << "Mythen::new_mythen_detector() exception: " << e.what() << endl;
      }
    }
    
    // 2. create a new mythenDetector object
    try {
      m_detector = new mythenDetector(m_detector_id);
      
      // Set hostname and port
      mythen_set("hostname");
      mythen_set("port");
      
      first = false;
      DetId = m_detector_id;
    } catch (exception& e) {
      cout << "Mythen::new_mythen_detector() exception: " << e.what() << endl;
    }
  }
}



/* Get the Mythen busy flag */
bool Camera::getBusyFlag() {
  
  DEB_MEMBER_FUNCT();
  return m_camera_busy;
}


/* Set the Busy Flag */
void Camera::setBusyFlag(bool flg) {
  
  DEB_MEMBER_FUNCT();
  m_camera_busy = flg;
}


/* Set RefreshFlag=true, which will cause refresh() to be called */
void Camera::SetRefreshFlag() {
  
  DEB_MEMBER_FUNCT();
  m_refresh_flag = true;
}

/* Check that the input string contains only allowed characters */
bool Camera::isGoodString(string istr) {
  
  DEB_MEMBER_FUNCT();
  bool isgood;
  char cc;
  int ii;
    
  for (ii=0, isgood=true; ii<istr.length(); ++ii) {
    cc = istr[ii];
    if ((cc<'0' || cc>'9') && (cc<'a' || cc>'z') && (cc<'A' || cc>'Z') &&
	cc!='.' && cc!='+' && cc!='-' && cc!='_') {
      isgood = false;
      break;
    }
  }
  
  return isgood;
}


/* Check whether the host is reachable */
int Camera::probeHost() {
  
  DEB_MEMBER_FUNCT();
  int err, fd;
  struct sockaddr_in hostAddr;
  struct hostent *hostDescr;
  
  // Get IP from the hostname
  hostDescr = gethostbyname(m_host_name.c_str());
  if (hostDescr==NULL) {
    return -1;
  }
  bzero(&hostAddr, sizeof(hostAddr));
  hostAddr.sin_family = AF_INET;
  bcopy((char *)hostDescr->h_addr, (char *)(&hostAddr.sin_addr.s_addr), hostDescr->h_length);
  hostAddr.sin_port = htons(m_port_nb);
  

  // Open socket
  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd<0) {
	return fd;
  }
  
  
  // Set the socket timeout to 1 sec
  struct timeval timeoutTime;
  timeoutTime.tv_sec  = 1;
  timeoutTime.tv_usec = 0;
  err = setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (void *) &timeoutTime, sizeof(struct timeval));
  if (err!=0) {
    close(fd);
    return err;
  }

  
  // Try to connect
  err = connect(fd, (struct sockaddr *)(&hostAddr), sizeof(hostAddr));
  if (err<0) {
    close(fd);
    return err;
  }

  // No error: close socket and return 0
    close(fd);
    return 0;
}

/* Check that the input string matches with some of the reference strings
   Returns "true" if a matching is found
*/
bool Camera::check_input(string input, const string *reference, int dimension) {

  
  DEB_MEMBER_FUNCT();
  int ii;
  for (ii=0; ii<dimension; ++ii) {
    if (input==reference[ii]) return true;
  }
  return false;
}

/* Send a "generic" command to the mythenDetector object */
string Camera::MythenExecuteLine(int Action) {
  
  DEB_MEMBER_FUNCT();
  
  char **myArgs;
  string answer="";
  
  // Simulation mode or no detector defined: return without running any command
  if (cam_sim_mode || !m_detector) return "";
  
  // An acquisition is running... return to avoid interferences
  if (getBusyFlag()) return "";
  
  // Only READOUT_ACTION can be run without arguments 
  if (Action != READOUT_ACTION) return "";
  
  try {
    // Send the command
    answer = m_detector->executeLine(0, (char **)myArgs, Action);
    } catch (exception& e) {
    cout << "Camera::MythenExecuteLine() exception: " << e.what() << endl;
  }
  
  // Free the list of arguments
  
  // Return the command answer
  return answer;
}

/* Send a "generic" command to the mythenDetector object */
string Camera::MythenExecuteLine(string line1, int Action) {
  
  DEB_MEMBER_FUNCT();
  
  char *myArgs[1];
  string answer="";
  
  // Simulation mode or no detector defined: return without running any command
    if (cam_sim_mode || !m_detector) return "";
    
// An acquisition is running... return to avoid interferences
//    if (getBusyFlag()) return "";
    
    bool returnFlag=true;
    if (!getBusyFlag()) returnFlag=false; // No acquisition is running
    if ((line1=="online" || line1=="status") && Action==GET_ACTION)
      returnFlag=false; // These can be run also during acquisition
    
    if (returnFlag) return "";
    
    myArgs[0] = new char[line1.length()+1];
    strcpy(myArgs[0], line1.c_str());
    
    try {
      // Send the command
	answer = m_detector->executeLine(1, (char **)myArgs, Action);
    } catch (exception& e) {
	cout << "Camera::MythenExecuteLine() exception: " << e.what() << endl;
    }

    // Free the list of arguments
    delete myArgs[0];
    
    // Return the command answer
    return answer;
}

/* Send a "generic" command to the mythenDetector object */
string Camera::MythenExecuteLine(string line1, string line2, int Action) {
  
  DEB_MEMBER_FUNCT();

  char *myArgs[2];
  string answer="";
  
  // Simulation mode or no detector defined: return without running any command
  if (cam_sim_mode || !m_detector) return "";
  
  // An acquisition is running... return to avoid interferences
  if (getBusyFlag()) return "";

  myArgs[0] = new char[line1.length()+1];
  strcpy(myArgs[0], line1.c_str());
  
  myArgs[1] = new char[line2.length()+1];
  strcpy(myArgs[1], line2.c_str());
  
    try {
      // Send the command
      answer = m_detector->executeLine(2, (char **)myArgs, Action);
    } catch (exception& e) {
      cout << "Camera::MythenExecuteLine() exception: " << e.what() << endl;
    }
    
    // Free the list of arguments
    delete myArgs[0];
    delete myArgs[1];
    
    // Return the command answer
    return answer;
}

/* Send a "standard" command to the mythenDetector object */
void Camera::mythen_set(string var)
{
  
  DEB_MEMBER_FUNCT();
  
  char *myArgs[MAX_ARGS], LStr[L_STRING];
  int ii, myNArg;
  string answer;
  
  myNArg=0;

  // An acquisition is running... return to avoid interferences
  if (getBusyFlag()) return;
  
  // 1. Depending on <var>, the appropriate command line is created
  if (var=="positions" || var=="trimen") {
    
    string line;
    if (var=="positions")
      line = m_positions;
    else
      line = m_trimenergies;
    
    // Create the first argument to the command
    myNArg = 1;
    myArgs[0] = new char[16];
    strcpy(myArgs[0], var.c_str());
    
    if (line.empty()) {
      // The string of positions (trim energies) is empty:
      // use the string "0" instead, which means "no positions" ("no trim energies")
      myNArg = 1;
      myArgs[1]= new char[24];
      strcpy(myArgs[1], "0");
      
    } else {
      // The string of positions (trim energies) is not empty and differs from the one read from the detector:
      // use it in the "PUT_ACTION" command
      
      char *pch;
      strcpy(LStr, line.c_str()); // Must use a copy, otherwise *m_Positions (*m_TrimEnergies) itself would be split...
      pch = strtok(LStr," ");
      while (pch!=NULL && myNArg<MAX_ARGS) {
	// Scan the copy of *m_Positions (*m_TrimEnergies), creating the list of arguments
	myArgs[myNArg]= new char[256];
	strcpy(myArgs[myNArg], pch);
	++myNArg;
	pch = strtok(NULL, " ");
      }
      
      if (atoi(myArgs[1]) > myNArg-2) {
	// Reduce the number of positions/energies to the ones indeed in the command line
	sprintf(myArgs[1], "%d", myNArg-2);
      }	
    }
    
  } else if (var=="ratecorr") {
    
    myNArg=2;
    myArgs[0] = new char[16];
    strcpy(myArgs[0], var.c_str());
    
    myArgs[1] = new char[16];
    if (m_rate_corr_deadtime == 0.) // No rate corrections are applied
      strcpy(myArgs[1], "0");
    else if (m_rate_corr_deadtime < 0) // Default rate corrections are applied
      strcpy(myArgs[1], "-1");
    else
      sprintf(myArgs[1], "%f", m_rate_corr_deadtime);
    
  } else if (var=="flatfield") {

    myNArg=2;
    myArgs[0] = new char[16];
    strcpy(myArgs[0], var.c_str());
    
    myArgs[1] = new char[M_STRING];
    if (!m_flat_field_enabled) // No flat field corrections are applied
      strcpy(myArgs[1], "");
    else
      strcpy(myArgs[1], m_flat_field_filename.c_str());
    
  } else if (var=="badchannelsfname") {
    
    myNArg=2;
    myArgs[0] = new char[16];
    strcpy(myArgs[0], var.c_str());
    
    myArgs[1] = new char[M_STRING];
    strcpy(myArgs[1], m_bad_channels_filename.c_str());
    
  } else if (var=="angconvfname") {
    
    myNArg=2;
    myArgs[0] = new char[16];
    strcpy(myArgs[0], var.c_str());

    myArgs[1] = new char[M_STRING];
    
    strcpy(myArgs[1], m_ang_conv_filename.c_str());
    
  } else if (var=="globaloff" || var=="fineoff" || var=="delay" || var=="nmod" || var=="threshold" ||
	     var=="exptime" || var=="period" || var=="gates" || var=="frames" || var=="cycles" ||
	     var=="probes" || var=="dynamicrange" || var=="trimdir" || var=="caldir" || var=="outdir" ||
	     var=="fname" || var=="index" || var=="threaded" || var=="hostname" || var=="port" ||
	     var=="binsize" || var=="online") {
    myNArg=2;
    myArgs[0] = new char[16];
    strcpy(myArgs[0],var.c_str());
    
    myArgs[1] = new char[M_STRING];
    if (var=="globaloff") {
      sprintf(myArgs[1], "%f", m_global_offset);
    } else if (var=="fineoff") {
      sprintf(myArgs[1], "%f", m_fine_offset);
    } else if (var=="delay") {
      sprintf(myArgs[1], "%f", m_delay_after_trigger);
    } else if (var=="nmod") {
      sprintf(myArgs[1], "%d", m_n_modules);
    } else if (var=="threshold") {
      sprintf(myArgs[1], "%d", m_threshold);
    } else if (var=="exptime") {
      sprintf(myArgs[1], "%f", m_exposure);
    } else if (var=="period") {
      sprintf(myArgs[1], "%f", m_frame_period);
    } else if (var=="gates") {
      sprintf(myArgs[1], "%d", m_gates_per_frame);
    } else if (var=="frames") {
      sprintf(myArgs[1], "%d", m_frames_per_cycle);
    } else if (var=="cycles") {
      sprintf(myArgs[1], "%d", m_n_cycles);
    } else if (var=="probes") {
      sprintf(myArgs[1], "%d", m_n_probes);
    } else if (var=="dynamicrange"){ 
      sprintf(myArgs[1], "%d", m_dynamic_range);
    } else if (var=="trimdir") {
      strcpy(myArgs[1], m_trim_dir.c_str());
    } else if (var=="caldir") {
      strcpy(myArgs[1], m_calibration_dir.c_str());
    } else if (var=="outdir") {
      strcpy(myArgs[1], m_out_dir.c_str());
    } else if (var=="fname") {
      strcpy(myArgs[1], m_out_filename.c_str());
    } else if (var=="index") {
      sprintf(myArgs[1], "%d", m_out_fileindex);
    } else if (var=="threaded") {
      sprintf(myArgs[1], "%d", m_threaded);
    } else if (var=="hostname") {
      strcpy(myArgs[1], m_host_name.c_str());
    } else if (var=="port") {
      sprintf(myArgs[1], "%d", m_port_nb);
    } else if (var=="binsize") {
      sprintf(myArgs[1], "%f", m_bin_size);
    } else if (var=="online") {
      sprintf(myArgs[1], "%d", m_online);
    }
    
    
  } else if (var=="flags") {
    myNArg=2;
    myArgs[0] = new char[16];
    strcpy(myArgs[0],var.c_str());
    
    myArgs[1] = new char[16];
    strcpy(myArgs[1], m_flags.c_str());
    
  }
  
  // If simulation mode or no detector defined: do not send the command

  if (!cam_sim_mode && m_detector){
    try {
      // 2. Send the command
      answer = m_detector->executeLine(myNArg, (char **)myArgs, PUT_ACTION);
      if (var=="threshold") m_threshold = atoi(answer.c_str()); // the actual set value
    } catch (exception& e) {
      cout << "Camera::mythen_set() exception: " << e.what() << endl;
    }
  }

  // 3. Free the list of arguments
  for (ii=0; ii<myNArg; ++ii) {
    strcpy(myArgs[ii],"");
    delete myArgs[ii];
  }  
}


string Camera::mythen_get(mythenDetector myDetector, char *parameter) {
  char *myArgs[1];
  myArgs[0] = new char[64];
  string answer;
  
  try {
    strcpy(myArgs[0], parameter);
    answer=myDetector.executeLine(1, (char **)myArgs, GET_ACTION);
  } catch (exception& e) {
    answer="";
    cout << "Camera::mythen_get() exception: " << e.what() << endl;
  }
  
  return answer;
}

void   Camera::setPositions(std::string positions){
  
  DEB_MEMBER_FUNCT();

  DEB_PARAM() << DEB_VAR1(positions);

  m_positions = positions;

  mythen_set("positions");

}

std::string Camera::getPositions(){
  
  DEB_MEMBER_FUNCT();
 
  DEB_RETURN() << DEB_VAR1(m_positions);

  return m_positions;
}

void   Camera::setTrimEnergies(std::string trimenergies){
  
  DEB_MEMBER_FUNCT();

  DEB_PARAM() << DEB_VAR1(trimenergies);

  m_trimenergies = trimenergies;
  mythen_set("trimenergies");

}

std::string Camera::getTrimEnergies(){
  
  DEB_MEMBER_FUNCT();
 
  DEB_RETURN() << DEB_VAR1(m_trimenergies);

  return m_trimenergies;
}

void   Camera::setThreshold(short threshold){
  
  DEB_MEMBER_FUNCT();

  DEB_PARAM() << DEB_VAR1(threshold);

  m_threshold = threshold;
  mythen_set("threshold");

}

short Camera::getThreshold(){
  
  DEB_MEMBER_FUNCT();
 
  DEB_RETURN() << DEB_VAR1(m_threshold);

  return m_threshold;
}

void  Camera::Refresh()
{
  DEB_MEMBER_FUNCT();
  m_refresh_thread.sendCmd(CameraThread::Refresh);
}

int Camera::getIntParameter(std::string parameter){

  int value;

  if(parameter == "threaded"){ 
    value = m_threaded;
  } else if (parameter == "nmod"){
    value = m_n_modules;
  } else if (parameter == "maxmod"){
    value = m_n_max_modules;
  } else if (parameter == "maxmod"){
    value = m_n_max_modules;
  } else if (parameter == "detectornumber"){
    value = m_detector_serial_number;
  } else if (parameter == "detectorversion"){
    value = m_detector_firmware_version;
  } else if (parameter == "softwareversion"){
    value = m_detector_software_version;
  } else if (parameter == "thisversion"){
    value = m_mythen_software_version;
  } else if (parameter == "gates"){
    value = m_gates_per_frame;
  } else if (parameter == "frames"){
    value = m_frames_per_cycle;
  } else if (parameter == "cycles"){
    value = m_n_cycles;
  } else if (parameter == "probes"){
    value = m_n_probes;
  } else if (parameter == "dynamicrange"){
    value = m_dynamic_range;
  } else if (parameter == "online"){
    value = m_online;
  } else if (parameter == "index"){
    value = m_out_fileindex;
  } else {
    throw LIMA_HW_EXC(Error, "Parameter does not exit");    
  }

  return value;


}

void Camera::setIntParameter(std::string parameter, int value){

  if(parameter == "threaded"){
    m_threaded = value;
  } else if (parameter == "nmod"){
    m_n_modules = value;
  } else if (parameter == "maxmod"){
    m_n_max_modules = value;
  } else if (parameter == "gates"){
    m_gates_per_frame = value;
  } else if (parameter == "frames"){
    m_frames_per_cycle = value;
  } else if (parameter == "cycles"){
      // cycles is set in prepareAcq according to the acq_nb_frames value given in
      // the lima device  
     m_n_cycles = value;
  } else if (parameter == "probes"){
    m_n_probes = value;
  } else if (parameter == "dynamicrange"){
    m_dynamic_range = value;
  } else if (parameter == "online"){
    m_online = value;
  } else if (parameter == "index"){
    m_out_fileindex = value;
  } else if (parameter == "detectornumber" || parameter == "detectorversion" || parameter == "softwareversion" || parameter == "thisversion" ){
    throw LIMA_HW_EXC(Error, "Only read values");
  } else {
    throw LIMA_HW_EXC(Error, "Parameter does not exit");    
  }
   
  mythen_set(parameter);

}

std::string Camera::getIntParameterList(void){

  std::string parameter_list ="cycles,detectornumber,detectorversion,dynamicrange,frames,gates,index,maxmod,moduleversion,nmod,online,probes,softwareversion,thisversion,threaded";

  return parameter_list;

}

float Camera::getFloatParameter(std::string parameter){

  float value;

  if (parameter == "globaloff"){
    value = m_global_offset;
  } else if (parameter == "fineoff"){
    value = m_fine_offset;
  } else if (parameter == "binsize"){
    value = m_bin_size;
  } else if (parameter == "period"){
    value = m_frame_period;
  } else if (parameter == "delay"){
    value = m_delay_after_trigger;
  } else if (parameter == "ratecorr"){
    value = m_rate_corr_deadtime;
  } else {
    throw LIMA_HW_EXC(Error, "Parameter does not exit");    
  }
 
  
  return value;


}

void Camera::setFloatParameter(std::string parameter, float value){

  if (parameter == "globaloff"){
    m_global_offset = value;
  } else if (parameter == "fineoff"){
    m_fine_offset = value;
  } else if (parameter == "binsize"){
    m_bin_size = value;
  } else if (parameter == "period"){
    m_frame_period = value;
  } else if (parameter == "delay"){
    m_delay_after_trigger = value;
  } else if (parameter == "ratecorr"){
    m_rate_corr_deadtime = value;
  } else {
    throw LIMA_HW_EXC(Error, "Parameter does not exit");    
  }
   
  mythen_set(parameter);

}

std::string Camera::getFloatParameterList(void){

  std::string parameter_list ="binsize,delay,fineoff,globaloff,period,ratecorr";

  return parameter_list;

}

std::string Camera::getStringParameter(std::string parameter){

  string value;
  
  if (parameter == "angconvfname"){
    value = m_ang_conv_filename;
  } else if (parameter == "badchannelsfname"){
    value = m_bad_channels_filename;
  } else if (parameter == "caldir"){
    value = m_calibration_dir;
  } else if (parameter == "fname"){
    value = m_out_filename;
  } else if (parameter == "outdir"){
    value = m_out_dir;
  } else if (parameter == "trimdir"){
    value = m_trim_dir;
  } else {
    throw LIMA_HW_EXC(Error, "Parameter does not exit");    
  }
 
  
  return value;


}

void Camera::setStringParameter(std::string parameter, std::string value){

  if (parameter == "angconvfname"){
    m_ang_conv_filename = value;
  } else if (parameter == "badchannelsfname"){
    m_bad_channels_filename = value;
  } else if (parameter == "caldir"){
    m_calibration_dir = value;
  } else if (parameter == "fname"){
    m_out_filename = value;
  } else if (parameter == "outdir"){
    m_out_dir = value;
  } else if (parameter == "trimdir"){
    m_trim_dir = value;
  } else {
    throw LIMA_HW_EXC(Error, "Parameter does not exit");    
  }
   
  mythen_set(parameter);

}

std::string Camera::getStringParameterList(void){

  std::string parameter_list ="angconvfname,badchannelsfname,caldir,fname,outdir,trimdir";

  return parameter_list;

}
