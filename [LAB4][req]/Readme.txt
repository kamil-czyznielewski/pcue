Na co nale¿y zwróciæ uwagê w rozwi¹zaniu:

* Z ró¿nych plików pousuwano wszystko co wi¹¿e siê z obs³ug¹ przetwornika AC i diod.
* W Serial.c zmieniono funkcje obs³ugi znaków na nie blokuj¹ce (zwrracaj¹ 0 jak nic nie odebra³y)
* W Serial.c zmieniono funkcjê inicjalizacji UARTów tak, by inicjalizowane by³y oba na raz.
* W Serial.c dodano funkcje wypisywania ca³ych stringów UARTami 0 i 1
* Zmieniono Retarget.c tak, by wszystkie *printfy wypisywa³y wynik na UART1
* Rozwi¹zanie z Blinky.c jest oparte na tym co dzia³a³o, ale dziêki [http://demotywatory.pl/3145043/] nigdy
  nie opuœci³o laboratorium. Parê rzeczy trzeba posprawdzaæ, ale nie da siê tego zrobiæ bez sprzêtu.