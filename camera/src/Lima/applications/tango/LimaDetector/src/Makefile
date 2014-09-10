CC=g++
CFLAGS=-W -Wall -O3 -I/dsdsds
LDFLAGS=-L/home/xpix_user/PCI_VALIDATED/trunk/sw/xpci_lib \
		   -lxpci_lib -L../third-party/yat/lib/i386/static -lyat
EXEC=ds_LimaDetector
S2SRC= AcquisitionTask.cpp ClassFactory.cpp Factory.cpp LimaDetector.cpp LimaDetectorClass.cpp \
LimaDetectorStateMachine.cpp main.cpp
S2OBJ= $(S2SRC:.cpp=.o)

all: $(EXEC)


ds_LimaDetector: $(S2OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean mrproper

clean:
	rm *.o

mrproper: clean
	rm $(EXEC)
