Na co nale�y zwr�ci� uwag� w rozwi�zaniu:

* Z r�nych plik�w pousuwano wszystko co wi��e si� z obs�ug� przetwornika AC i diod.
* W Serial.c zmieniono funkcje obs�ugi znak�w na nie blokuj�ce (zwrracaj� 0 jak nic nie odebra�y)
* W Serial.c zmieniono funkcj� inicjalizacji UART�w tak, by inicjalizowane by�y oba na raz.
* W Serial.c dodano funkcje wypisywania ca�ych string�w UARTami 0 i 1
* Zmieniono Retarget.c tak, by wszystkie *printfy wypisywa�y wynik na UART1
* Rozwi�zanie z Blinky.c jest oparte na tym co dzia�a�o, ale dzi�ki [http://demotywatory.pl/3145043/] nigdy
  nie opu�ci�o laboratorium. Par� rzeczy trzeba posprawdza�, ale nie da si� tego zrobi� bez sprz�tu.