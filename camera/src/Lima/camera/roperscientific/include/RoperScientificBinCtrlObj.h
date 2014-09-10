#ifndef ROPERSCIENTIFICBINCTRLOBJ_H
#define ROPERSCIENTIFICBINCTRLOBJ_H

#include "Debug.h"
#include "RoperScientific.h"
#include "RoperScientificCompatibility.h"
#include "HwInterface.h"
#include "RoperScientificCamera.h"

namespace lima
{
  namespace RoperScientific
  {
    class Camera;
    /*******************************************************************
     * \class BinCtrlObj
     * \brief Control object providing Basler Bin interface
     *******************************************************************/
    class LIBROPERSCIENTIFIC_API BinCtrlObj : public HwBinCtrlObj
    {
    	DEB_CLASS_NAMESPC(DebModCamera, "BinCtrlObj", "RoperScientific"); 
    public:
    	BinCtrlObj(Camera& cam);
    	virtual ~BinCtrlObj();

    	virtual void setBin(const Bin& bin);
    	virtual void getBin(Bin& bin);
    	//allow all binning
    	virtual void checkBin(Bin& bin);
    private:
    	Camera& m_cam;

    };

  } // namespace RoperScientific
} // namespace lima

#endif // ROPERSCIENTIFICBINCTRLOBJ_H
