aviex-objs = AviexCamera.o AviexInterface.o AviexDetInfoCtrlObj.o AviexSyncCtrlObj.o AviexRoiCtrlObj.o AviexBinCtrlObj.o

SRCS = $(aviex-objs:.o=.cpp) 

CXXFLAGS += -I../include -I../../../hardware/include -I../../../common/include \
			-I../sdk/linux/include \
			-DOS_UNIX -Wall -pthread -fPIC -g

all:	Aviex.o

Aviex.o:	$(aviex-objs)
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