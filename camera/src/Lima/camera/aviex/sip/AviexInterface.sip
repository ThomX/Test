namespace Aviex
{
  /*******************************************************************
   * \class Interface
   * \brief Aviex hardware interface
   *******************************************************************/

  class Interface: HwInterface
  {
%TypeHeaderCode
#include <AviexInterface.h>
%End
  public:
    Interface(Aviex::Camera& cam);
    virtual ~Interface();

    //- From HwInterface
    //virtual void getCapList(CapList&) const;
    virtual void reset(ResetLevel reset_level);
    virtual void prepareAcq();
    virtual void startAcq();
    virtual void stopAcq();
    virtual void getStatus(StatusType& status /Out/);
    virtual int getNbHwAcquiredFrames();
  };

}; // namespace Aviex

