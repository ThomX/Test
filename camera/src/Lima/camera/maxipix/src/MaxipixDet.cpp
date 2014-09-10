//###########################################################################
// This file is part of LImA, a Library for Image Acquisition
//
// Copyright (C) : 2009-2011
// European Synchrotron Radiation Facility
// BP 220, Grenoble 38043
// FRANCE
//
// This is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//###########################################################################
#include <vector>
#include <string>
#include <sstream>

#include "MaxipixDet.h"
#include "PriamAcq.h"

using namespace lima;
using namespace Maxipix;

MaxipixDet::MaxipixDet()
	  :m_xchip(0), m_ychip(0), 
	   m_xgap(0), m_ygap(0),
	   m_type(Bpp16), m_version(MXR2),
           m_mis_cb_act(false)
{
    DEB_CONSTRUCTOR();
    setNbChip(1, 1);
}

MaxipixDet::~MaxipixDet()
{
    DEB_DESTRUCTOR();
}

void MaxipixDet::setVersion(Version version)
{
    DEB_MEMBER_FUNCT();
    m_version= version;
}

void MaxipixDet::setNbChip(int xchip, int ychip)
{
    DEB_MEMBER_FUNCT();
    if ((xchip != m_xchip)||(ychip != m_ychip)) {
        m_xchip= xchip;
        m_ychip= ychip;
        m_nchip= m_xchip*m_ychip;
	_updateSize();
    }
}

void MaxipixDet::setPixelGap(int xgap, int ygap)
{
    DEB_MEMBER_FUNCT();
    if ((xgap != m_xgap)||(ygap != m_ygap)) {
        m_xgap= xgap;
    	m_ygap= ygap;
	_updateSize();
    }
}

void MaxipixDet::setChipsRotation(const std::list<RotationMode>& rotations)
{
    DEB_MEMBER_FUNCT();
    DEB_PARAM() << DEB_VAR1(rotations.size());

    m_rotations = rotations;
}

void MaxipixDet::_updateSize() {
    DEB_MEMBER_FUNCT();
    int w= m_xchip*256 + (m_xchip-1)*m_xgap;
    int h= m_ychip*256 + (m_ychip-1)*m_ygap;
    m_size= Size(w, h);
    if (m_mis_cb_act) {
	maxImageSizeChanged(m_size, m_type);
    }
}
	
void MaxipixDet::getImageSize(Size& size)
{
    DEB_MEMBER_FUNCT();
    size= m_size;
}

void MaxipixDet::getPixelSize(double& x_size, double& y_size)
{
    DEB_MEMBER_FUNCT();
    x_size = y_size = PixelSize;
}

void MaxipixDet::getImageType(ImageType& type)
{
    DEB_MEMBER_FUNCT();
    type= m_type;
}

void MaxipixDet::getDetectorType(std::string& type)
{
    DEB_MEMBER_FUNCT();
    std::ostringstream os;
    os << "MAXIPIX";
    type= os.str();
}

void MaxipixDet::getDetectorModel(std::string& type)
{
    DEB_MEMBER_FUNCT();
    std::ostringstream os;

    if (!m_rotations.size())
      os << m_xchip << "x" << m_ychip << "(gap:" << m_xgap << "x" << m_ygap << ")-";
    else
     os << m_xchip << "x" << m_ychip << "(chip rotation)-";

    switch (m_version) {
      case MPX2:	os << "MPX2" ; break;
      case MXR2:	os << "MXR2"; break;
      case TPX1:	os << "TPX1"; break;
      default:		os << "DUMMY";
    };

    type= os.str();
}

MaxipixReconstruction* MaxipixDet::getReconstructionTask()
{
    DEB_MEMBER_FUNCT();
    MaxipixReconstruction *returnReconstruct = NULL;
    if(!m_xgap && !m_ygap)
      {
	if(!m_rotations.size()) // No reconstruction
	  return NULL;
	
	returnReconstruct = new MaxipixReconstruction(MaxipixReconstruction::M_FREE, MaxipixReconstruction::RAW);
	returnReconstruct->setChipsRotation(m_rotations);
      }
    else if ((m_xchip==2)&&(m_ychip==2)) 
      {
	returnReconstruct = new MaxipixReconstruction(MaxipixReconstruction::M_2x2, MaxipixReconstruction::RAW);
      }
    else if ((m_xchip==5)&&(m_ychip)) 
      {
	returnReconstruct = new MaxipixReconstruction(MaxipixReconstruction::M_5x1, MaxipixReconstruction::RAW);
      }
    else 
      {
	throw LIMA_HW_EXC(Error, "Unknown reconstruction model");
      }
    returnReconstruct->setXnYGapSpace(m_xgap,m_ygap);
    return returnReconstruct;
}

void MaxipixDet::setMaxImageSizeCallbackActive(bool cb_active)
{
    DEB_MEMBER_FUNCT();
    m_mis_cb_act = cb_active;
}
     
