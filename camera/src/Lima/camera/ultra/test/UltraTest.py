from Lima import Core
from Lima import Ultra

cam=Ultra.Camera()

print cam.getTecColdTemp()
print cam.getTecSupplyVolts()
print cam.getAdcPosSupplyVolts()
print cam.getAdcNegSupplyVolts()
print cam.getVinPosSupplyVolts()
print cam.getVinNegSupplyVolts()
print cam.getHeadADCVdd()
print cam.getHeadVdd()
print cam.getHeadVref()
print cam.getHeadVrefc()
print cam.getHeadVpupref()
print cam.getHeadVclamp()
print cam.getHeadVres1()
print cam.getHeadVres2()
print cam.getHeadVTrip()
print cam.getFpgaXchipReg()
print cam.getFpgaPwrReg()
print cam.getFpgaSyncReg()
print cam.getFpgaAdcReg()
print cam.getFrameCount()
print cam.getFrameCount()
print cam.getFrameErrorCount()
print cam.getHeadPowerEnabled()
print cam.getTecPowerEnabled()
print cam.getBiasEnabled()
print cam.getSyncEnabled()
print cam.getCalibEnabled()
print cam.get8pCEnabled()
print cam.getTecOverTemp()
print cam.getAdcOffset(0)
for i in range(16):
     print cam.getAdcOffset(i)

for i in range(16):
     print cam.getAdcGain(i)
 
print cam.getAux1()
print cam.getAux2()

print cam.getXchipTiming()
# change s1width
cam.setXchipTiming(72,700,36,144,736,36,53,1)
print cam.getXchipTiming()

iface=Ultra.Interface(cam)
ctrl=Core.CtControl(iface)
acq=ctrl.acquisition()
acq.setAcqNbFrames(5)

saving=cont.saving()
saving.setDirectory('/home/ppc/users/id24-data')
saving.setFormat(Core.CtSaving.EDF)
saving.setPrefix('ultra_')
saving.setSuffix('.edf')
saving.setSavingMode(Core.CtSaving.AutoFrame)
saving.setOverwritePolicy(Core.CtSaving.Append)

ctrl.prepareAcq()
ctrl.startAcq()

