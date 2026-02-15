# Capteur Matter Température/Humidité

Capteur de température et humidité connecté via **Matter** (compatible SmartThings, Google Home, Apple Home, Alexa).

Affichage sur écran e-paper 2.13" avec graphique d'historique et tendances.

---

## 🛠️ Liste du matériel

### Composants requis

| Composant | Modèle | Prix | Où acheter |
|-----------|--------|------|------------|
| **Microcontrôleur** | ESP32-C3 SuperMini | ~3-5€ | [AliExpress](https://www.aliexpress.com/wholesale?SearchText=esp32-c3+supermini) · [Amazon FR](https://www.amazon.fr/s?k=esp32-c3) |
| **Capteur T°/Humidité** | DHT22 (AM2302) | ~3-5€ | [AliExpress](https://www.aliexpress.com/wholesale?SearchText=dht22) · [Amazon FR](https://www.amazon.fr/s?k=dht22+am2302) |
| **Écran e-paper** | Waveshare 2.13" (GDEY0213B74) | ~15-20€ | [Waveshare Official](https://www.waveshare.com/2.13inch-e-paper-hat.htm) · [AliExpress](https://www.aliexpress.com/wholesale?SearchText=waveshare+2.13+e-paper) |
| **Câbles** | Jumper wires F-F (x10) | ~2€ | [AliExpress](https://www.aliexpress.com/wholesale?SearchText=jumper+wires+female) |
| **Alimentation** | Câble USB-C | ~2€ | N'importe quel câble USB-C |

**Budget total :** 25-35€

### Optionnel

| Composant | Usage | Prix |
|-----------|-------|------|
| Résistance 10kΩ | Pull-up DHT22 si version brute | ~0.10€ |
| Boîtier imprimé 3D | Protection | DIY ou ~5€ |

---

## 🔌 Schéma de câblage

### Vue d'ensemble

```
                    ESP32-C3 SuperMini
                    ┌─────────────────┐
DHT22 VCC    ──────→│ 3V3         GND │←────── DHT22 GND
DHT22 DATA   ──────→│ GPIO4           │
                    │                 │
E-Paper VCC  ──────→│ 3V3         GND │←────── E-Paper GND
E-Paper BUSY ──────→│ GPIO5           │
E-Paper RST  ──────→│ GPIO6           │
E-Paper DC   ──────→│ GPIO7           │
E-Paper CS   ──────→│ GPIO10      USB │←────── USB-C 5V
E-Paper CLK  ──────→│ GPIO2           │
E-Paper DIN  ──────→│ GPIO3           │
                    │                 │
                    │ [BOOT]  [RESET] │
                    └─────────────────┘
```

### Tableau de connexions

#### ESP32-C3 ↔ DHT22

| DHT22 Pin | Couleur | ESP32-C3 Pin | Description |
|-----------|---------|--------------|-------------|
| VCC | Rouge | 3V3 | Alimentation 3.3V |
| GND | Noir | GND | Masse |
| DATA | Jaune | GPIO4 | Signal données |

#### ESP32-C3 ↔ E-Paper 2.13"

| E-Paper Pin | ESP32-C3 Pin | Fonction | Note |
|-------------|--------------|----------|------|
| VCC | 3V3 | Alimentation | ⚠️ **3.3V SEULEMENT** |
| GND | GND | Masse | |
| DIN | GPIO3 | MOSI (données SPI) | |
| CLK | GPIO2 | SCK (horloge SPI) | |
| CS | GPIO10 | Chip Select | |
| DC | GPIO7 | Data/Command | |
| RST | GPIO6 | Reset | |
| BUSY | GPIO5 | Signal occupation | |

---

## 🔧 Guide de montage pas à pas

### Étape 1 : Préparation

**Ce dont tu as besoin :**
- Tous les composants ci-dessus
- Un ordinateur avec Arduino IDE
- 30 minutes de temps

**Outils optionnels :**
- Multimètre (pour vérifier continuité)
- Fer à souder (si connexions permanentes)

### Étape 2 : Identification des pins

**ESP32-C3 SuperMini :**

Les GPIO sont marqués sur le PCB. Repère :
- `3V3` et `GND` (alimentation)
- `GPIO2`, `GPIO3`, `GPIO4`, `GPIO5`, `GPIO6`, `GPIO7`, `GPIO10`
- Boutons `BOOT` et `RESET`

**DHT22 Module (3 pins) :**
```
Vue de dessus
┌───────┐
│  DHT  │
│ ┌───┐ │
│ └───┘ │
└───────┘
 │ │ │
 V G D
 C N A
 C D T
 
VCC = Alimentation (+)
GND = Masse (-)
DATA = Signal
```

**E-Paper 2.13" :**

Connecteur avec 8 pins généralement étiquetées.

### Étape 3 : Câblage du DHT22

**Si tu as un module DHT22 (carte bleue/rouge) :**

1. **VCC** (rouge) → ESP32 **3V3**
2. **GND** (noir) → ESP32 **GND**
3. **DATA** (jaune) → ESP32 **GPIO4**

**Si tu as un DHT22 brut (4 pins) :**

```
DHT22 vu de face (grille devant)
┌─────────────┐
│ ┌─────────┐ │
│ │ ▓▓▓▓▓▓▓ │ │  ← Grille
│ └─────────┘ │
└─────────────┘
  1   2   3   4

Pin 1: VCC  → ESP32 3V3
Pin 2: DATA → ESP32 GPIO4 + résistance 10kΩ vers Pin 1
Pin 3: NC (non connecté)
Pin 4: GND  → ESP32 GND
```

### Étape 4 : Câblage de l'écran e-paper

**⚠️ ATTENTION CRITIQUE : L'écran e-paper fonctionne UNIQUEMENT en 3.3V !**

Brancher du 5V **détruira** l'écran. Vérifie deux fois.

**Ordre de câblage recommandé (pour la sécurité) :**

1. **D'abord la masse :**
   ```
   E-Paper GND → ESP32 GND
   ```

2. **Puis l'alimentation (vérifie bien 3V3 !) :**
   ```
   E-Paper VCC → ESP32 3V3 (PAS 5V !)
   ```

3. **Signaux SPI :**
   ```
   E-Paper DIN  → ESP32 GPIO3
   E-Paper CLK  → ESP32 GPIO2
   E-Paper CS   → ESP32 GPIO10
   ```

4. **Signaux de contrôle :**
   ```
   E-Paper DC   → ESP32 GPIO7
   E-Paper RST  → ESP32 GPIO6
   E-Paper BUSY → ESP32 GPIO5
   ```

**Astuce :** Utilise des câbles de couleurs différentes :
- Rouge = VCC
- Noir = GND
- Autres couleurs = signaux

### Étape 5 : Vérification avant mise sous tension

**Checklist de sécurité :**

- [ ] **E-Paper VCC connecté à 3V3** (PAS 5V !)
- [ ] Aucun court-circuit VCC/GND (teste avec multimètre si possible)
- [ ] Tous les câbles bien enfoncés
- [ ] Les bonnes GPIO sont utilisées
- [ ] Aucun fil ne touche d'autres pins

**Si multimètre disponible :**
- Résistance entre VCC et GND > 1kΩ (pas de court-circuit)

### Étape 6 : Premier test matériel

**Ne branche PAS encore l'ESP32 !**

D'abord, vérifie le câblage avec ce test :

1. **Prends une photo** de ton montage
2. **Compare** avec le schéma ci-dessus
3. **Vérifie chaque connexion** une par une

**Maintenant, branche l'ESP32 en USB :**

1. Connecte le câble USB-C
2. **Vérifie que la LED power s'allume** sur l'ESP32
3. Si l'ESP32 chauffe anormalement → **DÉBRANCHE IMMÉDIATEMENT**

**Téléverse ce code de test :**

```cpp
void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n=== TEST MATERIEL ===");
  Serial.println("[OK] ESP32 demarre");
  
  // Test pins
  pinMode(4, INPUT);   // DHT22
  pinMode(2, OUTPUT);  // E-Paper CLK
  pinMode(3, OUTPUT);  // E-Paper DIN
  pinMode(5, INPUT);   // E-Paper BUSY
  pinMode(6, OUTPUT);  // E-Paper RST
  pinMode(7, OUTPUT);  // E-Paper DC
  pinMode(10, OUTPUT); // E-Paper CS
  
  Serial.println("[OK] Pins configurees");
  Serial.println("\n=== MONTAGE OK ===");
}

void loop() {
  delay(1000);
}
```

**Résultat attendu :** Dans le moniteur série (115200 baud) :
```
=== TEST MATERIEL ===
[OK] ESP32 demarre
[OK] Pins configurees

=== MONTAGE OK ===
```

---

## 💻 Installation du logiciel

### Arduino IDE

1. **Télécharge Arduino IDE 2.x :** https://www.arduino.cc/en/software
2. **Installe et lance** Arduino IDE

### Configuration ESP32

1. **Fichier → Préférences**
2. **URLs de gestionnaire de cartes**, ajoute :
   ```
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
3. **Outils → Type de carte → Gestionnaire de cartes**
4. Cherche **"esp32"**
5. Installe **"esp32 by Espressif Systems" version 3.3.5 (pas testé sur supérieur)**

### Bibliothèques

**Outils → Gérer les bibliothèques**, installe :

- `GxEPD2` by Jean-Marc Zingg
- `DHT sensor library` by Adafruit  
- `Adafruit Unified Sensor` by Adafruit

### Configuration de la carte

**Outils :**

| Paramètre | Valeur |
|-----------|--------|
| Type de carte | ESP32C3 Dev Module |
| Partition Scheme | Huge APP (3MB No OTA/1MB SPIFFS) |
| USB CDC On Boot | Enabled |
| Port | /dev/cu.usbmodem* (Mac) ou COM* (Windows) |

---

## 🎯 Caractéristiques

- ✅ **Mise à jour toutes les 2 minutes**
- ✅ **Toujours en ligne** dans SmartThings
- ✅ **Graphique 48 minutes** avec historique 24 points
- ✅ **Tendances** (↑↓) température et humidité
- ✅ **Icônes intelligentes** (thermomètre + goutte)
- ✅ **Indicateur WiFi** en temps réel

---

## 📊 Fonctionnement

### Cycle automatique (2 minutes)

```
Démarrage
  ↓
Connexion WiFi/Matter
  ↓
┌──────────────────────┐
│ Mesure temp + humidité│
│ Calcul tendances     │
│ Mise à jour écran    │
│ Sync SmartThings     │
└──────────────────────┘
  ↓
Attente 2 minutes
  ↓
Retour ↑
```

---

## 🎨 Interface

```
┌──────────────────────┐
│              📶      │  WiFi
│       23.3°          │  Température
│       42.3%          │  Humidité
│    🌡️ ↑    💧 ↓    │  Tendances
│   ─────────────────  │
│   Graphique 48 min   │
└──────────────────────┘
```

---

## 🚀 Premier démarrage

### 1. Téléversement

1. **Ouvre** `matter_always_on.ino`
2. **Vérifie** la config (Outils → voir ci-dessus)
3. **Upload** (→)
4. **Moniteur série** (115200 baud)

### 2. Logs attendus

```
=== CAPTEUR MATTER ===
[OK] DHT22
[OK] Ecran e-paper
[OK] Sensors initialises
[OK] Matter demarre

========================================
  APPAREIL NON COMMISSIONNE
========================================
QR Payload: MT:Y.K9042C00KA0648G00
========================================

>>> SCANNE LE QR CODE <<<
```

L'écran e-paper affiche le QR code.

### 3. Ajout à SmartThings

1. **App SmartThings** → **+** (Ajouter)
2. **Appareil** → **Scanner le code QR**
3. **Scanne le QR** sur l'écran e-paper
4. **Suis les instructions**

**Note :** Smartphone et hub SmartThings sur **même WiFi 2.4GHz**.

### 4. Fonctionnement

```
[OK] Connecte!
Capteur operationnel!
T: 23.3C, H: 42.3%
Prochaine mise a jour dans 2 min
```

---

## ⚙️ Personnalisation

### Intervalle de mise à jour

Ligne ~60 :
```cpp
const unsigned long UPDATE_INTERVAL = 2 * 60 * 1000;  // 2 min
```

### Taille historique

Ligne ~48 :
```cpp
#define HISTORY_SIZE 24  // 24 points = 48 min
```

Pour 24h avec update 2 min :
```cpp
#define HISTORY_SIZE 720  // 720 * 2 min = 24h
```

### Seuils tendances

Ligne ~195 :
```cpp
if (tempDiff > 0.5) tempTrend = 1;   // Hausse > +0.5°C
if (humDiff > 2.0) humTrend = 1;     // Hausse > +2%
```

---

## 🔧 Decommissioning

**Retirer de SmartThings :**

Maintiens **BOOT** enfoncé **5 secondes**.

L'appareil redémarre et affiche le QR code.

---

## 🐛 Troubleshooting

### DHT22 ne fonctionne pas

- ✓ Vérifie connexions VCC/GND/DATA
- ✓ Attends 2s après `dht.begin()`
- ✓ Ajoute résistance 10kΩ si DHT22 brut

### Écran reste blanc

- ✓ Vérifie alimentation **3.3V** (PAS 5V !)
- ✓ Vérifie toutes connexions SPI
- ✓ Logs : `[OK] Ecran` doit apparaître

### Commissioning échoue

- ✓ WiFi en **2.4GHz** (pas 5GHz)
- ✓ Hub SmartThings sur même réseau
- ✓ QR code bien affiché
- ✓ Logs : `Matter demarre` doit apparaître

### Tendances absentes

Normal au 1er cycle. Attendre 2 min.

### Graphique vide

Normal au démarrage. Se remplit progressivement.

---

## 🔗 Ressources

### Datasheets

- [ESP32-C3](https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_en.pdf)
- [DHT22](https://www.sparkfun.com/datasheets/Sensors/Temperature/DHT22.pdf)
- [E-Paper 2.13"](https://www.waveshare.com/wiki/2.13inch_e-Paper_HAT)

### Documentation

- [Matter Spec](https://csa-iot.org/all-solutions/matter/)
- [GxEPD2 Library](https://github.com/ZinggJM/GxEPD2)
- [Adafruit DHT](https://github.com/adafruit/DHT-sensor-library)
- [Arduino-ESP32](https://docs.espressif.com/projects/arduino-esp32/)

### Guides connexes

- [ESP32 Pinout](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
- [DHT22 Tutorial](https://learn.adafruit.com/dht)
- [E-Paper Guide](https://www.waveshare.com/wiki/2.13inch_e-Paper_HAT)

---

## ⚡ Sécurité

### Électricité

- ⚠️ **Écran e-paper = 3.3V UNIQUEMENT** (5V = destruction)
- Débranche avant modification câblage
- Vérifie connexions avant alimenter

### Surchauffe

- ESP32 peut chauffer (~50°C = normal)
- Ne bloque pas ventilation
- Si chauffe excessif → débranche

### Humidité

- DHT22 supporte 99% humidité
- Ne l'immerge jamais
- Évite condensation directe

---


## 🎉 C'est parti !

1. ✅ Achète les composants
2. ✅ Monte le matériel
3. ✅ Téléverse le code
4. ✅ Scanne le QR
5. ✅ Profite ! 🚀
