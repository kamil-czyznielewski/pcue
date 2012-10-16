To jest wersja projektu Inetd-BF537 przygotowana do wspó³pracy
z VisualDSP++ 4.5

Poniewa¿ jednak linker wersji 4.5 nie mo¿e odnaleŸæ definicji
jednej z kluczowych funkcji, jej wywo³anie zosta³o zakomentowane.
Trzeba to jakoœ poprawiæ przed wgraniemprojektu na p³ytkê.
Byæ mo¿e zadzia³a zwyczajne przekopiowanie plików z kodem do katalogu
projektu dla VisualDSP++ 5.0.

Dok³adne namiary:
Nazwa funkcji: sethostaddr()
Wywo³anie w: lwip_sysboot_threadtype.c, linia 116