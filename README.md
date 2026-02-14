# Capteur Matter - Alimentation permanente

Cette version est conçue pour fonctionner **branché en permanence** (USB ou alimentation 5V).

---

## 🎯 Caractéristiques

- ✅ **Mise à jour toutes les 2 minutes**
- ✅ **Toujours en ligne** dans SmartThings
- ✅ **Graphique 48 minutes** avec historique 24 points
- ✅ **Tendances** (↑↓) température et humidité
- ✅ **Icônes intelligentes** (thermomètre + goutte)
- ✅ **Indicateur WiFi** en temps réel

---

## 🔧 Configuration Arduino IDE

| Paramètre | Valeur |
|-----------|--------|
| **Carte** | ESP32C3 Dev Module |
| **Partition** | Huge APP (3MB No OTA/1MB SPIFFS) |
| **USB CDC On Boot** | Enabled |
| **Version ESP32** | 3.3.5 |

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

**Toujours connecté** : SmartThings voit l'appareil en permanence

---

## 🎨 Interface

```
┌──────────────────────┐
│              📶      │  WiFi (toujours visible)
│                      │
│       23.3°          │  Température
│                      │
│       42.3%          │  Humidité
│                      │
│    🌡️ ↑    💧 ↓    │  Icônes + Tendances
│   ─────────────────  │
│                      │
│   Graphique 24h      │  Historique
│      /\  /\          │
│     /  \/  \         │
│                      │
└──────────────────────┘
```

---

## 🚀 Installation

### 1. Premier boot

```
1. Téléverse le code
2. Ouvre le moniteur série (115200 baud)
3. L'écran affiche le QR code
4. Scanne avec SmartThings
```

### 2. Commissioning SmartThings

```
SmartThings App:
  → + (Ajouter)
  → Appareil
  → Scanner le code QR
  → Scanne le QR sur l'écran e-paper
  → Suit les instructions
```

### 3. Fonctionnement normal

```
✅ Appareil visible dans SmartThings
✅ Température et humidité mises à jour toutes les 15 min
✅ Graphique historique visible
✅ Tendances affichées dès le 2e cycle
```

---

## ⚙️ Personnalisation

### Changer l'intervalle de mise à jour

Dans le code, ligne ~60 :

```cpp
const unsigned long UPDATE_INTERVAL = 2 * 60 * 1000;  // 2 minutes
```

**Exemples :**
- 1 minute : `1 * 60 * 1000`
- 5 minutes : `5 * 60 * 1000`
- 10 minutes : `10 * 60 * 1000`

### Augmenter l'historique du graphique

**Par défaut :** 24 points = 48 minutes d'historique (avec update 2 min)

**Pour 24h d'historique** avec update 2 min :

```cpp
#define HISTORY_SIZE 720  // 720 points * 2 min = 1440 min = 24h
```

**Compromis (6 heures) :**

```cpp
#define HISTORY_SIZE 180  // 180 points * 2 min = 6h
```

**Compromis (3 heures) :**

```cpp
#define HISTORY_SIZE 90   // 90 points * 2 min = 3h
```

**Note :** Plus l'historique est grand, plus il utilise de RAM (4 bytes par point).

### Seuils de tendances

Dans `updateAndDisplay()`, ligne ~195 :

```cpp
// Température
if (tempDiff > 0.5) tempTrend = 1;       // Hausse si > +0.5°C
else if (tempDiff < -0.5) tempTrend = -1; // Baisse si < -0.5°C

// Humidité
if (humDiff > 2.0) humTrend = 1;        // Hausse si > +2%
else if (humDiff < -2.0) humTrend = -1;  // Baisse si < -2%
```

### Seuils d'alerte humidité

Dans `drawHumidityIcon()`, ligne ~410 :

```cpp
if (hum > 70 || hum < 30) {  // Alerte si > 70% ou < 30%
  // Point d'exclamation dans la goutte
}
```

---

## 🔧 Decommissioning

Pour retirer l'appareil de SmartThings et recommencer :

**Maintiens le bouton BOOT enfoncé pendant 5 secondes**

Le moniteur série affichera :
```
Decommissioning...
```

L'appareil redémarre et affiche à nouveau le QR code.

---

## 📋 Logs attendus

### Premier boot

```
=== CAPTEUR MATTER ===
Mode: Alimentation permanente

[OK] DHT22
[OK] Ecran e-paper
[OK] Sensors initialises
[OK] Matter demarre

========================================
  APPAREIL NON COMMISSIONNE
========================================
Code manuel: 34970112332
QR Payload: MT:Y.K9042C00KA0648G00
========================================

>>> SCANNE LE QR CODE <<<
```

### Fonctionnement normal

```
Attente connexion Matter...
..........
[OK] Connecte!

Capteur operationnel!
Mise a jour toutes les 2 minutes

T: 23.3C, H: 42.3%
Tendance temp: ↑ (+0.8°C)
Prochaine mise a jour dans 2 min

T: 23.1C, H: 43.1%
Tendance temp: → (-0.2°C)
Prochaine mise a jour dans 2 min
```

---

## 🐛 Troubleshooting

### L'appareil n'apparaît pas dans SmartThings

1. Vérifier WiFi 2.4GHz (pas 5GHz)
2. Hub SmartThings sur même réseau
3. Re-scanner le QR code

### Les tendances ne s'affichent pas

Normal au premier cycle (pas de valeur précédente).
Attendre 2 minutes pour le 2e cycle.

### Le graphique est vide

Normal au démarrage. Se remplit progressivement :
- 10 min : 5 points
- 48 min : 24 points (graphique complet avec config par défaut)

**Note :** Pour un graphique 24h, augmente `HISTORY_SIZE` à 720 (voir section Personnalisation).

### Icône WiFi déconnectée

Si WiFi coupe momentanément, l'icône se met à jour au prochain cycle (2 min max).

---

## 🚀 Prêt à l'emploi !

1. ✅ Télécharge le dossier
2. ✅ Téléverse le code
3. ✅ Scanne le QR
4. ✅ Profite ! 🎉
