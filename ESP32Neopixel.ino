
// Dernière modif: 19/06/26
// Signaux Néopixel,   ESP32,  Ruban 240 LED par mètre p30

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <DCC_Decoder.h>                // Minabay library
#include <CircularBuffer.h>
#define kDCC_INTERRUPT    35            // pin 35 receives DCC interrupts
#define LED_BUILTIN       2             // LED ESP32
#define LED_COUNT  64                   // Nombre de LED max. par ruban 
#define BRIGHTNESS 30                   // NeoPixel brightness, 0 (min) to 255 (max)
#define OFFSET_DECODEUR  1

//===== Configuration de base. Attribution des sorties décodeurs virtuels =============
//===== Décodeur 1 =========================
// 1  Voie libre, Vert
// 2  Voie libre, Vert cli
// 3  Avertissement, Jaune
// 4  Avertissement, Jaune cli
// 5  Sémaphore, Rouge
// 6  Sémaphore, Rouge cli
// 7  Carré, 2 Rouges
// 8  Carré, 2 Rouges cli
//===== Décodeur 2 =========================
// 9  Manœuvre, Blanc
// 10 Manœuvre, Blanc cli
// 11 Ralentissement, 2 jaunes horizontaux
// 12 Ralentissement, 2 jaunes horizontaux cli
// 13 Rappel ralentissement, 2 jaunes verticaux
// 14 Rappel ralentissement, 2 jaunes verticaux cli
// 15 Carré violet
// 16 Carré violet blanc cli
//===== Décodeur 3 =========================
// 17 Direction 1 ON           O X X
// 18 Direction 1 OFF          X X X
// 19 Direction 1 et 2 ON      O O X
// 20 Direction 1 et 2 OFF     X X X
// 21 Direction 1, 2 et 3 ON   O O O
// 22 Direction 1, 2 et 3 OFF  X X X
// 23 Oeuilleton ON
// 24 Oeuilleton OFF

// === Ecrans SNCF ===
const int I = 2;                        // 2 feux, violet blanc                                                             1 Décodeur vituel
const int RV = 2;                       // 2 feux, rouge vert                                                               1 Décodeur vituel
const int A = 3;                        // 3 feux, jaune rouge vert                                                         1 Décodeur vituel
const int B = 4;                        // 4 feux, jaune rouge vert blanc                                                   1 Décodeur vituel
const int C = 5;                        // 5 feux, jaune rouge vert blanc rouge (ou violet)                                 2 décodeurs virtuels
const int R = 6;     // Disque          // 6 feux, jaune jaune vert jaune rouge jaune                                       2 décodeurs virtuels
const int F = 7;                        // 7 feux, jaune rouge vert blanc rouge jaune jaune                                 2 décodeurs virtuels
const int FID = 10;                     // 10 feux, jaune rouge vert blanc rouge jaune jaune blanc blanc blanc              3 décodeurs virtuels (Indicateur de direction)
const int H = 9;                        // 9 feux, jaune rouge vert blanc rouge jaune jaune jaune jaune                     2 décodeurs virtuels
const int HID = 12;                     // 12 feux, jaune rouge vert blanc rouge jaune jaune jaune jaune blanc blanc blanc  3 Décodeurs virtuels (Indicateur de direction)

const int8_t TabSignaux[112][2] = {                                        // Tableau des signaux, pour 14 rubans de 8 signaux
  {3, B}, {2, A}, {4, A}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},          // Ruban LED 1: N° de signal et type d'écran
  {11, F}, {9, HID}, {15, A}, {16, F}, {0, 0}, {0, 0}, {0, 0}, {0, 0},     // Ruban LED 2: N° de signal et type d'écran
  {12, C}, {5, F}, {13, A}, {18, RV}, {0, 0}, {0, 0}, {0, 0}, {0, 0},      // Ruban LED 3: N° de signal et type d'écran
  {1, A}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},          // Ruban LED 4: N° de signal et type d'écran
  {8, A}, {10, H}, {14, B}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},        // Ruban LED 5: N° de signal et type d'écran
  {7, R}, {17, I}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},         // Ruban LED 6: N° de signal et type d'écran
  {6, A}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},          // Ruban LED 7: N° de signal et type d'écran
  {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},          // Ruban LED 8: N° de signal et type d'écran
  {12, C}, {5, F}, {13, A}, {18, RV}, {0, 0}, {0, 0}, {0, 0}, {0, 0},      // Ruban LED 9: N° de signal et type d'écran
  {20, C}, {21, F}, {22, R}, {23, H}, {0, 0}, {0, 0}, {0, 0}, {0, 0},      // Ruban LED 10: N° de signal et type d'écran
  {8, A}, {10, H}, {14, B}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},        // Ruban LED 11: N° de signal et type d'écran
  {7, R}, {17, I}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},         // Ruban LED 12: N° de signal et type d'écran
  {6, A}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},          // Ruban LED 13: N° de signal et type d'écran
  {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}           // Ruban LED 14: N° de signal et type d'écran
};

Adafruit_NeoPixel strip1(LED_COUNT, 13, NEO_GRB + NEO_KHZ800);            // Ruban LED 1 (PIN GPIO)
Adafruit_NeoPixel strip2(LED_COUNT, 14, NEO_GRB + NEO_KHZ800);            // Ruban LED 2
Adafruit_NeoPixel strip3(LED_COUNT, 16, NEO_GRB + NEO_KHZ800);            // Ruban LED 3
Adafruit_NeoPixel strip4(LED_COUNT, 17, NEO_GRB + NEO_KHZ800);            // Ruban LED 4
Adafruit_NeoPixel strip5(LED_COUNT, 18, NEO_GRB + NEO_KHZ800);            // Ruban LED 5
Adafruit_NeoPixel strip6(LED_COUNT, 19, NEO_GRB + NEO_KHZ800);            // Ruban LED 6
Adafruit_NeoPixel strip7(LED_COUNT, 21, NEO_GRB + NEO_KHZ800);            // Ruban LED 7
Adafruit_NeoPixel strip8(LED_COUNT, 22, NEO_GRB + NEO_KHZ800);            // Ruban LED 8
Adafruit_NeoPixel strip9(LED_COUNT, 23, NEO_GRB + NEO_KHZ800);            // Ruban LED 9
Adafruit_NeoPixel strip10(LED_COUNT, 25, NEO_GRB + NEO_KHZ800);           // Ruban LED 10
Adafruit_NeoPixel strip11(LED_COUNT, 26, NEO_GRB + NEO_KHZ800);           // Ruban LED 11
Adafruit_NeoPixel strip12(LED_COUNT, 27, NEO_GRB + NEO_KHZ800);           // Ruban LED 12
Adafruit_NeoPixel strip13(LED_COUNT, 32, NEO_GRB + NEO_KHZ800);           // Ruban LED 13
Adafruit_NeoPixel strip14(LED_COUNT, 33, NEO_GRB + NEO_KHZ800);           // Ruban LED 14

CircularBuffer <int, 20> DataRx;
const uint32_t vert = 0x00FF00;             // Vert          http://www.proftnj.com/RGB3.htm
const uint32_t rouge = 0xFF0000;            // Rouge
const uint32_t violet = 0x9900CC;           // Violet
const uint32_t orange = 0xFF4500;           // Orange
const uint32_t jaune = 0xFFFF00;            // Jaune
const uint32_t noir = 0x000000;             // Noir
const uint32_t blanc = 0x808080;            // Blanc
uint8_t nbrLedRuban[14];// = {0, 0, 0, 0, 0, 0, 0, 0};  // Nombre de LED par ruban
uint32_t feu[10];
uint8_t numRuban;                                   // 14 rubans
uint8_t adrLED;
uint8_t numLED = 0;
uint8_t numDeco;
uint8_t Ecran;                                      // RV, A, B, C, F, FID, H, HID, I
uint8_t numSignal;                                  // 1 à 112
uint8_t numSortie;                                  // 1 à 24
uint8_t i, nf;                                      // N° du feu
bool cli;                                           // Clignotement ON OFF
uint16_t timeCli = 700;                             // ms. Temps de clignotement
unsigned long currentTimeCli;                       // Millis clignotement
unsigned long previousTimeCli;
uint32_t TabFeuxCli[64][14];
//--------------------------------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);                                                                // Moniteur
  delay(100);
  DCC.SetBasicAccessoryDecoderPacketHandler(BasicAccDecoderPacket_Handler, true);      // instanciate the DCC
  DCC.SetupDecoder( 0x00, 0x00, kDCC_INTERRUPT );                                        // its IT
  strip1.begin();
  strip2.begin();
  strip3.begin();
  strip4.begin();
  strip5.begin();
  strip6.begin();
  strip7.begin();
  strip8.begin();
  strip9.begin();
  strip10.begin();
  strip11.begin();
  strip12.begin();
  strip13.begin();
  strip14.begin();
  strip1.setBrightness(BRIGHTNESS);
  strip2.setBrightness(BRIGHTNESS);
  strip3.setBrightness(BRIGHTNESS);
  strip4.setBrightness(BRIGHTNESS);
  strip5.setBrightness(BRIGHTNESS);
  strip6.setBrightness(BRIGHTNESS);
  strip7.setBrightness(BRIGHTNESS);
  strip8.setBrightness(BRIGHTNESS);
  strip9.setBrightness(BRIGHTNESS);
  strip10.setBrightness(BRIGHTNESS);
  strip11.setBrightness(BRIGHTNESS);
  strip12.setBrightness(BRIGHTNESS);
  strip13.setBrightness(BRIGHTNESS);
  strip14.setBrightness(BRIGHTNESS);

  pinMode(LED_BUILTIN , OUTPUT);
  numRuban = 1;                                               // Num ruban
  int nd = OFFSET_DECODEUR;                                   // nd, Num Déco
  for (i = 0; i < 112; i++) {                                 // Calcul, nombre de LED par ruban
    nbrLedRuban[numRuban] += TabSignaux[i][1];                // Additionne les écrans
    if ((i & 7) == 7)
      numRuban++;                                             // Ruban suivant
  }
  Info();
  AllClear();
}
//----------------------------------------------------------------------------------------------------------------
void loop() {
  DCC.loop();
  while (!DataRx.isEmpty())
    SignalDetect();

  currentTimeCli = millis();
  if ((currentTimeCli - previousTimeCli) < timeCli)       // 700ms.
    return;
  previousTimeCli = currentTimeCli;
  Clignotement();                                        // Clignotement des feux
}
//-----------------------------------------------------------------------------------------------------------------
void BasicAccDecoderPacket_Handler(int numDecoder, boolean activate, byte numSortie) {
  if (activate) {
    DataRx.push (numDecoder + 1);                          // Mémorise les data dans tampon circulaire.
    DataRx.push (numSortie + 1);
  }
}
//-------------------------------------------------------------------------------------------------------------------
void SignalDetect() {                                    // Recherche du signal
  digitalWrite(LED_BUILTIN , HIGH);                      // LED ESP ON
  numDeco = DataRx.shift();                              // Reprend n° de décodeur
  numSortie = DataRx.shift();                            // Reprend n° de numSortie
  adrLED = 0;
  int nD = 0;
  int sortie;
  numRuban = 1;
  for (i = 0; i < 112; i++) {                             // Cherche sur quel ruban est le signal.
    if ((TabSignaux[i][1]) > 0) {
      nD++;
      sortie = 0;
      if (numDeco == nD) break;                          // Décodeur trouvé, break
    }
    if ((TabSignaux[i][1]) > 4) {
      nD++;
      sortie = 8;
      if (numDeco == nD) break;                          // Décodeur trouvé, break
    }
    if ((TabSignaux[i][1]) > 9) {
      nD++;
      sortie = 16;
      if (numDeco == nD) break;                              // Décodeur trouvé, break
    }
    adrLED += TabSignaux[i][1];                          // Adresse de base du signal (n° LED du bas)
    if ((i & 7) == 7) {
      numRuban++;                       // Ruban suivant
      adrLED = 0;                       // Début ruban
    }
  }
  numSortie += sortie;
  Ecran = TabSignaux[i][1];                              // Type d'écran
  Debug(nD, numSortie);
  SignalChange();                                        // Nouvel aspect du signal en RAM
  SignalUpdate();                                        // RAM vers ruban LED
  digitalWrite(LED_BUILTIN , LOW);                       // LED ESP OFF
}
//---------------------------------------------------------------------------------------------------------------
void SignalChange() {
  if (numSortie > Ecran * 2)
    return;
  if (numSortie < 17) {                                   // Efface signal sauf indicateur de directions et oeuilleton
    for (int j = 0; j < Ecran; j++) {
      feu[j] = noir;                                      // Signal noir
      TabFeuxCli[j + adrLED][numRuban] = noir;            // Feux lignotants OFF
    }
  }
  switch (numSortie) {//======================== Signaux SNCF ===========================================
    case 1:
    case 2: feu[2] = vert;  break;                               // Voie libre
    case 3:
    case 4:                                                      // Avertissement (Jaune)
      switch (Ecran) {
        case R:                                                  // Disque
          feu[3] = jaune;
          break;
        default:
          feu[0] = jaune;
      } break;
    case 5:
    case 6:                                                      // Sémaphore (Rouge)
      switch (Ecran) {
        case R:                                                  // Disque
          feu[4] = rouge;
          feu[0] = jaune;
          break;
        default:
          feu[1] = rouge;
      } break;
    case 7: feu[1] = rouge;
    case 8: feu[1] = rouge;                                      // Carré (2 Rouges)
      switch (Ecran) {
        case B: feu[3] = blanc;
          break;
        case R:                                                  // Disque
          feu[1] = noir;
          break;
        default:
          feu[4] = rouge;
      } break;
    case 9:
    case 10:
      if (Ecran != R)                                             // Disque pas de blanc
        feu[3] = blanc; break;                                    // Manœuvre (Blanc)
    case 11:
    case 12: feu[5] = jaune;                                      // Ralentissement (2 jaunes horizontaux)
      switch (Ecran) {
        case R:
          feu[1] = jaune;                                         // Disque
          break;
        case F:
          feu[6] = jaune;                                         // Ecran F
          break;
        case H:
          feu[7] = jaune;                                         // Ecran H
          break;
      } break;
    case 13:
    case 14:
      if (Ecran == H) {                                           // Rappel ralentissement(2 jaunes verticaux)
        feu[6] = jaune;
        feu[8] = jaune;
      } break;
    case 15:
    case 16: feu[4] = violet;                                     // Carré violet blanc
      if (Ecran == I) {
        feu[0] = violet;
        feu[1] = blanc;
      } break;
    case 17: feu[9] = blanc; break;                                   // O X X     Indicateur de directions
    case 18: feu[9] = noir; break;                                    // X X X
    case 19: feu[9] = blanc; feu[10] = blanc; break;                  // O O X
    case 20: feu[9] = noir; feu[10] = noir; break;                    // X X X
    case 21: feu[9] = blanc; feu[10] = blanc; feu[11] = blanc; break; // O O O
    case 22: feu[9] = noir; feu[10] = noir; feu[11] = noir; break;    // X X X
    case 23: feu[12] = blanc; break;                                   // Oeuilleton ON
    case 24: feu[12] = noir; break;                                    // Oeuilleton OFF
    default: Serial.println(F("Erreur numSortie"));
  }
  if (numSortie < 17) {
    for (int j = 0; j < Ecran; j++) {
      if (feu[j] != noir) {
        if (numSortie % 2 == 0)                                        // Pair
          TabFeuxCli[j + adrLED][numRuban] = feu[j];                   // Feux clignotants
      }
    }
  }
}
//-------------------------------------------------------------------------------------------------------------------
void SignalUpdate() {
  for (nf = 0; nf < Ecran ; nf++) {                      // nf = N° du feu
    SignalSet();
  }
  SignalShow();
}
//------------------------------------------------------------------------------------------------------------
void SignalSet() {
  switch (numRuban) {                                   // Sélection du ruban
    case 1: strip1.setPixelColor(adrLED + nf, feu[nf]); break;
    case 2: strip2.setPixelColor(adrLED + nf, feu[nf]); break;
    case 3: strip3.setPixelColor(adrLED + nf, feu[nf]); break;
    case 4: strip4.setPixelColor(adrLED + nf, feu[nf]); break;
    case 5: strip5.setPixelColor(adrLED + nf, feu[nf]); break;
    case 6: strip6.setPixelColor(adrLED + nf, feu[nf]); break;
    case 7: strip7.setPixelColor(adrLED + nf, feu[nf]); break;
    case 8: strip8.setPixelColor(adrLED + nf, feu[nf]); break;
    case 9: strip9.setPixelColor(adrLED + nf, feu[nf]); break;
    case 10: strip10.setPixelColor(adrLED + nf, feu[nf]); break;
    case 11: strip11.setPixelColor(adrLED + nf, feu[nf]); break;
    case 12: strip12.setPixelColor(adrLED + nf, feu[nf]); break;
    case 13: strip13.setPixelColor(adrLED + nf, feu[nf]); break;
    case 14: strip14.setPixelColor(adrLED + nf, feu[nf]); break;
  }
}
//-----------------------------------------------------------------------------------------------------------
void SignalShow() {
  switch (numRuban) {                                   // Sélection du ruban
    case 1: strip1.show(); break;                       // RAM vers LED
    case 2: strip2.show(); break;
    case 3: strip3.show(); break;
    case 4: strip4.show(); break;
    case 5: strip5.show(); break;
    case 6: strip6.show(); break;
    case 7: strip7.show(); break;
    case 8: strip8.show(); break;
    case 9: strip3.show(); break;
    case 10: strip4.show(); break;
    case 11: strip5.show(); break;
    case 12: strip6.show(); break;
    case 13: strip7.show(); break;
    case 14: strip8.show(); break;
  }
}
//----------------------------------------------------------------------------------------------------------
void Clignotement() {
  cli = !cli;
  for (int i = 0; i < 64; i++) {
    if (TabFeuxCli[i][numRuban] != noir)
      SignalCli(i);
  }
  SignalShow();
}
//------------------------------------------------------------------------------
void SignalCli(int i) {
  switch (numRuban) {                                   // Sélection du ruban
    case 1: strip1.setPixelColor(i, cli ? noir : TabFeuxCli[i][numRuban]) ; break;
    case 2: strip2.setPixelColor(i, cli ? noir : TabFeuxCli[i][numRuban]) ; break;
    case 3: strip3.setPixelColor(i, cli ? noir : TabFeuxCli[i][numRuban]) ; break;
    case 4: strip4.setPixelColor(i, cli ? noir : TabFeuxCli[i][numRuban]) ; break;
    case 5: strip5.setPixelColor(i, cli ? noir : TabFeuxCli[i][numRuban]) ; break;
    case 6: strip6.setPixelColor(i, cli ? noir : TabFeuxCli[i][numRuban]) ; break;
    case 7: strip7.setPixelColor(i, cli ? noir : TabFeuxCli[i][numRuban]) ; break;
    case 8: strip8.setPixelColor(i, cli ? noir : TabFeuxCli[i][numRuban]) ; break;
    case 9: strip9.setPixelColor(i, cli ? noir : TabFeuxCli[i][numRuban]) ; break;
    case 10: strip10.setPixelColor(i, cli ? noir : TabFeuxCli[i][numRuban]) ; break;
    case 11: strip11.setPixelColor(i, cli ? noir : TabFeuxCli[i][numRuban]) ; break;
    case 12: strip12.setPixelColor(i, cli ? noir : TabFeuxCli[i][numRuban]) ; break;
    case 13: strip13.setPixelColor(i, cli ? noir : TabFeuxCli[i][numRuban]) ; break;
    case 14: strip14.setPixelColor(i, cli ? noir : TabFeuxCli[i][numRuban]) ; break;
  }
}
//----------------------------------------------------------------------------------------------------------
void AllClear() {
  strip1.clear();
  strip2.clear();
  strip3.clear();
  strip4.clear();
  strip5.clear();
  strip6.clear();
  strip7.clear();
  strip8.clear();
}
//-------------------------------------------------------------------------------------------------------------
void Info() {
  numDeco = 0;
  numRuban = 1;
  Serial.println("\n============ Configuration signaux décodeurs ============");
  Serial.print("\n--- Ruban n° ");
  Serial.print(numRuban);
  Serial.print(F(": "));
  Serial.print(nbrLedRuban[numRuban]);                       // Affiche, nombre de LED par ruban
  Serial.println(F(" LED ---"));
  for (i = 0; i < 111; i++) {                               // Attribue un, deux ou trois n° de décodeur suivant écran du signal
    if ((TabSignaux[i][1]) > 0) {
      Serial.print("Signal n° ");
      Serial.print(TabSignaux[i][0]);
      Serial.print(". Décodeur n° ");
      numDeco++;
      Serial.print(numDeco);
      if ((TabSignaux[i][1]) > 4) {
        numDeco++;
        Serial.print(", ");
        Serial.print(numDeco);
      }
      if ((TabSignaux[i][1]) > 9) {
        numDeco++;
        Serial.print(", ");
        Serial.print(numDeco);
      }
      Serial.println();
    }
    if ((i & 7) == 7) {
      numRuban++;
      Serial.print("\n--- Ruban n° ");
      Serial.print(numRuban);
      Serial.print(F(": "));
      Serial.print(nbrLedRuban[numRuban]);                     // Affiche, nombre de LED par ruban
      Serial.println(F(" LED ---"));
    }
  }
}
//-------------------------------------------------------------------------------------------------------------
void Debug(int nd, int ns) {
  Serial.print("Signal n° ");
  Serial.print(TabSignaux[i][0]);
  Serial.print("  ");
  Serial.print(F(" Décodeur N°: ")) ;
  Serial.print(nd);
  Serial.print("  ");
  Serial.print(F(" Num sortie: ")) ;
  Serial.print(ns);
  Serial.print("  ");
  Serial.print(F(" Ecran: ")) ;
  Serial.print(TabSignaux[i][1]);
  Serial.print("  ");
  Serial.print(F(" Adr LED: ")) ;
  Serial.print(adrLED);
  Serial.print("  ");
  Serial.print(F(" Num ruban: ")) ;
  Serial.println(numRuban);
}
