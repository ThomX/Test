rayonixhs-objs = FrameStatusCb.o RayonixHsCamera.o RayonixHsInterface.o RayonixHsDetInfoCtrlObj.o RayonixHsSyncCtrlObj.o RayonixHsBinCtrlObj.o RayonixHsShutterCtrlObj.o

SRCS = $(rayonixhs-objs:.o=.cpp)

#CXXFLAGS += -I../include -I../../../hardware/include -I../../../common/include -I../sdk/include/craydl\

CXXFLAGS += -I../include -I../../../hardware/include -I../../../common/include -I/opt/rayonix/include/craydl\
            -I/opt/rayonix/include \
            -I/opt/rayonix/include/marccd \
	    -pthread -fPIC

all:	RayonixHs.o

RayonixHs.o:	$(rayonixhs-objs)
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
