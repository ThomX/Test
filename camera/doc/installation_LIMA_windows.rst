Contexte :
----------

- Windows 7 32bits

Prérequis :
-----------

- Python 2.7.2 installé
- GnuWin32: gsl-1.8
- Microsoft visual C++ 2008 Express

Installation :
--------------

Source : p3-9 doc http://lima.blissgarden.org/latex/Lima.pdf

clone de lima à l'adresse https://github.com/esrf-bliss/Lima

Modif du fichier config.inc pour une caméra Basler
COMPILE_CORE=1
COMPILE_SIMULATOR=0
COMPILE_SPS_IMAGE=1
COMPILE_ESPIA=0
COMPILE_FRELON=0
COMPILE_MAXIPIX=0
COMPILE_PILATUS=0
COMPILE_BASLER=1
COMPILE_PROSILICA=0
COMPILE_ROPERSCIENTIFIC=0
COMPILE_MYTHEN=0
COMPILE_ADSC=0
COMPILE_UEYE=0
COMPILE_XH=0
COMPILE_XSPRESS3=0
COMPILE_ULTRA=0
COMPILE_XPAD=0
COMPILE_PERKINELMER=0
COMPILE_ANDOR=0
COMPILE_ANDOR3=0
COMPILE_PHOTONICSCIENCE=0
COMPILE_PCO=0
COMPILE_MARCCD=0
COMPILE_POINTGREY=0
COMPILE_IMXPAD=0
COMPILE_DEXELA=0
COMPILE_RAYONIXHS=0
COMPILE_AVIEX=0
COMPILE_CBF_SAVING=0
COMPILE_NXS_SAVING=0
COMPILE_FITS_SAVING=0
COMPILE_EDFGZ_SAVING=0
COMPILE_TIFF_SAVING=0
COMPILE_HDF5_SAVING=0
COMPILE_CONFIG=1
COMPILE_GLDISPLAY=0
LINK_STRICT_VERSION=0
export COMPILE_CORE COMPILE_SPS_IMAGE COMPILE_SIMULATOR \
       COMPILE_ESPIA COMPILE_FRELON COMPILE_MAXIPIX COMPILE_PILATUS \
       COMPILE_BASLER COMPILE_PROSILICA COMPILE_ROPERSCIENTIFIC COMPILE_ADSC \
       COMPILE_MYTHEN COMPILE_UEYE COMPILE_XH COMPILE_XSPRESS3 COMPILE_ULTRA COMPILE_XPAD COMPILE_PERKINELMER \
       COMPILE_ANDOR COMPILE_ANDOR3 COMPILE_PHOTONICSCIENCE COMPILE_PCO COMPILE_MARCCD COMPILE_DEXELA\
       COMPILE_POINTGREY COMPILE_IMXPAD COMPILE_RAYONIXHS COMPILE_AVIEX COMPILE_CBF_SAVING COMPILE_NXS_SAVING \
       COMPILE_FITS_SAVING COMPILE_EDFGZ_SAVING COMPILE_TIFF_SAVING COMPILE_HDF5_SAVING COMPILE_CONFIG\
       COMPILE_GLDISPLAY \
       LINK_STRICT_VERSION


Erreurs rencontrées :
---------------------

Lors de l'exécution de install.bat, une erreur se produit :

Dans le fichier install.bat, le script de compilation de la caméra Basler est le suivant :

rem compilation des plugins des cameras actives dans le fichier config.inc
for /f "delims=:" %%i in ('type %CurrentPath%\config.inc') do (
	set ligne=%%i
	set ligne_temp=%%i
	if "!ligne:~0,7!" == "COMPILE" (
		rem call:strlen longueur !ligne! 
		rem echo longueur : !longueur!
		if "!ligne:~-1!" == "1" (
			if not "!ligne:~8,-2!" == "CORE" (
				rem compilation du plugin active
				cd /D !CurrentPath!\camera\!ligne:~8,-2!\build\msvc\9.0\lib!ligne:~8,-2!
				msbuild.exe Lib!ligne:~8,-2!.sln /t:build /fl /flp:logfile=MyProjectOutput.log /p:Configuration=Release;Plateform=Win32 /v:d
			)
		)
	)
)

La raison semble être que dans !CurrentPath!\camera\!ligne:~8,-2!\build\msvc\9.0\libBasler, la bibliothèque ne s'appelle pas  "LibBasler.sln" mais "LibLimaBasler.sln"
Il faudrait donc soit renommer la solution en "LibBasler.sln", soit modifier install.bat et mettre "msbuild.exe LibLima!ligne:~8,-2!.sln"

Nous avons choisi la seconde solution (modification de install.bat). Les bibliothèque Basler sont copiées dans "C:\Python27\Lib\site-packages\Lima" (voir la sortie dans le fichier sortie_compilation_limaCCDs_windows.txt)
Cela permet visiblement de terminer la compilation.
