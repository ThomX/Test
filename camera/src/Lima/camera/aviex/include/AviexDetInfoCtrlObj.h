#ifndef AVIEXDETINFOCTRLOBJ_H
#define AVIEXDETINFOCTRLOBJ_H

#include "Debug.h"
//#include "Aviex.h"
#include "AviexCompatibility.h"
#include "HwDetInfoCtrlObj.h"


namespace lima {
namespace Aviex {

class Camera;
/*******************************************************************
 * \class DetInfoCtrlObj
 * \brief Control object providing Aviex detector info interface
 *******************************************************************/

class LIBAVIEX_API DetInfoCtrlObj: public HwDetInfoCtrlObj {
	DEB_CLASS_NAMESPC(DebModCamera, "DetInfoCtrlObj", "Aviex");

public:
	DetInfoCtrlObj(Camera& cam);
	virtual ~DetInfoCtrlObj();

	virtual void getMaxImageSize(Size& max_image_size);
	virtual void getDetectorImageSize(Size& det_image_size);

	virtual void getDefImageType(ImageType& def_image_type);
	virtual void getCurrImageType(ImageType& curr_image_type);
	virtual void setCurrImageType(ImageType curr_image_type);

	virtual void getPixelSize(double& x_size, double &y_size);
	virtual void getDetectorType(std::string& det_type);
	virtual void getDetectorModel(std::string& det_model);

	virtual void registerMaxImageSizeCallback(HwMaxImageSizeCallback& cb);
	virtual void unregisterMaxImageSizeCallback(HwMaxImageSizeCallback& cb);

private:
	Camera& m_cam;
};

} // namespace Aviex
} // namespace lima

#endif // AVIEXDETINFOCTRLOBJ_H
