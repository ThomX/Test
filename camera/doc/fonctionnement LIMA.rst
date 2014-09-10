Fonctionnement d'une caméra sous LIMA :
=======================================


Configuration :
---------------

- ouvrir Jive
- lancer Tools/Server Wizard :

  * entrer :

    + Server name : LimaCCDs
    + Instance name : 1

  * ne cliquer sur «Next» que après avoir lancé «python $PYTHONPATH/Ressources/Lima/applications/tango/LimaCCDs.py 1»
  * cliquer sur «Next» dans Tools/Server Wizard
  * dans «Class Selection», sélectionner LimaCCDs puis cliquer sur «Declare device» et entrer:

    + Device name : Maquette/diagnostics/1
    + LimaCameraType : Basler
    + autres : laisser les valeurs par défaut

  * une fois le message «Configuration done» affiché, cliquer sur «New Class» puis sélectionner «Basler» et cliquer sur «Declare device» et entrer:

    + Device name : Maquette/diagnostics/basler
    + cam_ip_address : 192.168.1.104 (une IP libre)
    + autres : laisser les valeurs par défaut

  * quand le message «Server restart : Would you like to reinitialize the server» s'affiche, cliquer sur «Oui» (une Segmentation fault apparaît mais ça semble «normal»). Dans le terminal LimaCCDs, relancer «python $PYTHONPATH/Ressources/Lima/applications/tango/LimaCCDs.py 1»
  * cliquer sur «Finish»

- lancer Tools/Server Wizard :
  * configurer :

    + Server name : LimaViewer
    + Instance name : 1

  * ne cliquer sur «Next» que après avoir lancé «python $PYTHONPATH/Ressources/Lima/applications/tango/LimaViewer.py 1»
  * cliquer sur «Next» dans Tools/Server Wizard
  * dans «Class Selection», sélectionner LimaViewer puis cliquer sur «Declare device» et entrer :

    + Device name : Maquette/diagnostics/viewer
    + Dev_Ccd_name : Maquette/diagnostics/1

  * cliquer sur «Next» puis «Finish»
  * quand le message «Server restart : Would you like to reinitialize the server» s'affiche, cliquer sur «Oui» puis dans le terminal LimaCCDs, relancer «python $PYTHONPATH/Ressources/Lima/applications/tango/LimaViewer.py 1»




Erreurs :
---------
- Le message suivant s'affiche après configuration de la classe Basler :
  | server restart : Would you like to reinitialize the server

  quand on clique sur «Oui», une Segmentation fault apparaît -> ça semble «normal»

- L'avertissement (warning) suivant s'affiche :
  | Failed to import EventChannelFactory notifd/factory/tangoserver from the Tango database

  -> le démon notify_daemon est non démarré. Si TANGO 8 est installé, ce message peut être ignoré car Zmq remplace ce démon.

- Le message suivant s'affiche
  | acA640-100gm#00305313ECBA#192.168.1.104:3956'. The device is controlled 
  | by another application. Err: GX status 0xe1018006 (0xE1018006)
  | [thrown]

  lorsque Pylon viewer fonctionne en parallèle et empêche l'accès à la caméra.

- lorsque LimaViewer est arrêté, les messages d'acquisition cessent. -> LimaViewer est juste un objet appelant une fonction de l'objet LIMA qui instancie un module de type Basler. Si aucun LimaViewer ne fonctionne, il n'y a aucun client pour l'objet LIMA (donc aucune erreur d'acquisition).

- Le message suivant s'affiche :
  | <AcquisitionStatus=AcqReady, ImageCounters=<LastImageAcquired=0, 
  |   LastBaseImageReady=0, LastImageSaved=-1, LastCounterReady=-1>
  | <type=3 (UINT16), dimension_0=659, dimension_1=494, frameNumber=0, 
  |   timestamp=1.06024, header=< >, buffer=<owner=Mapped, refcount=1, 
  |   data=0xa219940>>

  -> la caméra fonctionne et une image a été acquise
