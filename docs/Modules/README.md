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
[n] = { -- ID du message reçu, commence à 1 et va jusqu'au nombre de messages reçus
	["data"] = string, -- Message reçu
	["direction"] = Vec3, -- Direction calculée vers l'émetteur du message
	["distance"] = number, -- Distance approximative vers l'émetteur du message
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

Le module de navigation est un co-processeur dédié aux calculs de contrôle des propulseurs. Il vous permet de donner des ordres simples pour diriger votre vaisseau sans avoir à contrôler directement le moteur.
Ce module fonctionne en utilisant les modules radar et moteurs présents sur le vaisseau.

### API

```lua
Navigation:FollowTarget(integer targetSignature, number triggerDistance = nil)
```

Place le vaisseau en mode poursuite vers une cible en particulier (celle-ci doit être dans le champs de détection du radar), orientant le nez de l'appareil vers la cible au cours de la poursuite.

Si `triggerDistance` est fourni, il s'agit de la distance (vis-à-vis de la cible) en dessous de laquelle l'événement `OnNavigationDestinationReached` sera déclenché.

Les appels à FollowTarget et MoveToPosition ne sont pas cumulables, seul le dernier appel fait effet.

```lua
Navigation:MoveToPosition(Vec3 targetPosition, number triggerDistance = nil)
```

Oriente et dirige le vaisseau vers un point fixe dans l'espace.

Si `triggerDistance` est fourni, il s'agit de la distance (vis-à-vis de la cible) en dessous de laquelle l'événement `OnNavigationDestinationReached` sera déclenché.

Les appels à FollowTarget et MoveToPosition ne sont pas cumulables, seul le dernier appel fait effet.

```lua
Navigation:Stop()
```

Arrête immédiatement la poursuite/le déplacement précédemment initié.
Note : Ceci ne stoppe pas le vaisseau mais désactive uniquement la navigation, ainsi votre contrôle manuel sur le moteur n'est pas impacté.

N'a aucun effet si le module était déjà inactif au moment de l'appel.

### Événements

```lua
Spaceship:OnNavigationDestinationReached()
```

Est appelé lorsque le vaisseau se trouve à une distance inférieure à la `triggerDistance` (si spécifiée) d'un précédent appel à `FollowTarget` ou `MoveToPosition`.

## Radar

Le radar est le module permettant à votre vaisseau de connaître son entourage, il permet d'effectuer des scans donnant des informations brutes sur les divers objets à proximité d'un vaisseau ainsi que d'obtenir des informations plus précises à l'aide de scan ciblés.
En outre, certains radars disposent d'un scan passif, déclenchant des événements lorsqu'un objet rentre dans le champ de détection.

Ce module s'occupe de récupérer diverses informations et calculer une signature radar unique pour chaque cible, permettant de les désigner indépendamment les unes des autres.

### API

```lua
Radar:EnablePassiveScan(boolean enable)
```

Active/désactive le scan passif du radar, permettant de déclencher régulièrement (à un rythme dépendant du module) des scans des environs pour détecter de nouvelles entrées dans le champ de détection.

À terme, le scan passif possèdera un coût énergétique.

```lua
table Radar:GetTargetInfo(integer signature)
```

Récupère des informations précises à propos d'une cible en particulier faisant partie du champ de détection du radar.
Si la cible désignée par la `signature` ne fait pas partie du champ de détection du radar, `nil` est retourné.

Les informations sont retournées sous la forme suivante :
```lua
{
	["angularVelocity"] = Vec3, -- Vélocité angulaire de la cible
	["direction"] = Vec3, -- Vecteur normalisé relatif entre le radar et la cible
	["distance"] = number, -- Distance approximative vers la cible
	["emSignature"] = number, -- Signature électromagnétique de la cible
	["linearVelocity"] = Vec3, -- Vélocité linéaire de la cible
	["rotation"] = Quaternion, -- Rotation de la cible
	["signature"] = integer, -- Signature radar de la cible
	["size"] = number, -- Rayon de la cible
	["volume"] = number, -- Volume de la cible
}
```

```lua
boolean Radar:IsPassiveScanEnabled()
```

Renvoie `true` si le scan passif est activé, `false` sinon. (Par défaut, le scan passif est activé).

```lua
table Radar:Scan()
```

Effectue un scan de l'entourage du radar, récupérant des informations brutes sur toutes les objets détectés.

Les informations sont retournées sous la forme suivante :
```lua
[n] = { -- ID de l'objet, commençant à 1 et allant jusqu'au nombre d'obets repérées par le radar
	["direction"] = Vec3, -- Vecteur normalisé relatif entre le radar et l'objet
	["distance"] = number, -- Distance approximative vers l'objet
	["emSignature"] = number, -- Signature électromagnétique de l'objet
	["signature"] = integer, -- Signature radar de l'objet
	["size"] = number, -- Rayon approximatif de l'objet
},
[n+1] = {
	...
}
```

### Événements

```lua
Spaceship:OnRadarNewObjectInRange(integer signature, number emSignature, number radius, Vec3 direction, number distance)
```

Est déclenché lorsque le scan passif (si activé) détecte un nouvel objet dans les environs du vaisseau.

Les paramètres sont :

- `signature` : La signature radar du nouvel objet.
- `emSignature` : La signature électromagnétique de l'objet.
- `radius` : Rayon approximatif de l'objet (en mètres).
- `direction` : Vecteur normalisé relatif entre le radar et l'objet.
- `distance` : Distance approximative, en mètres, entre le radar et le nouvel objet détecté.