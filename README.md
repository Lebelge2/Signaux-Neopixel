# Signaux-Neopixel

Les rubans LED NeoPixel sont des diodes lumineuses intelligentes et adressables individuellement. 
Contrairement aux LED classiques, vous pouvez contrôler la couleur et la luminosité de chaque LED d'un ruban avec un seul fil de données provenant d'un microcontrôleur.

Bonjour.

Depuis peu je m'intéresse à ce type de signalisation lumineuse.
Après la construction d'un TCO complet avec des rubans LED 5mm espacés de 7mm (140/m).      https://forum.locoduino.org/index.php?topic=2080.0

Je tente la même approche avec des rubans LED CMS 2020 espacés de 4,2mm  (240/m) pour réaliser des signaux simples et complexes.

En principe, un fil connecté au microcontrôleur parcourt tous les signaux en série. Ce qui n'est guère pratique.(Coupures, mauvais contacts)
L'alternative est un câblage "étoile série", où plusieurs rubans de signaux en série seraient connectés au microcontrôleur.

Dans mon projet avec un ESP32, on peut y connecter 14 rubans de 8 signaux et plus.
Tous les types de signaux fonctionnent: 2,3,4,5,6,7,8,9 feux fixes et clignotants, disques et indicateurs de direction 3 feux, œilleton.

La programmation est extrêmement simple, dans un tableau, renseigner le n° de signal et son type d'écran (A,B,C,R,E,F,FID,H,HID,I,RV)
Première ligne du tableau, les signaux successifs sur le ruban 1.
Deuxième ligne du tableau, les signaux successifs sur le ruban 2.
Troisième ligne les signaux sur le ruban 3, etc...

Terminé la programmation complexe des CV...
L'ESP32 remplace plusieurs dizaines de décodeurs d'accessoires...

Ensuite contrairement à une programmation classique de signaux, c'est l'ESP qui calcule les numéros et nombres de décodeurs virtuels nécessaires au fonctionnement des signaux.
(Signal 4 feux, un décodeur. Si plus, deux décodeurs et un troisième si indicateur de directions).
La configuration s'affiche sur le moniteur série, faire un copier/coller/imprimer.

Construction des signaux:
Ces rubans de LED sont très minces, 3mm de largeur. 1,3mm d'épaisseur et les composants type CMS 0804.
Soudures délicates et difficiles.
Je cherche une méthode accessible de fabrication.
Je penche pour un support PCB prédécoupé et précâblé, et y coller les rubans LED. 

Pour la démo, 4 signaux cablés en série sur une sortie de l'ESP 
(Signaux prototypes fait de plastique et carton.)

Câblage:
4 fils: Alimentation 5V, Data série IN OUT.

Petite video:  https://youtu.be/GUk0NhaDMao
