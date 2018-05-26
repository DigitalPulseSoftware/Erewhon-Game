# Modules

Les modules sont l'interface entre un vaisseau et son environnement, sa seule façon d'agir et d'effectuer des actions.

Ils sont regroupés selon des catégories définies par leur type et fonctions, tous les modules d'un même type possèdent une base commune en terme d'API mais peuvent les étendre à leur guise.

Voici la liste des modules :

## Noyau (Core)

Le module central du vaisseau, celui-ci représente l'unité centrale d'exécution (le processeur) de votre vaisseau.

[Page dédiée au module core](Core.md)

## Armement (Weapon)

Module d'armement, permet de contrôler l'arme du vaisseau.

[Page dédiée au module d'armement](Weapon.md)

## Communications

Module de communication, permet d'envoyer et de recevoir des messages vers d'autres vaisseaux (alliés ou ennemis).

[Page dédiée au module de communications](Communications.md)

## Moteur (Engine)

Le module moteur contrôle directement les moteurs et les déplacements du vaisseau, néanmoins celui-ci donne un contrôle très bas-niveau sur le moteur, permettant uniquement de donner des impulsions d'une certaine durée.

[Page dédiée au module de communications](Engine.md)

## Navigation

Le module de navigation est un co-processeur dédié aux calculs de contrôle des propulseurs. Il vous permet de donner des ordres simples pour diriger votre vaisseau sans avoir à contrôler directement le moteur.

[Page dédiée au module de communications](Navigation.md)

## Radar

Le radar est le module permettant à votre vaisseau de connaître son entourage, il permet d'effectuer des scans donnant des informations brutes sur les divers objets à proximité d'un vaisseau ainsi que d'obtenir des informations plus précises à l'aide de scan ciblés.
En outre, certains radars disposent d'un scan passif, déclenchant des événements lorsqu'un objet rentre dans le champ de détection.

[Page dédiée au module de communications](Radar.md)
