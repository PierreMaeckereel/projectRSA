# projectRSA : MyAdBlock

Ce projet est un proxy http bloqueur de publicité codé en langage C

## lancement en ligne de commande

Pour le lancer :
* le compiler au préalable avec : gcc -c -Wall myAdBlock myadsblocker.c
* ouvrir un terminal et run 'myAdBlock' avec un numéro de port en argument, par exemple 8080
* ensuite configurer votre navigateur web pour qu'il accepte ce proxy :
* adresse ip : 127.0.0.1
* port : le numéro spécifié en argument

Ensuite, ouvrir une nouvelle page web dans ce navigateur : la requête passe automatiquement par le proxy

## lancement avec le makefile

Pour lancer le programme à l'aide du makefile, lancer un terminal et :
* le compiler avec make
* le lancer avec make run. Un numéro de port par défaut est utilisé : le 8080
* supprimer les fichiers générés avec make clean ou make rmproper
