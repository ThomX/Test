mythen-objs =  MythenInterface.o MythenCamera.o
SRCS = $(mythen-objs:.o=.cpp) 

CXXFLAGS += -I/usr/local/include/slsdetector -I../include -I../../../hardware/include -I../../../common/include \
	-pthread -fPIC -g 

all:	Mythen.o

Mythen.o:	$(mythen-objs)
	$(LD) -o $@ -r $+

clean:
	rm -f *.o *.P

%.o : %.cpp
	$(COMPILE.cpp) -MD $(CXXFLAGS) -o $@ $<
	@cp $*.d $*.P; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	-e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
	rm -f $*.d

-include $(SRCS:.cpp=.P)

.PHONY: check-syntax
check-syntax:
	$(CXX) -Wall -Wextra -fsyntax-only $(CXXFLAGS) $(CHK_SOURCES)
