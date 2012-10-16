¯eby ³atwo da³o siê przekleiæ pliki z kodem do projektu dla
VisualDSP++ 5.0, implementacja serwera us³ugi ping-pong znajduje
siê w miejscu implementacji us³ugi echo.

Us³uga ping-pong sprowadza siê do odbicia pi³ki rzuconej w serwer.
Na ka¿de odebrane "ping" serwer odpowiada "pong". Inne komendy s¹
przez niego ignorowane.

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