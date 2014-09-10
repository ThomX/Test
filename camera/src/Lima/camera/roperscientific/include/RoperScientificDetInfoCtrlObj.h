#ifndef ROPERSCIENTIFICDETINFOCTRLOBJ_H
#define ROPERSCIENTIFICDETINFOCTRLOBJ_H

#include "Debug.h"
#include "RoperScientific.h"
#include "RoperScientificCompatibility.h"
#include "HwDetInfoCtrlObj.h"


namespace lima {
namespace RoperScientific {

class Camera;
/*******************************************************************
 * \class DetInfoCtrlObj
 * \brief Control object providing RoperScientific detector info interface
 *******************************************************************/

class LIBROPERSCIENTIFIC_API DetInfoCtrlObj: public HwDetInfoCtrlObj {
	DEB_CLASS_NAMESPC(DebModCamera, "DetInfoCtrlObj", "RoperScientific");

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

} // namespace RoperScientific
} // namespace lima

#endif // ROPERSCIENTIFICDETINFOCTRLOBJ_H
