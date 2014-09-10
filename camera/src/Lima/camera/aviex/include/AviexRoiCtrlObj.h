#ifndef AVIEXROICTRLOBJ_H
#define AVIEXROICTRLOBJ_H

#include "Debug.h"
#include "AviexCompatibility.h"
#include "HwInterface.h"
#include "AviexCamera.h"

namespace lima
{
  namespace Aviex
  {
    class Camera;

    /*******************************************************************
     * \class RoiCtrlObj
     * \brief Control object providing Basler Roi interface
     *******************************************************************/

    class LIBAVIEX_API RoiCtrlObj : public HwRoiCtrlObj
    {
    	DEB_CLASS_NAMESPC(DebModCamera, "RoiCtrlObj", "Aviex");

    public:
    	RoiCtrlObj(Camera& cam);
    	virtual ~RoiCtrlObj();

    	virtual void setRoi(const Roi& set_roi);
    	virtual void getRoi(Roi& hw_roi);
    	virtual void checkRoi(const Roi& set_roi, Roi& hw_roi);

    private:
    	Camera& m_cam;
    };

  } // namespace Aviex
} // namespace lima

#endif // AVIEXROICTRLOBJ_H
