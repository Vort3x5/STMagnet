#set document(title: "Część Fizyczna")
#set document(author: "Szymon Boniuk")
#set document(author: "Jakub Celiński")
#set page(paper: "a4", margin: 2.5cm)
#set text(lang: "pl", font: "New Computer Modern", size: 12pt)
#set par(justify: true)

#align(center)[
  #text(size: 18pt, weight: "bold")[
    Część Fizyczna

    Szymon Boniuk & Jakub Celiński
  ]
  
  #v(0.5em)
  
  #text(size: 16pt)[
    Cyfrowy Magnetometr z Wyświetlaczem OLED
  ]
  
  #v(1em)
]

= Cel Projektu

Celem projektu jest zbudowanie działającego magnetometru cyfrowego z wykorzystaniem mikrokontrolera STM32, 
sensora magnetycznego oraz wyświetlacza OLED. 
Urządzenie mierzy natężenie lokalnego pola magnetycznego w trzech osiach przestrzennych i prezentuje wyniki w czasie rzeczywistym na ekranie.

= Wykorzystane Komponenty Sprzętowe

#table(
  columns: (1.5fr, 3fr),
  align: (left, left),
  stroke: 0.5pt,
  [*Komponent*], [*Opis*],
  [STM32 Nucleo-F446RE], [Płytka rozwojowa z mikrokontrolerem ARM Cortex-M4 o częstotliwości 84 MHz],
  [GY-271 (QMC5883P)], [Trzyosiowy magnetometr cyfrowy z interfejsem I²C],
  [Wyświetlacz OLED 128×64], [Monochromatyczny ekran z kontrolerem SSD1306],
)

= Podstawy Fizyczne

== Efekt Halla

Projekt wykorzystuje czujnik magnetyczny oparty na *efekcie Halla*, który jest podstawowym zjawiskiem fizycznym umożliwiającym detekcję pola magnetycznego. 

Gdy przez przewodnik płynie prąd elektryczny $I$ w kierunku prostopadłym do zewnętrznego pola magnetycznego $arrow(B)$, na nośniki ładunku działa *siła Lorentza*:

$ arrow(F) = q(arrow(v) times arrow(B)) $

gdzie:
- $q$ -- ładunek nośnika
- $arrow(v)$ -- prędkość nośnika ładunku
- $arrow(B)$ -- wektor indukcji magnetycznej

Siła ta powoduje nagromadzenie ładunków po jednej stronie przewodnika, co prowadzi do powstania *napięcia Halla* $V_H$ prostopadłego zarówno do kierunku prądu, jak i pola magnetycznego:

$ V_H = (I B)/(n q t) $

gdzie:
- $n$ -- koncentracja nośników ładunku
- $t$ -- grubość przewodnika

Pomiar tego napięcia pozwala na wyznaczenie natężenia pola magnetycznego. W czujniku QMC5883P zastosowano trzy niezależne elementy Halla zorientowane wzajemnie prostopadle, co umożliwia pomiar wszystkich trzech składowych wektora $arrow(B)$.

== Pole Magnetyczne Ziemi

Magnetometr wykrywa przede wszystkim *pole magnetyczne Ziemi*, które w Polsce charakteryzuje się następującymi parametrami:

- Całkowite natężenie: około *50 µT* (mikrotesli)
- Składowa horyzontalna: $B_h approx 20$ µT
- Składowa wertykalna: $B_v approx 45$ µT
- Inklinacja magnetyczna: około *67°*

Pole magnetyczne Ziemi powstaje w wyniku procesów zachodzących w zewnętrznym jądrze naszej planety, gdzie konwekcja stopionego żelaza generuje prądy elektryczne tworzące pole magnetyczne zgodnie z prawami elektromagnetyzmu.

Wartość bezwzględną całkowitego wektora indukcji magnetycznej obliczamy ze wzoru:

$ |arrow(B)| = sqrt(B_x^2 + B_y^2 + B_z^2) $

== Komunikacja I²C - Podstawy Elektrotechniczne

Czujnik magnetyczny QMC5883P komunikuje się z mikrokontrolerem przez magistralę *I²C* (Inter-Integrated Circuit). Jest to dwukierunkowa, szeregowa magistrala danych wykorzystująca dwie linie:

- *SDA* (Serial Data) -- linia danych
- *SCL* (Serial Clock) -- linia zegarowa

Transmisja danych odbywa się w sposób szeregowy, synchronicznie z sygnałem zegarowym. Magistrala I²C wykorzystuje konfigurację z otwartym kolektorem (open-drain), wymagającą rezystorów podciągających (pull-up) do napięcia zasilania.

Adres I²C czujnika QMC5883P to `0x2C` (w 7-bitowej notacji), co odpowiada fizycznemu adresowi `0x58` w 8-bitowej transmisji.

#pagebreak()

== Kalibracja i Kompensacja Zakłóceń Magnetycznych

Dokładny pomiar pola magnetycznego wymaga uwzględnienia zakłóceń pochodzących z otoczenia i samego urządzenia. Wyróżniamy dwa główne rodzaje zakłóceń:

=== Zakłócenia Twarde (Hard-Iron)

Zakłócenia twarde pochodzą od elementów ferromagnetycznych trwale związanych z urządzeniem (np. elementy PCB, komponenty elektroniczne). Powodują one *stałe przesunięcie* odczytów we wszystkich trzech osiach.

Matematycznie można je reprezentować jako wektor offsetu $arrow(O) = [O_x, O_y, O_z]$.

=== Zakłócenia Miękkie (Soft-Iron)

Zakłócenia miękkie wynikają z *odkształcenia* pola magnetycznego przez materiały ferromagnetyczne w pobliżu czujnika. Skutkują one niejednorodnymi współczynnikami skalowania w różnych kierunkach.

Reprezentowane są przez macierz przekształcenia $S$.

=== Proces Kalibracji

Kalibracja polega na wyznaczeniu elipsoidy pomiarowej w przestrzeni 3D i przekształceniu jej na sferę poprzez operacje przesunięcia i skalowania:

$ arrow(B)_("kal") = S dot (arrow(B)_("raw") - arrow(O)) $

W praktyce, kalibrację przeprowadza się poprzez obrót czujnika w różnych orientacjach i zarejestrowanie maksymalnych oraz minimalnych wartości w każdej osi. Następnie oblicza się:

$ O_i = (B_(i,"max") + B_(i,"min"))/2 $

$ S_i = 2/(B_(i,"max") - B_(i,"min")) $

dla $i in {x, y, z}$.

== Filtracja Sygnału - Redukcja Szumów

Ze względu na szumy pomiarowe zastosowano *cyfrowy filtr dolnoprzepustowy* pierwszego rzędu, zwany również wykładniczą średnią ruchomą (Exponential Moving Average, EMA):

$ y_n = alpha dot x_n + (1 - alpha) dot y_(n-1) $

gdzie:
- $alpha in (0, 1)$ -- współczynnik wygładzania
- $x_n$ -- aktualny pomiar surowy
- $y_(n-1)$ -- poprzednia wartość filtrowana
- $y_n$ -- aktualna wartość filtrowana

Filtr ten działa jako *filtr dolnoprzepustowy RC pierwszego rzędu*, tłumiąc szybkie zmiany sygnału (wysokie częstotliwości - szumy), zachowując powolne zmiany pola magnetycznego (niskie częstotliwości - sygnał użyteczny).

Częstotliwość graniczna filtra $f_c$ związana jest ze współczynnikiem $alpha$ i częstotliwością próbkowania $f_s$:

$ f_c = (f_s)/(2 pi) dot (alpha)/(1 - alpha) $

= Schemat Elektryczny Połączeń

Poniżej przedstawiono schemat połączeń elektrycznych wszystkich komponentów systemu:

```
╔══════════════════════════════════════════════════════════════════╗
║                   STM32 NUCLEO-F446RE                            ║
║                   ARM Cortex-M4 @ 84 MHz                         ║
╚══════════════════════════════════════════════════════════════════╝
             │                    │                    │
             │ PB8 (I2C1_SCL)     │ PB9 (I2C1_SDA)     │ PA2 (USART2_TX)
             │                    │                    │ PA3 (USART2_RX)
             │                    │                    │
             ↓                    ↓                    ↓
   ┌─────────┴────────────────────┴──────────┐      [UART → PC]
   │         I²C Bus (3.3V logic)            │
   │    ┌──────────────┬──────────────┐      │
   │    │ Pull-up 4.7k │ Pull-up 4.7k │      │
   │    │    ↑ VCC     │    ↑ VCC     │      │
   │    │              │              │      │
   ↓    ↓              ↓              │      │
┌──────────────┐  ┌──────────────────────────┴─────┐
│  QMC5883P    │  │  OLED Display SSD1306          │
│  3-Axis      │  │  128×64 pixels                 │
│  Magnetometer│  │  I²C Interface                 │
├──────────────┤  ├────────────────────────────────┤
│ VCC  → 3.3V  │  │ VCC  → 3.3V                    │
│ GND  → GND   │  │ GND  → GND                     │
│ SCL  → PB8   │  │ SCL  → PB8 (I2C1_SCL)          │
│ SDA  → PB9   │  │ SDA  → PB9 (I2C1_SDA)          │
└──────────────┘  └────────────────────────────────┘
   I²C Addr:          I²C Addr:
   0x2C (7-bit)       0x3C (7-bit)
   0x58 (8-bit)       0x78 (8-bit)

════════════════════════════════════════════════════════════════════
Zasilanie: 3.3V z pinu STM32 Nucleo
Magistrala I²C: Standard Mode (100 kHz) lub Fast Mode (400 kHz)
Pull-up resistors: 4.7 kΩ (wbudowane w moduły GY-271 i OLED)
════════════════════════════════════════════════════════════════════
```

== Szczegółowy Opis Połączeń

=== Magistrala I²C

Oba urządzenia peryferyjne (magnetometr QMC5883P i wyświetlacz OLED) współdzielą tę samą magistralę I²C:

- *SCL (Serial Clock)*: PB8 -- sygnał zegarowy generowany przez STM32 (master)
- *SDA (Serial Data)*: PB9 -- dwukierunkowa linia danych

Rezystory podciągające (pull-up) o wartości 4.7 kΩ są zazwyczaj wbudowane w moduły GY-271 i OLED, zapewniając prawidłową pracę magistrali w konfiguracji open-drain.

=== Zasilanie

Wszystkie komponenty zasilane są napięciem *3.3V* z pinu zasilającego STM32 Nucleo:
- Napięcie logiczne: 3.3V (zgodne z logiką CMOS mikrokontrolera)
- Pobór prądu QMC5883P: ~100 µA (tryb ciągły)
- Pobór prądu OLED: ~20 mA (zależny od liczby zapalonych pikseli)

=== Interfejs Szeregowy UART

Mikrokontroler komunikuje się z komputerem przez interfejs *UART* (USART2):
- *TX (PA2)*: transmisja danych do PC
- *RX (PA3)*: odbiór danych z PC
- Prędkość: 115200 baud

Interfejs ten wykorzystywany jest do debugowania i wyświetlania szczegółowych informacji o pomiarach.

#pagebreak()

= Funkcjonalność Systemu

System realizuje następujące funkcje:

+ *Inicjalizacja czujnika QMC5883P*:
  - Weryfikacja ID chipu (oczekiwana wartość: 0x80)
  - Soft reset układu
  - Odblokowanie ukrytych rejestrów
  - Konfiguracja trybu pomiaru ciągłego
  - Ustawienie zakresu pomiarowego: ±8 Gauss
  - Częstotliwość próbkowania: 100 Hz
  - Nadpróbkowanie (OSR): 512 próbek

+ *Ciągły pomiar trzech składowych pola magnetycznego* ($B_x$, $B_y$, $B_z$)

+ *Kalibracja czujnika* w celu eliminacji zakłóceń hard-iron i soft-iron

+ *Cyfrowa filtracja sygnału* w celu redukcji szumów pomiarowych

+ *Obliczanie całkowitej wartości natężenia pola*:
  $ |arrow(B)| = sqrt(B_x^2 + B_y^2 + B_z^2) $

+ *Wyświetlanie pomiarów w czasie rzeczywistym* na ekranie OLED

+ *Wizualizacja danych przez interfejs szeregowy* UART dla celów diagnostycznych
