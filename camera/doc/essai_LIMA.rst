essais LIMA (10/09/2014)
========================

Configuration
-------------
- Linux tangoserver 2.6.32-65-generic #131-Ubuntu SMP Fri Aug 15 20:39:11 UTC 2014 i686 GNU/Linux
- 00:19.0 Ethernet controller: Intel Corporation 82578DM Gigabit Network Connection (rev 05)
- version LIMA compilée en respectant la doc LIMA 1.3
- caméra Basler ACE acA640-100gm (renommée par Basler dernièrement acA640-120gm)

Essais précédents faits :
-------------------------
- nous arrivons à voir des images avec Pylon sous windows et sous linux, ce n'est donc a priori ni un problème de matériel, ni de système


1er essai
---------

- lancement des DS LimaViewer et LimaCCDs dans un terminal :
- lancement de Astor
- on peut arrêter les DS via Astor mais pas les démarrer : lorsqu'on arrête le DS LimaCCDs ou LimaViewer, le message suivant s'affiche :

| LimaCCDs :  not found in 'StartDsPath' property:
| - /home/tangosrv/SW_tango/ArchivingRoot12.4.2/device/linux
| - /home/tangosrv/SW_tango/ArchivingRoot/device/linux
| - /home/tangosrv/SW_tango/DeviceServers
| - /usr/lib/tango

  Cliquer sur «Details» affiche un seconde fois le même message. Il faut alors redémarrer le DS à la main (dans le terminal).


2e essai
--------

- lancement des ATKpanel de LimaViewer et LimaCCDs via Jive
- quand on change le paramètre acq_nb_frames dans LimaCCDs, il est répercuté sur LimaViewer, mais pas pris en compte avant de redémarrer l'acquisition (stopAcq, prepareAcq, startAcq).
  * quand acq_nb_frames est négatif, on obtiend une exception ;
  * quand acq_nb_frames est strictement positif, le redémarrage met le LimaViewer dans l'état FAULT (rouge, mais dans Astor, il est toujours vert) ;
  * repasser acq_nb_frames à 0 et redémarrer l'acquisition (stopAcq, prepareAcq, startAcq) remet LimaViewer à l'état ON (vert).
- quand on change un autre paramètre dans LimaCCDs que acq_nb_frames, il est répercuté sur LimaCCDs, sans poser de problème après redémarrage de l'acquisition (stopAcq, prepareAcq, startAcq).
- quand on change le paramètre acq_nb_frames dans LimaViewer, il est répercuté sur LimaViewer, mais pas pris en compte avant de redémarrer l'acquisition (stopAcq, prepareAcq, startAcq).
  * quand acq_nb_frames est négatif, on obtiend une exception ;
  * quand acq_nb_frames est strictement positif, le redémarrage de LimaViewer (init, start) met le LimaViewer dans l'état FAULT (rouge, mais dans Astor, il est toujours vert) ;
  * repasser acq_nb_frames à 0 et redémarrer LimaViewer (iit, start) remet LimaViewer à l'état ON (vert).


- quand LimaViewer est à l'état FAULT et LimaCCDs à l'état ON, la sortie du DS LimaCCDs est :
[2014/09/10 11:20:57.0bd823] b29deb70 *Control*Control::Shutter::getMode (CtShutter.cpp:115)-Error: Exception(Error): No shutter capability [thrown]
[2014/09/10 11:20:57.794258] b29deb70 *Application*LimaCCDs::LimaCCDs::read_acc_saturated_cblevel-Error: Accumulation threshold plugins not loaded
[2014/09/10 11:20:57.803187] b31dfb70 *Camera*_AcqThread::Camera::threadFunction (BaslerCamera.cpp:560)-Error: No image acquired! Error code : 0xhex= e1000014 Error description : GX status 0xe1000014
[2014/09/10 11:20:57.0c96a2] b29deb70     *Control*Control::Shutter::getCloseTime (CtShutter.cpp:151)-Error: Exception(Error): No shutter capability [thrown]
[2014/09/10 11:20:57.835558] b29deb70     *Control*Control::Shutter::getOpenTime (CtShutter.cpp:139)-Error: Exception(Error): No shutter capability [thrown]
[2014/09/10 11:20:57.863893] b31dfb70 *Camera*_AcqThread::Camera::threadFunction (BaslerCamera.cpp:560)-Error: No image acquired! Error code : 0xhex= e1000014 Error description : GX status 0xe1000014

- quand LimaCCDs et LimaViewer sont à l'état ON, la sortie du DS LimaCCDs est :
[2014/09/10 11:23:08.0763d3] b29deb70 *Control*Control::Shutter::getMode (CtShutter.cpp:115)-Error: Exception(Error): No shutter capability [thrown]
[2014/09/10 11:23:08.501949] b29deb70 *Application*LimaCCDs::LimaCCDs::read_acc_saturated_cblevel-Error: Accumulation threshold plugins not loaded
[2014/09/10 11:23:08.503732] b31dfb70 *Camera*_AcqThread::Camera::threadFunction (BaslerCamera.cpp:560)-Error: No image acquired! Error code : 0xhex= e1000014 Error description : GX status 0xe1000014
[2014/09/10 11:23:08.082403] b29deb70     *Control*Control::Shutter::getCloseTime (CtShutter.cpp:151)-Error: Exception(Error): No shutter capability [thrown]
[2014/09/10 11:23:08.544642] b29deb70     *Control*Control::Shutter::getOpenTime (CtShutter.cpp:139)-Error: Exception(Error): No shutter capability [thrown]


3e essai
--------

- installation de LIMA sous Windows afin de comparer le fonctionnement
- Après enregistrement du server LimaCCDS avec le "Tools server Wizard" de jive et le démarrage en ligne de commande, nous obtenons une erreur :

C:\Python27\Lib\site-packages>python.exe LimaCCDs.py 1
Failed to import EventChannelFactory notifd/factory/controlthomx1.lal.in2p3.fr from the Tango database
Warning optional plugin bpm_server.beam_viewer can't be load, dependency not satisfied.
Warning optional plugin bpm_server.TangoBridge can't be load, dependency not satisfied.
Warning optional plugin bpm_server.webserver can't be load, dependency not satisfied.
Warning optional plugin bpm_server.xbpmds can't be load, dependency not satisfied.
Warning optional plugin bpm_server.xbpmds_webserver can't be load, dependency not satisfied.
For more pulgins dependency  information start server with -v4


Clique sur next du server wizard mais problème, la class LimaCCDS est bien créée mais pas la class Balser ! 
Après un clic sur "déclaration device" et modification de la propriété "LimaCameraType" avec la valeur "Basler", il n'y a toujours pas de classe Basler.


Interprétation
--------------
- Il est possible qu'un paramétrage soit nécessaire avant de pouvoir utiliser la caméra en acquisition.
- DÉCRIRE ICI LES DIFFÉRENCES ENTRE LES INFOS DE LA DOC LIMA ET CE QUI APPARAIT EN SALLE MAQUETTE
