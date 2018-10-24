[Retour à la liste des modules](README.md)

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