# Modules

Les modules sont l'interface entre un vaisseau et son environnement, sa seule façon d'agir et d'effectuer des actions.

Ils sont regroupés selon des catégories définies par leur type et fonctions, tous les modules d'un même type possèdent une base commune en terme d'API mais peuvent les étendre à leur guise.

Voici la liste des modules :

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

## Armement (Weapon)

Module d'armement, permet de contrôler l'arme du vaisseau.

Actuellement un vaisseau ne peut avoir qu'un seul module d'armement, cela va changer dans le futur et ce module pourrait contrôler toutes les armes du vaisseau.

### API

```lua
Core:Shoot()
```

Tire un projectile devant le vaisseau.

## Communications

Module de communication, permet d'envoyer et de recevoir des messages vers d'autres vaisseaux (alliés ou ennemis).

Les messages que vous envoyés peuvent et seront reçus également par des vaisseaux à qui ils ne seront pas destinés, attention à 

Actuellement, les messages sont de simples chaînes de caractères, à terme elles vont sans doute devenir un objet à part entière et intégrer des concepts comme l'émetteur, le chiffrement, etc.

### API

```lua
Communications:BroadcastCone(Vec3 direction, number distance, string message)
```

Envoie un message dans une direction précise de façon suffisamment concentré pour être reçu `distance` mètres plus loin.

```lua
Communications:BroadcastSphere(number distance, string message)
```

Envoie un message à tous les vaisseaux pour qu'il soit reçu jusqu'à une distance approximative de `distance` mètres.

### Événements

```lua
Spaceship:OnCommunicationReceivedMessages(table messages)
```

Est appelé régulièrement (la fréquence dépend du module de communications) avec les messages reçus depuis le dernier appel.

La structure des messages est la suivante :
```lua
[n] = { -- ID du message reçu, part toujours de 1 et va jusqu'à n, n étant le nombre de messages reçus
	["data"] = Message reçu
	["direction"] = Direction calculée vers l'émetteur du message
	["distance"] = Distance approximative vers l'émetteur du message
},
[n+1] = {
	...
}
```

## Moteur (Engine)

Le module moteur contrôle directement les moteurs et les déplacements du vaisseau, néanmoins celui-ci donne un contrôle très bas-niveau sur le moteur, permettant uniquement de donner des impulsions d'une certaine durée.

### API

```lua
Engine:Impulse(Vec3 impulse, number duration)
```

Applique une impulsion `impulse` d'une durée de `duration` secondes.
L'impulsion est normalisée indépendamment sur chaque axe et représente le pourcentage de puissance moteur à appliquer sur cet axe.
Exemple : Vec3(0.5, -1.0, 1.0) donne une impulsion à 50% sur l'axe X, à -100% (moteur arrière) sur l'axe Y et à 100% sur l'axe Z.

Attention que selon la direction, le moteur peut ne pas fournir la même poussée (typiquement les vaisseaux fournissent plus de poussée en se déplaçant devant eux).
Toutes les impulsions sont dans l'espace local par-rapport au vaisseau.

Si cette fonction est appelée plusieurs fois, les impulsions sont cumulées mais leurs durées restent indépendantes.

## Navigation

WIP

## Radar

WIP