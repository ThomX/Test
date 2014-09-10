namespace Mythen
{
  class Interface : HwInterface
  {
%TypeHeaderCode
#include <MythenInterface.h>
%End
  public:
    Interface(Mythen::Camera&);
    virtual ~Interface();

    virtual void getCapList(std::vector<HwCap> &cap_list /Out/) const;
    virtual void reset(ResetLevel reset_level);
    virtual void prepareAcq();
    virtual void startAcq();
    virtual void stopAcq();
    virtual void getStatus(StatusType& status /Out/);

    virtual int getNbHwAcquiredFrames();
    virtual int getNbAcquiredFrames();

  };

	

   



};
