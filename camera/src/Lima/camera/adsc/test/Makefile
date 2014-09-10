############################################################################
# This file is part of LImA, a Library for Image Acquisition
#
# Copyright (C) : 2009-2011
# European Synchrotron Radiation Facility
# BP 220, Grenoble 38043
# FRANCE
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
############################################################################

std-objs = ../../../common/src/Common.o ../../../hardware/src/Hw.o
adsc-objs = ../src/Adsc.o

test-objs = test.o
test-ext-objs = $(adsc-objs)

testadsc-objs = testadsc.o
testadsc-ext-objs = $(adsc-objs) $(std-objs)

testadschwinterface-objs = testadschwinterface.o
testadschwinterface-ext-objs = $(adsc-objs) $(std-objs)

SRCS = $(test-objs:.o=.cpp) $(testadsc-objs:.o=.cpp) \
       $(testadschwinterface-objs:.o=.cpp)

INC = -I../include -I../../../common/include -I../../../hardware/include -I../../../third-party/Processlib/core/include
CXXFLAGS += $(INC) -Wall -pthread -g
#LDFLAGS = ../sdk/adsc_sources/lib/linux/libdetcon_th.a -L../../../third-party/Processlib/build -lprocesslib -pthread
LDFLAGS = -L../sdk/adsc_sources/lib/linux -L../../../third-party/Processlib/build -ldetcon_th -lauxlib -lprocesslib -pthread

all: test testadsc testadschwinterface

test:	$(test-objs) $(test-ext-objs) $(std-objs)
	$(CXX) $(LDFLAGS) -o $@ $+

testadsc: $(testadsc-objs) $(testadsc-ext-objs) 
	$(CXX) $(LDFLAGS) -o $@ $+

testadschwinterface: $(testadschwinterface-objs) \
		     $(testadschwinterface-ext-objs) 
	$(CXX) $(LDFLAGS) -o $@ $+

clean: 
	rm -f $(test-objs) test *.edf
	rm -f $(testadsc-objs) testadsc
	rm -f $(testadschwinterface-objs) testadschwinterface
	rm -f *.P

%.o : %.cpp
	$(COMPILE.cpp) -MD $(CXXFLAGS) -o $@ $<
	@cp $*.d $*.P; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	-e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
	rm -f $*.d

-include $(SRCS:.cpp=.P)
