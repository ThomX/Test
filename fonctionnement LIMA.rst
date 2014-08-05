Fonctionnement d'une caméra sous LIMA
=====================================

Contexte
---------
- Basler acA640-100gm
- Linux tangoserver 2.6.32-62-generic #126-Ubuntu SMP Fri Jul 4 20:20:04 UTC 2014 i686 GNU/Linux
- caméra essayée sous windows/pylon sans souci


Actions
-------
- déclarer dans ~/.bashrc
| export PYTHONPATH=<my-lima-installation>
| export LD_LIBRARY_PATH=<my-lima-installation>/Lima/lib
- ouvrir Jive
- lancer Tools/Server Wizard :
  * entrer :
    + Server name : LimaCCDs
    + Instance name : 1
  * exécuter
| tangosrv@tangoserver:~/SW_tango/Ressources/Lima/applications/tango$ cd /home/tangosrv/SW_tango/Ressources/Lima/applications/tango/ && python LimaCCDs.py 1
| Warning: EnvHelper could not find LimaCameraType for server cameraTest
| Failed to import EventChannelFactory notifd/factory/tangoserver from the Tango database
| Warning optional plugin bpm_server.beam_viewer can't be load, dependency not satisfied.
| Warning optional plugin bpm_server.webserver can't be load, dependency not satisfied.
| Warning optional plugin bpm_server.xbpmds_webserver can't be load, dependency not satisfied.
| Warning optional plugin bpm_server.TangoBridge can't be load, dependency not satisfied.
| Warning optional plugin bpm_server.xbpmds can't be load, dependency not satisfied.
| For more pulgins dependency  information start server with -v4
  * ne cliquer sur «Next» que après avoir lancé 
| python $PYTHONPATH/Ressources/Lima/applications/tango/LimaCCDs.py 1
  * cliquer sur «Next» dans Tools/Server Wizard
  * dans «Class Selection», sélectionner LimaCCDs puis cliquer sur «Declare device» et entrer:
    + Device name : Maquette/diagnostics/1
    + LimaCameraType : Basler
    + autres : laisser les valeurs par défaut
  * une fois le message «Configuration done» affiché, cliquer sur «New Class» puis sélectionner «Basler» et cliquer sur «Declare device» et entrer:
    + Device name : Maquette/diagnostics/basler
    + cam_ip_address : 192.168.1.104 (une IP libre)
    + autres : laisser les valeurs par défaut
  * quand le message «Server restart : Would you like to reinitialize the server» s'affiche, cliquer sur «Oui»
  * l'instance Lima CCDs s'arrête par une erreur 
| Segmentation fault
ou par l'erreur
| tangosrv@tangoserver:~/SW_tango/Ressources/Lima/applications/tango$ python LimaCCDs.py 1
| Warning: EnvHelper could not find LimaCameraType for server 1
| Failed to import EventChannelFactory notifd/factory/tangoserver from the Tango database
| Warning optional plugin bpm_server.beam_viewer can't be load, dependency not satisfied.
| Warning optional plugin bpm_server.webserver can't be load, dependency not satisfied.
| Warning optional plugin bpm_server.xbpmds_webserver can't be load, dependency not satisfied.
| Warning optional plugin bpm_server.TangoBridge can't be load, dependency not satisfied.
| Warning optional plugin bpm_server.xbpmds can't be load, dependency not satisfied.
| For more pulgins dependency  information start server with -v4
| cam_ip_address 192.168.1.104
| [2014/07/24 16:32:30.038755] b2afcb70     *Camera*Basler::Camera::Camera (BaslerCamera.cpp:227)-Error: Exception(Error): Failed to open 'Basler acA640-100gm#00305313ECBA#192.168.1.104:3956'. The device is controlled by another application. Err: GX status 0xe1018006 (0xE1018006)
| [thrown]
| python: /build/buildd/sip4-qt3-4.10.1/siplib/siplib.c:8599: sipSimpleWrapper_init: Assertion `parseErr != ((void *)0)' failed.
| Abandon
- dans le terminal LimaCCDs, relancer 
| tangosrv@tangoserver:~/SW_tango/Ressources/Lima/applications/tango$ python LimaCCDs.py 1
| Failed to import EventChannelFactory notifd/factory/tangoserver from the Tango database
| Warning optional plugin bpm_server.beam_viewer can't be load, dependency not satisfied.
| Warning optional plugin bpm_server.webserver can't be load, dependency not satisfied.
| Warning optional plugin bpm_server.xbpmds_webserver can't be load, dependency not satisfied.
| Warning optional plugin bpm_server.TangoBridge can't be load, dependency not satisfied.
| Warning optional plugin bpm_server.xbpmds can't be load, dependency not satisfied.
| For more pulgins dependency  information start server with -v4
| cam_ip_address 192.168.1.104
| create device RoiCounterDeviceServer Maquette/roicounter/1
| create device PeakFinderDeviceServer Maquette/peakfinder/1
| create device LimaTacoCCDs Maquette/limatacoccds/1
| create device FlatfieldDeviceServer Maquette/flatfield/1
| create device MaskDeviceServer Maquette/mask/1
| create device BackgroundSubstractionDeviceServer Maquette/backgroundsubstraction/1
| create device LiveViewer Maquette/liveviewer/1
| Warning: setFrameRate() not supported
| create device Roi2spectrumDeviceServer Maquette/roi2spectrum/1
- quand on exécute l'instance LimaViewer dans un nouveau terminal, on obtient l'alerte suivante
| tangosrv@tangoserver:~/SW_tango/Ressources/Lima/applications/tango$ python LimaViewer.py 1
| Failed to import EventChannelFactory notifd/factory/tangoserver from the Tango database
  * après l'installation du device LimaViewer, quyand Jive demande 
| Would you like to reinitialize the server ?
  on obtient le retour 
| segmentation fault
- quand on exécute à nouveau l'instance LimaViewer.py dans son terminal, on obtient à nouveau l'alerte suivante :
| tangosrv@tangoserver:~/SW_tango/Ressources/Lima/applications/tango$ python LimaViewer.py 1
| Failed to import EventChannelFactory notifd/factory/tangoserver from the Tango database
  alors qu'on obtient l'erreur suivante dans le terminal LimaCCDs
| tangosrv@tangoserver:~/SW_tango/Ressources/Lima/applications/tango$ python LimaCCDs.py 1
| Failed to import EventChannelFactory notifd/factory/tangoserver from the Tango database
| Warning optional plugin bpm_server.beam_viewer can't be load, dependency not satisfied.
| Warning optional plugin bpm_server.webserver can't be load, dependency not satisfied.
| Warning optional plugin bpm_server.xbpmds_webserver can't be load, dependency not satisfied.
| Warning optional plugin bpm_server.TangoBridge can't be load, dependency not satisfied.
| Warning optional plugin bpm_server.xbpmds can't be load, dependency not satisfied.
| For more pulgins dependency  information start server with -v4
| cam_ip_address 192.168.1.104
| create device RoiCounterDeviceServer Maquette/roicounter/1
| create device PeakFinderDeviceServer Maquette/peakfinder/1
| create device LimaTacoCCDs Maquette/limatacoccds/1
| create device FlatfieldDeviceServer Maquette/flatfield/1
| create device MaskDeviceServer Maquette/mask/1
| create device BackgroundSubstractionDeviceServer Maquette/backgroundsubstraction/1
| create device LiveViewer Maquette/liveviewer/1
| Warning: setFrameRate() not supported
| create device Roi2spectrumDeviceServer Maquette/roi2spectrum/1
| [2014/07/24 16:37:29.608044] b3378b70 *Camera*_AcqThread::Camera::threadFunction (BaslerCamera.cpp:560)-Error: No image acquired! Error code : 0xhex= e1000014 Error description : GX status 0xe1000014
- quand on arrête LimaViewer dans le terminal, un message d'erreur s'affiche dans l'instance LimaCCDs qui s'arrête
- quand on arrête LimaCCDs dans le terminal, il est impossible de redémarrer l'instance LimaCCDs sans effacer l'instance de Jive et déclarer de nouvelles instances.
| tangosrv@tangoserver:~/SW_tango/Ressources/Lima/applications/tango$ python LimaCCDs.py 1
| Failed to import EventChannelFactory notifd/factory/tangoserver from the Tango database
| Warning optional plugin bpm_server.beam_viewer can't be load, dependency not satisfied.
| Warning optional plugin bpm_server.webserver can't be load, dependency not satisfied.
| Warning optional plugin bpm_server.xbpmds_webserver can't be load, dependency not satisfied.
| Warning optional plugin bpm_server.TangoBridge can't be load, dependency not satisfied.
| Warning optional plugin bpm_server.xbpmds can't be load, dependency not satisfied.
| For more pulgins dependency  information start server with -v4
| cam_ip_address 192.168.1.104
| [2014/07/24 16:40:58.910433] b770e6c0     *Camera*Basler::Camera::Camera (BaslerCamera.cpp:227)-Error: Exception(Error): Failed to open 'Basler acA640-100gm#00305313ECBA#192.168.1.104:3956'. The device is controlled by another application. Err: GX status 0xe1018006 (0xE1018006)
| [thrown]
| python: /build/buildd/sip4-qt3-4.10.1/siplib/siplib.c:8599: sipSimpleWrapper_init: Assertion `parseErr != ((void *)0)' failed.
| Abandon [Abort]
- déboguage : on crée le script suivant pour vérifier
| import time
| from Lima import Basler,Core
| 
| cam = Basler.Camera(cam_ip_add) #replace cam_ip_add with the camera ip
| i = Basler.Interface(cam)
| c = Core.CtControl(i)
| 
| c.prepareAcq()
| c.startAcq()
| 
| status = c.getStatus()
| 
| while status.AcquisitionStatus != Core.AcqReady:
| 	time.sleep(0.1)
| 	status = c.getStatus()
| 
| print status
| print c.ReadImage()
  et le script suivant (avec le port 8000) :
| import time
| from Lima import Basler,Core
| 
| cam = Basler.Camera(cam_ip_add, 8000) #replace cam_ip_add with the camera ip
| i = Basler.Interface(cam)
| c = Core.CtControl(i)
| 
| c.prepareAcq()
| c.startAcq()
| 
| status = c.getStatus()
| 
| while status.AcquisitionStatus != Core.AcqReady:
| 	time.sleep(0.1)
| 	status = c.getStatus()
| 
| print status
| print c.ReadImage()
  Lorsqu'on l'exécute sans lancer ATKPanel ni les instances LIMA, l'erreur suivante s'affiche :
| <AcquisitionStatus=AcqReady, ImageCounters=<LastImageAcquired=0, LastBaseImageReady=0, LastImageSaved=-1, LastCounterReady=-1>
| <type=3 (UINT16)| , dimension_0=659, dimension_1=494, frameNumber=0, timestamp=1.06024, header=< >, buffer=<owner=Mapped, refcount=1, data=0xa219940>>
  et si on relance le script, on obtient :
| [2014/07/31 14:59:01.679245] b5caeb70 *Camera*_AcqThread::Camera::threadFunction (BaslerCamera.cpp:560)-Error: No image acquired! Error code : 0xhex= e1000014 Error description : GX status 0xe1000014
  On essaie 
| export PYLON_GIGE_HEARTBEAT=10000
  mais cela ne change pas grand chose.
