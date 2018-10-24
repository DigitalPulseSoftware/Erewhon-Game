[Retour à la liste des modules](README.md)

## Noyau (Core)

Le module central du vaisseau, celui-ci représente l'unité centrale d'exécution (le processeur) de votre vaisseau.

Actuellement, il vous permet de récupérer des informations sur votre propre vaisseau, mais ceci est susceptible d'être déplacé vers un autre module plus spécifique.

### API:

```lua
Vec3 Core:GetAngularVelocity()
```

Renvoie la vélocité angulaire (de rotation) du vaisseau au moment de l'appel.

```lua
number Core:GetIntegrity()
```

Renvoie l'intégrité physique du vaisseau (ses points de vie) au moment de l'appel, actuellement `100` représente le maximum et `0` le minimum (la mort).

```lua
Vec3 Core:GetLinearVelocity()
```

Renvoie la vélocité linéaire (de déplacement) du vaisseau au moment de l'appel.

```lua
Vec3 Core:GetPosition()
```

Renvoie la position absolue du vaisseau au moment de l'appel.

```lua
Quaternion Core:GetRotation()
```

Renvoie la rotation absolue du vaisseau au moment de l'appel.

```lua
integer Core:GetSignature()
```

Renvoie la signature radar du vaisseau.

### Évenements

```lua
Spaceship:OnStart()
```

Appelé une seule fois, une fois le vaisseau prêt à combattre dans l'arène.
Il s'agit du premier événement déclenché dans l'exécution du script et peut vous permettre d'initialiser des variables.

```lua
Spaceship:OnTick(number elapsedTime)
```

Appelé régulièrement, à une fréquence dépendant du module utilisé.

Le paramètre `elapsedTime` désigne le nombre de secondes écoulées depuis le dernier appel à `OnTick` (ou `OnStart` s'il s'agit du premier appel).