Celem zadania jest stworzenie mechanizmu, który przez HTTP
pozwoli sterowaæ diodami na p³ytce.

Implementacja serwera jest w pliku Echo_Worker_ThreadType.c.
Dodatkowo port nas³uchu dla w¹tku dawniej obs³uguj¹cego us³ugê
echo mo¿na zmieniæ w pliku Echo_Server_ThreadType.c, w linii 51.
Obecnie jest on ustawiony na 80, tj. odpowiedni dla HTTP.
W Echo_Server_ThreadType.c w funkcji inicjalizuj¹cej serwer
znajduj¹ siê polecenia inicjalizuj¹ce diody.

Implementacja serwera us³ugi http znajduje siê w miejscu
implementacji us³ugi echo. Jedyn¹ ró¿nic¹ jest zmiana portu
serwera z 7 na 80.

Ze swojej strony polecam zakomentowanie printfów wypisuj¹cych,
¿e nawi¹zano lub zerwano po³¹czenie.