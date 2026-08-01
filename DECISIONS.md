# Jak podzieliłem kod i dlaczego
Kod został zaprojektowany jako modularny monolit z możliwością późniejszego podziału na niezależne serwisy.

Taka architektura pozwala na stosunkowo łatwe wydzielenie poszczególnych modułów do osobnych procesów wraz z rozwojem projektu. Każdy moduł ma jasno określoną odpowiedzialność, jest możliwie niezależny od pozostałych i łatwy w dalszym rozwijaniu oraz utrzymaniu.

Oczywiście zakres tego podziału był ograniczony czasem przeznaczonym na wykonanie zadania.

# Jak rozwiązałem idempotencję synchronizacji
Każda transakcja otrzymuje unikalny identyfikator UUID zapisany w lokalnej bazie danych, co uniemożliwia zapisanie duplikatów.

Ten sam identyfikator może zostać wykorzystany również po stronie backendu do zapewnienia idempotentności i odrzucania ewentualnych duplikatów wynikających np. z ponownej próby wysłania danych.

Transakcja jest zawsze najpierw zapisywana w lokalnej bazie danych, niezależnie od działania wątku odpowiedzialnego za synchronizację z backendem. Dzięki temu nie powinna zostać utracona.

Rekord jest oznaczany jako zsynchronizowany dopiero po otrzymaniu potwierdzenia poprawnego przetworzenia przez backend.

# Co dzieje się przy zaniku zasilania w trakcie wydawania
Aktualny stan procesu jest zapisywany w pliku JSON znajdującym się w pamięci nieulotnej.

Po ponownym uruchomieniu aplikacja odczytuje zapisany stan i, w zależności od jego wartości, wysyła odpowiedni komunikat do cloud_service, a następnie do backendu.

Zakładam, że bez dodatkowych czujników lub rozszerzonej logiki nie da się jednoznacznie stwierdzić, czy produkt został ostatecznie wydany. W takiej sytuacji operator otrzymuje informację o zdarzeniu i powinien zweryfikować stan urządzenia.

# Czego nie zdążyłem i jak zrobiłbym to w ciągu tygodnia
Dokładny przegląd kodu.
Ujednolicenie stylu kodowania w całym projekcie.
Uzupełnienie dokumentacji projektowej o brakujące szczegóły.
Implementacja pełnoprawnego loggera wykorzystującego operatory <<.
Wydzielenie inicjalizacji wszystkich modułów do dedykowanych klas (Factory / Bootstrap) i maksymalne odchudzenie main().
Dodanie serwera testowego (fake backendu) do weryfikacji komunikacji.
Refaktoryzacja silnika vendingowego poprzez wydzielenie każdego stanu do osobnej klasy zgodnie z wzorcem State.
Dokładniejsze przetestowanie logiki synchronizacji oraz bazy danych.
Dodanie prostego w użyciu debuggera
CICD do projektu
Sprawdzenie wersji Windowsowej - próbowałem na szybko dodać, ale nie było wystarczająco dużo czasu aby dokończyć.

# Ile realnie zajęło wykonanie zadania
Łącznie nieco ponad 4godziny, na podstawie historii commitów:

około 30 minut – zaprojektowanie architektury,
około 1 godzina – przygotowanie projektu i działającego szkieletu aplikacji,
około 1 godzina – połączenie modułów i przygotowanie przepływu danych,
około 30 minut – implementacja maszyny stanów w vending_engine,
około 30 minut – implementacja warstwy SQLite oraz modułu synchronizacji,
pozostały czas – testy, dokumentacja (DECISIONS.md) i poprawki.

# Inne uwagi
Cztery godziny na wykonanie całego projektu to bardzo mało, nawet przy wykorzystaniu narzędzi AI.

Priorytetem było stworzenie architektury spełniającej wymagania zadania, a jednocześnie przygotowanej do dalszego rozwoju.

Rozbudowa projektu powinna być stosunkowo prosta dzięki modułowej strukturze, wyraźnemu podziałowi odpowiedzialności oraz wykorzystaniu interfejsów. Docelowo rozważałbym wydzielenie poszczególnych modułów do osobnych procesów lub usług.

Testy zostały wykonane jedynie w podstawowym zakresie i częściowo z wykorzystaniem AI. W projekcie produkcyjnym należałoby znacząco rozszerzyć ich zakres.

Sama implementacja logiki zajmuje stosunkowo niewiele czasu w porównaniu z przygotowaniem architektury projektu, przepływu danych oraz odpowiedniego podziału odpowiedzialności.

Nie zdążyłem przygotować serwera testowego do weryfikacji komunikacji. W rzeczywistym projekcie byłby to jeden z pierwszych elementów powstających po przygotowaniu wstępnego projektu architektury.

Nie jestem specjalistą od Qt. Moduł ten został wykorzystany głównie jako prosty interfejs do wyzwalania logiki biznesowej. W projekcie produkcyjnym zacząłbym od implementacji warstwy core, komunikując się z nią poprzez broker wiadomości (np. MQTT lub Kafka).