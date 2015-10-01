# Arduino_PPM

Il codice non è ancora abbastanza generico da permettere il settaggio completo della scheda da parte di Port.
Arduino al momento riconosce un set limitato di comandi.
Per testarne il funzionamento si legga quanto segue.
Esempio di dialogo e setup:
server per prima cosa invia un comando @p-------------# per indicare i pin in uso nell'applicazione.
in seguito specifica quali sono di sensore, quali di attuatore, e tra questi quali di servomotore e altre specificità.
di conseguenza arduino ha l'array pinCom[] che contiene, per esempio, 6 pin a 1 e 14 a 0. di questi 6, 4
sono di sensori e due di attuatori, perciò sensor[] ha 4 bit a 1 e att[] ne ha 2. dei due attuatori, mettiamo che
uno sia un servo, per cui servoM[] ha 1 bit a uno.

#Se voglio attivare un sensore sul pin 0 e un attuatore servomotore sul pin 9 devo:
#inviare @p10000000010000000000#  --->pin di interesse (attuatori e sensori)
#inviare @d10000000000000000000#  --->sensor[] ha1 pin a uno (lo zero)
#inviare @t00000000010000000000#  --->att[] ha 1 pin a uno (il 9)
#inviare @m00000000010000000000#  --->servoM[] ha 1 pin a uno (il 9)

#volendo, si può muovere il servo a piacere inviando, ad esempio, il comando: @m9:30# (se si volesse muovere di 30 gradi il servo sul pin 9)

