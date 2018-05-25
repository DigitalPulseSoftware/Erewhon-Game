# Concept

Utopia (appelé Erewhon en interne) est un jeu de programmation orienté stratégie, massivement multijoueur (MMORTS).
Cela signifie que le jeu est orienté autour du principe de la programmation (au sens large) afin d'atteindre ses objectifs, et qu'il fonctionne dans un univers persistant dans lequel tous les joueurs peuvent se retrouver.

Le coeur du jeu se situe au niveau principalement de l'affrontement de vaisseaux spatiaux, programmés par les différents joueurs, et c'est actuellement le coeur du développement. Mais de nombreuses idées sont autour du jeu.

À terme, il est souhaité que le jeu offre une gestion d'empire spatial sur fond d'exploration.

# Gameplay

Les joueurs disposent d'outils pour customiser leurs vaisseaux et les assembler en une flotte hétérogène, cela signifie que le joueur est libre et encouragé à placer des vaisseaux de types différents capables d'accomplir des fonctions différentes.

Chaque vaisseau possède un ensemble de modules lui permettant de connaître et agir sur son environnement, les modules possèdent tous un type et un rôle particulier (ex: module radar, module d'arme, module moteur, etc.).
En plus de cela, un vaisseau possède un programme écrit en Lua (on parlera ici de script) définissant son comportement, sa façon de réagir aux divers événements se produisant dans son entourage.
Les scripts interagissent avec les différents modules présents, ceux-ci fournissant une API différente selon leur nature, un module de communication vous permettra d'émettre et de recevoir des messages, un module radar vous permettra de détecter et suivre les autres vaisseaux présents dans votre entourage, etc.

Actuellement, le coeur du jeu réside en l'affrontement entre deux (ou plus) flottes ennemies dans une arène fixe, les flottes faisant leur apparition en bordure de l'arène et ayant pour objectif de détruire la flotte ennemie.