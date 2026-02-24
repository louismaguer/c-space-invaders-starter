# Projet final C — Space Invaders (SDL2) — Louis Maguer

## Instructions de compilation

Pour compiler le code, saisir les commandes ci-dessous.
```shell
$ make clean
$ make
$ ./space_invaders
```

Il faut au préalable avoir installé SDL2 :
- macOS : `brew install sdl2`
- WSL : `sudo apt install libsdl2-dev`

Le jeu se lance alors.

## Règles spécifiques

Il existe plusieurs classes d'ennemis :
- ennemis classiques en cyan
- ennemis résistants en bleu foncé
- ennemis rapides en jaune
- ennemis tirant plus fréquemment en rouge

Les cœurs sont représentés en magenta.

On peut mettre le jeu en pause en pressant les touches ``Esc`` ou ``P``.

## Modification des paramètres

On peut modifier les différents paramètres de base en modifiant le fichier `game.h`. En particulier, la vitesse des ennemis est paramétrable à travers les champs : 
```
#define ENEMY_SPEED 10.0f
#define SPEED_INCREMENT 2.5f
#define TIME_BETWEEN_ACCELERATIONS 2.5f
```