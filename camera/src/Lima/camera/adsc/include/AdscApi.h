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
#ifndef ADSC_API_H
#define ADSC_API_H

#include <vector>
#include "AdscCompatibility.h"
#include "SizeUtils.h"
#include "Exceptions.h"

namespace lima
{


/***************************************************************//**
 * @class AdscApi
 *
 * @brief This class configures and generates frames for the Simulator
 *
 *******************************************************************/
class LIBADSC_API AdscApi
{
  public:
	AdscApi();
	AdscApi( FrameDim &frame_dim, Bin &bin);
	~AdscApi();

	void getFrameDim( FrameDim &dim ) const;
	void setFrameDim( const FrameDim &dim );

	void getBin( Bin &bin ) const;
	void setBin( const Bin &bin );
	void checkBin( Bin &bin ) const;

	void getNextFrame( unsigned char *ptr ) throw (Exception);
	unsigned long getFrameNr();
	void resetFrameNr( int frame_nr=0 );

	void getMaxImageSize(Size& max_size);

  private:
	FrameDim m_frame_dim;                   /// Generated frame dimensions
	Bin m_bin;                              /// "Hardware" Bin

	unsigned long m_frame_nr;

	void checkValid( const FrameDim &frame_dim, const Bin &bin ) throw(Exception);
};


}

#endif /* ADSC_API_H */
