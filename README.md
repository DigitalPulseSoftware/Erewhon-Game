# Utopia

Utopia est un projet de jeu vidéo réalisé en live sur [Twitch](https://www.twitch.tv/sirlynixvanfrietjes) par [Lynix](https://github.com/DrLynix) et qui a vocation à être complété en tant qu'initiative communautaire.
Utopia utilise et est à l'origine du moteur de jeu [Nazara Engine](https://github.com/DigitalPulseSoftware/NazaraEngine/).

## Installation des dépendances

### Compilation de Nazara Engine pour Utopia

Le projet utilise la dernière version de Nazara Engine. Afin de compiler le projet, vous aurez donc besoin de l'installer ou le mettre à jour pour votre installation.
Vous pouvez suivre le [wiki de NazaraEngine](https://github.com/DigitalPulseSoftware/NazaraEngine/wiki/(FR)-Compiler-le-moteur) pour le compiler selon votre plateforme et configuration.

### Compilation du projet

Le dépôt contient le serveur ainsi que le client. Vous pouvez tester le projet en lançant le client sur un serveur existant ou en lançant le serveur sur votre machine locale avant.
Pour compiler le projet, il vous faudra une version d'un compilateur supportant C++17 avec la libstdc++ à jour.
Il faudra également avoir Nazara Engine, soit en l'ayant installé via un package manager, soit en l'ayant compilé et packagé.

#### Compilation sous Linux

Dans le cas où vous avez besoin de packager Nazara Engine, il faudra aller dans ``NazaraEngine/build``, lancer la commande

```
# Dans NazaraEngine/build
./premake5-linux64 package
```

Une fois ceci fait, il faut indiquer aux scripts de compilation d'Utopia où se trouve Nazara Engine.

Utopia dispose d'un patron pour la configuration du build, que vous pouvez trouver dans ``build/config.lua.default`` et copier dans ``build/config.lua``.
Vous devez le modifier selon votre environnement, par exemple de la façon suivante :

```
NazaraPath = [[chemin/vers/NazaraEngine/package]]
```

ou bien, si vous avez installé Nazara dans votre système :

```
NazaraPath = [[/usr]]
```

## Contribuer au projet

Utopia est un projet ouvert à la communauté, vous pouvez contribuer au projet des façons suivantes:

+ compiler et tester le projet en rapportant les problèmes dans les [issues](https://github.com/DigitalPulseSoftware/Erewhon-Game/issues)
+ proposer ou discuter des idées sur la [plateforme communautaire d'utopia]()
+ corriger les typographies ou écrire de la documentation sur le projet
+ implémenter un prototype d'une des idées et le proposer dans le projet
+ corriger des bugs en suivant les [issues](https://github.com/DigitalPulseSoftware/Erewhon-Game/issues)
+ parler du projet à des gens, écrire sur le projet
+ faire des infographies pour le projet
+ faire des modèles ou textures pour le jeu
