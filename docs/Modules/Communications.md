[Retour à la liste des modules](README.md)

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