�eby �atwo da�o si� przeklei� pliki z kodem do projektu dla
VisualDSP++ 5.0, implementacja serwera us�ugi ping-pong znajduje
si� w miejscu implementacji us�ugi echo.

Us�uga ping-pong sprowadza si� do odbicia pi�ki rzuconej w serwer.
Na ka�de odebrane "ping" serwer odpowiada "pong". Inne komendy s�
przez niego ignorowane.

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