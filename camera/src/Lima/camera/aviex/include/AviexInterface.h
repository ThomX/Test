#ifndef AVIEXINTERFACE_H
#define AVIEXINTERFACE_H

#include "Debug.h"
#include "AviexCompatibility.h"
#include "AviexDetInfoCtrlObj.h"
#include "AviexSyncCtrlObj.h"
#include "AviexBinCtrlObj.h"
#include "AviexRoiCtrlObj.h"
#include "HwInterface.h"
#include "HwBufferMgr.h"


using namespace std;

namespace lima
{
namespace Aviex
{
class Camera;
/*******************************************************************
 * \class Interface
 * \brief Aviex hardware interface
 *******************************************************************/

class LIBAVIEX_API Interface: public HwInterface
{
DEB_CLASS_NAMESPC(DebModCamera, "AviexInterface", "Aviex");

public:
	Interface(Camera& cam);
	virtual ~Interface();

	//- From HwInterface
	virtual void getCapList(CapList&) const;
	virtual void reset(ResetLevel reset_level);
	virtual void prepareAcq();
	virtual void startAcq();
	virtual void stopAcq();
	virtual void getStatus(StatusType& status);
	virtual int getNbHwAcquiredFrames();
	
    //! get the camera object to access it directly from client
    Camera& getCamera() { return m_cam;}

private:
	Camera& m_cam;
	CapList m_cap_list;
	DetInfoCtrlObj m_det_info;
	SyncCtrlObj m_sync;    
	BinCtrlObj m_bin;
	RoiCtrlObj m_roi;    
};

} // namespace Aviex
} // namespace lima

#endif // AVIEXINTERFACE_H
