To jest wersja projektu Inetd-BF537 przygotowana do wsp�pracy
z VisualDSP++ 4.5

Poniewa� jednak linker wersji 4.5 nie mo�e odnale�� definicji
jednej z kluczowych funkcji, jej wywo�anie zosta�o zakomentowane.
Trzeba to jako� poprawi� przed wgraniemprojektu na p�ytk�.
By� mo�e zadzia�a zwyczajne przekopiowanie plik�w z kodem do katalogu
projektu dla VisualDSP++ 5.0.

Dok�adne namiary:
Nazwa funkcji: sethostaddr()
Wywo�anie w: lwip_sysboot_threadtype.c, linia 116