#ifndef AVIEXSYNCCTRLOBJ_H
#define AVIEXSYNCCTRLOBJ_H

#include "Debug.h"
#include "AviexCompatibility.h"
#include "HwSyncCtrlObj.h"
#include "HwInterface.h"

namespace lima
{
  namespace Aviex
  {
    class Camera;

    /*******************************************************************
     * \class SyncCtrlObj
     * \brief Control object providing Aviex synchronization interface
     *******************************************************************/

    class LIBAVIEX_API SyncCtrlObj: public HwSyncCtrlObj
    {
    DEB_CLASS_NAMESPC(DebModCamera, "SyncCtrlObj", "Aviex");

    public:
    	SyncCtrlObj(Camera& cam);
    	virtual ~SyncCtrlObj();

    	virtual bool checkTrigMode(TrigMode trig_mode);
    	virtual void setTrigMode(TrigMode trig_mode);
    	virtual void getTrigMode(TrigMode& trig_mode);

    	virtual void setExpTime(double exp_time);
    	virtual void getExpTime(double& exp_time);

    	virtual void setLatTime(double lat_time);
    	virtual void getLatTime(double& lat_time);

    	virtual void setNbHwFrames(int nb_frames);
    	virtual void getNbHwFrames(int& nb_frames);

    	virtual void getValidRanges(ValidRangesType& valid_ranges);

    private:
    	Camera& m_cam;
    };

  } // namespace Aviex
} // namespace lima

#endif // AVIEXSYNCCTRLOBJ_H
