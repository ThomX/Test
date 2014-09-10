#include "AviexCamera.h"
#include "AviexBinCtrlObj.h"

using namespace lima;
using namespace lima::Aviex;

/*******************************************************************
 * \brief BinCtrlObj constructor
 *******************************************************************/
BinCtrlObj::BinCtrlObj(Camera &cam) : m_cam(cam) 
{
}

//-----------------------------------------------------
//
//-----------------------------------------------------
BinCtrlObj::~BinCtrlObj()
{
    DEB_DESTRUCTOR();
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void BinCtrlObj::setBin(const Bin& aBin)
{  
    DEB_MEMBER_FUNCT();	
	m_cam.setBin(aBin);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void BinCtrlObj::getBin(Bin &aBin)
{
    DEB_MEMBER_FUNCT();	
	m_cam.getBin(aBin);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void BinCtrlObj::checkBin(Bin &aBin)
{  
    DEB_MEMBER_FUNCT();
	m_cam.checkBin(aBin);
}
