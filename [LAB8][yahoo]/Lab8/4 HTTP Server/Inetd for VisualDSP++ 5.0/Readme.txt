Celem zadania jest stworzenie mechanizmu, kt�ry przez HTTP
pozwoli sterowa� diodami na p�ytce.

Implementacja serwera jest w pliku Echo_Worker_ThreadType.c.
Dodatkowo port nas�uchu dla w�tku dawniej obs�uguj�cego us�ug�
echo mo�na zmieni� w pliku Echo_Server_ThreadType.c, w linii 51.
Obecnie jest on ustawiony na 80, tj. odpowiedni dla HTTP.
W Echo_Server_ThreadType.c w funkcji inicjalizuj�cej serwer
znajduj� si� polecenia inicjalizuj�ce diody.

Implementacja serwera us�ugi http znajduje si� w miejscu
implementacji us�ugi echo. Jedyn� r�nic� jest zmiana portu
serwera z 7 na 80.

Ze swojej strony polecam zakomentowanie printf�w wypisuj�cych,
�e nawi�zano lub zerwano po��czenie.