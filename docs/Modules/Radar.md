[Retour à la liste des modules](README.md)

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