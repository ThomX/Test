Contexte :
----------

- tango v.7...
- archi: Linux 2.6.32-41-generic #90-Ubuntu SMP Tue May 22 11:31:25 UTC 2012 i686 GNU/Linux
- camera Basler Scout (non plugged)

Prérequis :
-----------

- python 2.6.5 installé
- gcc 4.4.3
- git 1.7.0.4

Actions :
---------

Source : p9-11 doc http://lima.blissgarden.org/latex/Lima.pdf

- Pylon:
|  get pylon-3.2.1-x68.tar.gz
  * décompresser dans /opt
| /opt/pylon/pylon-setup-env.sh /opt/pylon

  * remarque: dans le fichier INSTALL du tar.gz, line 74 : le saut de ligne après "export LD_LIBRARY_PATH=$..." devrait être supprimé
|  source /opt/bin/pylon-setup-env.sh
|  git clone --recursive git://github.com/esrf-bliss/Lima.git
|  cd Lima
|  make (ne fonctionne pas avant l'édition de config.inc)

  * éditer config.inc et écrire 

| COMPILE_CORE=1
| COMPILE_SIMULATOR=0
| COMPILE_SPS_IMAGE=1
| COMPILE_ESPIA=0
| COMPILE_FRELON=0
| COMPILE_MAXIPIX=0
| COMPILE_PILATUS=0
| COMPILE_BASLER=1
| COMPILE_CBF_SAVING=0
| export COMPILE_CORE COMPILE_SPS_IMAGE COMPILE_SIMULATOR \
|        COMPILE_ESPIA COMPILE_FRELON COMPILE_MAXIPIX COMPILE_PILATUS \
|        COMPILE_BASLER COMPILE_CBF_SAVING"

  * Doit-on laisser COMPILE_CONFIG=1 dans config.inc **=> Non, ce n'est pas obligatoire mais très utile donc c'est par défaut. Voire les  attributs/propriétés/commandes commençant par "config" (http://lima.blissgarden.org/applications/tango/doc/index.html#main-device-limaccds)**

| make config

  * Quand on exécute "make config" dans le répertoire Lima : "usr/bin/sip: not found" -> installer le paquet SIP4 (ne semble pas être indiqué dans la doc) **=> sera ajouté dans1 la doc**

| make install

  * Quand on exécute "make" dans le répertoire Lima : "../core/src/GslErrorMgr.cpp:24:27: error: gsl/gsl_errno.h: Aucun fichier ou dossier de ce type" -> install libgsl0-dev (ne semble pas être indiqué dans la doc)

  * Quand on exécute  "make" dans le répertoire Lima : un grand hombre d'alertes sans lien (un switch gère des cas avec un return après être sorti du switch et des alertes #pragma sur des définitions qui sont obsolètes)
make -C sip -j3

| export INSTALL_DIR=~/SW_tango/
| make config

- Après, p11, se déplacer dans Lima/application/tango/.
