[Retour à la liste des modules](README.md)

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