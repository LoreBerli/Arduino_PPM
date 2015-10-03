# Arduino_PPM

Al momento una scheda equipaggiata con software arduino.ino è in grado di gestire:

-sensore di temperatura

-sensore di luminosità

-attuatore LED

-attuatore servomotore

I componenti possono essere in funzione anche contemporaneamente. Ho supposto un solo sensore di ogni tipo per Com.
I sensori, per il momento, sono stati supposti solo analogici (per un fatto di risparmio di memoria, ottimizzabile nelle fasi finali del progetto).

Set di comandi (un comando inizia per "@" e termina per "#")

-@pin:00#   (l'applicazione userà il pin 00)

-@sen:00#  (il pin 00 è un sensore)

-@tmp:00#  (il pin 00 è un sensore di temperatura)

-@lum:01# (il pin 01 è un sensore di luminosità)

-@led:05# (il pin 05 è un LED)

-@att:05# (il pin 05 è un attuatore)

-@a#  (ack da Port)

-@pinreq# (dimmi i pin che hai attivi al momento)

-@stp:00#  (smetti di utilizzare il pin 00)

-@lon:05# (accendi il LED sul pin 05)

-@lof:05# (spengi il LED sul pin 05)

-@m09:60#  (ruota di 60 gradi il servo sul pin 09)

-


