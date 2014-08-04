Fonctionnement d'une caméra sous LIMA
=====================================

- ouvrir Jive
- lancer Tools/Server Wizard :
  * entrer :

    + Server name : LimaCCDs
    + Instance name : 1

  * lancer

| LimaCCDs.py 1

  * erreur :
| tangosrv@tangoserver:~/SW_tango/Lima/Core/v1.3.0$ python /home/tangosrv/SW_tango/Ressources/Lima/applications/tango/LimaCCDs.py 1
| Traceback (most recent call last):
| File "<string>", line 1, in <module>
| ImportError: No module named Lima
| ImportError: Traceback (most recent call last):
| File "LimaCCDs.py", line 62, in <module>
| 	from Lima import Core
| ImportError: No module named Lima 

  On peut la résoudre en déclarant 

| export PYTHONPATH=$INSTALL_DIR

  La bibiliothèque semble être $INSTALL_DIR/Lima/Core/v1.3.0/limacore.so mais ça ne permet pas de trouver processlib qui semble être $INSTALL_DIR/Lima/Core/v1.3.0/processlib.so

| tangosrv@tangoserver:~/SW_tango/Lima/Core/v1.3.0$ python /home/tangosrv/SW_tango/Ressources/Lima/applications/tango/LimaCCDs.py 1
| Traceback (most recent call last):
|  File "<string>", line 1, in <module>
|  File "/home/tangosrv/SW_tango/Lima/Core/__init__.py", line 30, in <module>
|    import processlib as Processlib
| ImportError: libprocesslib.so.1.3: cannot open shared object file: No such file or directory
| Traceback (most recent call last):
|  File "/home/tangosrv/SW_tango/Ressources/Lima/applications/tango/LimaCCDs.py", line 62, in <module>
|    from Lima import Core
|  File "/home/tangosrv/SW_tango/Lima/Core/__init__.py", line 30, in <module>
|    import processlib as Processlib
| ImportError: libprocesslib.so.1.3: cannot open shared object file: No such file or directory

  * Il faut déclarer 

| export PYTHONPATH=<my-lima-installation>
| export LD_LIBRARY_PATH=<my-lima-installation>/Lima/lib

  ce qui renvoie la sortie suivante 

| tangosrv@tangoserver:~/SW_tango/Ressources/Lima/applications/tango$ cd /home/tangosrv/SW_tango/Ressources/Lima/applications/tango/ && python LimaCCDs.py cameraTest
| Warning: EnvHelper could not find LimaCameraType for server cameraTest
| Failed to import EventChannelFactory notifd/factory/tangoserver from the Tango database
| Warning optional plugin bpm_server.beam_viewer can't be load, dependency not satisfied.
| Warning optional plugin bpm_server.webserver can't be load, dependency not satisfied.
| Warning optional plugin bpm_server.xbpmds_webserver can't be load, dependency not satisfied.
| Warning optional plugin bpm_server.TangoBridge can't be load, dependency not satisfied.
| Warning optional plugin bpm_server.xbpmds can't be load, dependency not satisfied.
| For more pulgins dependency  information start server with -v4

