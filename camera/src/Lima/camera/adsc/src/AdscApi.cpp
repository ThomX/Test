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
/***************************************************************//**
 * @file AdscApi.cpp
 * @brief This file contains the AdscApi class implementation
 *
 * @author A.Kirov
 * @date 03/06/2009
 *******************************************************************/

#include <ctime>
#include <cmath>
#include <vector>
#ifdef __unix
#include <sys/time.h>
#else
#include <time_compat.h>
#endif
#include <unistd.h>
#include "AdscApi.h"
#include "SizeUtils.h"


using namespace lima;
using namespace std;


/***************************************************************//**
 * @brief AdscApi class default constructor
 *
 *******************************************************************/
AdscApi::AdscApi()
{
	m_frame_dim = FrameDim(3072, 3072, Bpp16);
	m_bin = Bin(1,1);
	m_frame_nr = 0;
}


/***************************************************************//**
 * @brief AdscApi class constructor setting member variables
 *
 * Before setting we check the values for consistency
 *
 * @param[in] frame_dim    The frame dimensions
 * @param[in] peaks        A vector of GaussPeak structures
 * @param[in] grow_factor  Peaks grow % with each frame
 *******************************************************************/
AdscApi::AdscApi( FrameDim &frame_dim, Bin &bin)
{
	checkValid(frame_dim, bin);

	m_frame_dim = frame_dim;
	m_bin = bin; 

	m_frame_nr = 0;
}


/***************************************************************//**
 * @brief AdscApi class destructor
 *
 *******************************************************************/
AdscApi::~AdscApi()
{
}


/***************************************************************//**
 * @brief Checks the consistency of FrameDim, Bin and RoI
 *
 * First checks if Binning is valid
 * Then checks if FrameDim is inside of the MaxImageSize
 * Finally checks if the RoI is consistent with the binned frame dim
 *******************************************************************/
void AdscApi::checkValid( const FrameDim &frame_dim, const Bin &bin) throw(Exception)
{
	Size max_size;
	getMaxImageSize( max_size );

	Bin valid_bin = bin;
	checkBin(valid_bin);
	if (valid_bin != bin)
		throw LIMA_HW_EXC(InvalidValue, "Invalid bin");

	if( (frame_dim.getSize().getWidth()  > max_size.getWidth()) ||
	    (frame_dim.getSize().getHeight() > max_size.getHeight()) )
		throw LIMA_HW_EXC(InvalidValue, "Frame size too big");

	FrameDim bin_dim = frame_dim / bin;

}


/***************************************************************//**
 * @brief Gets frame dimention
 *
 * @param[out] dim  FrameDim object reference
 *******************************************************************/
void AdscApi::getFrameDim( FrameDim &dim ) const
{
	dim = m_frame_dim;
}


/***************************************************************//**
 * @brief Sets frame dimention
 *
 * @param[in] dim  FrameDim object reference
 *******************************************************************/
void AdscApi::setFrameDim( const FrameDim &dim )
{
	checkValid(dim, m_bin);

	m_frame_dim = dim;

	// Reset Bin
}


/***************************************************************//**
 * @brief Gets the Binning
 *
 * @param[out] bin  Bin object reference
 *******************************************************************/
void AdscApi::getBin( Bin &bin ) const
{
	bin = m_bin;
}


/***************************************************************//**
 * @brief Sets the Binning
 *
 * @param[in] bin  Bin object reference
 *******************************************************************/
void AdscApi::setBin( const Bin &bin )
{
	checkValid(m_frame_dim, bin);

	m_bin = bin;
}


/***************************************************************//**
 * @brief Returns the closest Binning supported by the "hardware"
 *
 * @param[in,out] bin  Bin object reference
 *******************************************************************/
void AdscApi::checkBin( Bin &bin ) const
{
	if ((bin == Bin(1,1)) || (bin == Bin(1,2)) || (bin == Bin(2,1)))
		bin = Bin(1,1);
	else
		bin = Bin(2,2);
}

/***************************************************************//**
 * @brief Fills the next frame into the buffer
 *
 * @param[in] ptr  an (unsigned char) pointer to an allocated buffer
 *
 * @exception lima::Exception  The image depth is not 1,2 or 4
 *******************************************************************/
void AdscApi::getNextFrame( unsigned char *ptr ) throw (Exception)
{
	switch( m_frame_dim.getDepth() ) {
		case 1 :
//			fillData<unsigned char>(ptr);
			break;
		case 2 :
//			fillData<unsigned short>(ptr);
			break;
		case 4 :
//			fillData<unsigned long>(ptr);
			break;
		default:
			throw LIMA_HW_EXC(NotSupported, "Invalid depth");
	}
	++m_frame_nr;
}


/***************************************************************//**
 * @brief Sets the internal frame number to a value. Default is 0.
 *
 * @param[in] frame_nr  int  The frame number, or nothing
 *******************************************************************/
void AdscApi::resetFrameNr( int frame_nr )
{
	m_frame_nr = frame_nr;
}


/***************************************************************//**
 * @brief Gets the internal frame number
 *
 * @return  unsigned long  The frame number.
 *******************************************************************/
unsigned long AdscApi::getFrameNr()
{
	return m_frame_nr;
}


/***************************************************************//**
 * @brief Gets the maximum "hardware" image size
 *
 * @param[out]  max_size  Reference to a Size object
 *******************************************************************/
void AdscApi::getMaxImageSize(Size& max_size)
{
	int max_dim = 3072;
	max_size = Size(max_dim, max_dim);
}
