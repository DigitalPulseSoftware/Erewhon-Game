# Minimal Viable Product

Le MVP d'Utopia se définit comme étant un simulateur de combats de vaisseaux spatiaux, autrement dit il s'agit d'une simplification reprenant l'un des coeurs du gameplay du jeu final.

## Description

Le MVP se compose d'un client et d'un serveur, ainsi que d'un launcher accompagnant le client capable de le mettre à jour et de l'exécuter.

Le jeu tourne autour du combat de flottes de vaisseaux spatiaux programmés. Le serveur met à disposition plusieurs arènes que les joueurs peuvent rejoindre pour faire s'affronter leurs flottes, programmées par leurs soins.

## Gameplay

### Combats

Une arène représente un champ de bataille physique, instancié avec quelques objets physiques neutres servant d'obstacles (astéroïdes, planètes, etc.), dans lequel les vaisseaux peuvent évoluer.
Chaque arène est configurée pour accueillir un certain nombre de personnes en combat (2 signifiant 1vs1, 3 signifiant 1vs1vs1, etc.).

Lorsqu'un joueur rejoint une arène, celui-ci est un spectateur en caméra libre capable uniquement de parler dans le chat.
Les joueurs peuvent utiliser une commande /fight <flotte> pour combattre avec la flotte de leurs choix, s'enregistrant dans la liste des combattants. Lorsque le nombre de combattants atteint le nombre demandé par l'arène, celle-ci démarre un combat dix secondes après utilisant N joueurs au hasard dans la liste des joueurs préparés. (Où N désigne le nombre prévu de combattants dans l'arène).

La flotte de chaque joueur apparaît ensuite dans un "coin" différent de l'arène (à une certaine distance, dépendant du nombre de participants), orientée vers le centre.
La caméra de chaque joueur est repositionnée derrière la flotte, toujours libre, mais dispose maintenant de la capacité de prendre le contrôle du vaisseau de sa flotte de son choix.

L'objectif du jeu est alors de détruire toutes les flottes des autres joueurs dans le temps imparti, après quoi l'arène se réinitialise entièrement et tous les joueurs participants redeviennent spectateur.

### Customisation des vaisseaux & programmation

Chaque vaisseau est composé de modules articulés autour d'une coque.

Une coque de vaisseau est ce qui lui donne la majeure partie de sa masse, son apparence, et 


L'utilisateur peut à l'aide du client :
- Paramétrer ses options
- S'enregistrer (faire un nouveau compte)
- Se connecter à un compte existant
- Créer, éditer et supprimer ses vaisseaux
- Créer, éditer et supprimer ses flottes
- Rejoindre/quitter une arène