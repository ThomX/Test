#ifndef AVIEXBINCTRLOBJ_H
#define AVIEXBINCTRLOBJ_H

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
     * \class BinCtrlObj
     * \brief Control object providing Basler Bin interface
     *******************************************************************/
    class LIBAVIEX_API BinCtrlObj : public HwBinCtrlObj
    {
    	DEB_CLASS_NAMESPC(DebModCamera, "BinCtrlObj", "Aviex"); 
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

  } // namespace Aviex
} // namespace lima

#endif // AVIEXBINCTRLOBJ_H
