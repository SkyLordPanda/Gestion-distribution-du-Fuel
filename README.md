# Projet Gestion de la distribution du Fuel

### **Objectifs :**
Une entreprise de livraison de fioul souhaite optimiser ses trajets pour la distribution et avoir une meilleure gestion et
suivi des camions. Pour cela, les camions sont équipés d'un boîtier traceur qui sera en communication avec un appareil
appelé "base" situé dans les locaux de l'entreprise.   

Rôle du boîtier traceur N°1 :

- *Acquérir les données de géolocalisation en temps réel (GPS).*
- *Permettre d’afficher les coordonnées de géolocalisation du véhicule, la vitesse en km/h au lieu des nœuds et l’heure courante qu’il pourra mettre à jour (IHM).*
- *Enregistrer les informations de parcours horodatées pour un suivi administratif en local (carte SD).*
- *Envoyer à la base les données de géolocalisation horodatées, la vitesse... sous forme sms (GSM).*

Partie embarquée N°1 :
![alt tag](https://user-images.githubusercontent.com/29919532/73275021-d3ce0800-41e6-11ea-87e2-85ac853d20bd.PNG)


Mon role dans ce projet etait donc de recevoir un SMS par réseau GSM  

### **Comment visionner mon programme ?**

***Pré-requis :***
- Proteus 8 (Pour les schéma)
- MikroC (Pour le code source)

**Pour le schéma**  
>Pour visionné le schéma il vous faut démarrer "GSM(bts).pdsprj" avec 
proteus 8

**Pour le code source**
>Pour regarder le code source, il faut démarrer "GSM_LPJJ.mcppi" avec mikroC

### **Liste des fichiers utiles**

- message_gsm_position.txt
> Montre une trame GSM à sa reception

- GSM_LPJJ.c
```
src/GSM_LPJJ.c
```
> Permet de visionner le code source sans le logiciel mikroC, Parcontre, le soucis c'est que les library ne seront pas incluses car elle sont propriétaire au logiciel mikroC

- AT_command.docx
> permet de ce faire une idée sur la communication entre le PIC18F45K22 (µController) et le module GSM

- GSM_Engine_doc.pdf
> La docummentation de mikroC sur les lib du module GSM (les commandes AT)

- l-gsmreseau.pdf & p-gsmmobile.pdf
> Cours qui ma beaucoup aider sur le fonctionnement d'un réseau GSM par Jean-Philippe Muller
