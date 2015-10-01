int pinCom[20]; //array che contiene i pin di interesse per l'applicazione particolare
int cycle=0; //variabile di ciclo
int buff[64]; //buffer di lettura. il buffer integrato che raccoglie i dati trasmessi della seriale contiene 64 bit, perciò anche lui ne ha al max 64
int end_loop=0;
int minor=0; //per controllare se la lunghezza di un messaggio è minore o maggiore di quella minima o massima consentita dal protocollo, o compresa tra le due
int middle=0;
int major=0;
float a_old_mis[20];//analogica
float a_mis[20]; //analogica
int array_start=1; //serve a inizializzare old_mis in loop solo una volta
const float voltage=5.0; //tensione di alimentazione data da arduino
int sensor_voltage=0;
float f_voltage=0;
#include <Servo.h>
Servo servo; //oggetto servomotore
int sensor[20]; //array che dice se il pin i-esimo è un sensore
int att[20];//o un attuatore
int servoM[20];//e un servo (vanno dati i gradi, è specifica la trattazione)
int pin_array[20]; //array di ciclo per i for                                       //pinCom[], sensor[], att[] e servoM[] sono usati nel setup
int server_pin=-1; //pin deciso da server per fare cose (es. comando attuatore)
int grades=0; //gradi servo comandati da server

void setup() {
  Serial.begin(9600); 
}

//bisogna leggere la stringa in arrivo mettedo dentro un buffer i byte letti serialmente. 
//a quel punto si processa ciò che abbiamo messo nel buffer e si esegue l'azione corrispondente
//ho supposto messaggi codificati su '0___22 (23 byte) massimo
//se arduino non riconosce nessuno dei messaggi codificati negli if una volta che sono stati ricevuti 23 byte da port, allora va segnalato errore, e per prevenire il blocco va
//inserito del codice che riazzera il buffer di lettura (altrimenti si hanno sovrapposizioni imprevedibili nel buffer di lettura e nessun comando corrisponde più)

//la logica di gestione dei sensori e degli attuatori va scritta "separata" da quella per la ricezione di messaggi seriali da port, e quando si verificano certe condizioni
//arduino dovrà inviare a port le misurazioni

void loop() {

  end_loop=0;

  if(array_start==1){
    for(int i=0;i<20;i++){
      a_old_mis[i]=-1.0;//valore "illegale"
      //Serial.print(old_mis[i]); //di controllo
      a_mis[i]=0.0;
    }
  }

  while( Serial.available()>0 ){
    buff[cycle]=Serial.read(); 
    cycle++;
    //Serial.println(cycle); //di controllo
    delay(5); //!FONDAMENTALE!-->il MC di arduino con questo baud rate è più rapido della seriale, perciò senza il delay si potrebbe uscire dal while prima della fine della trasmissione
              //di una parola causando errore, ma è solo che il successivo byte non è arrivato in tempo! per sicurezza si può anche aumentare il delay, dipende da quanto è veloce 
              //port ad inviare i byte di messaggio
  }

  end_loop=1; //quando vale 1 significa che è stata trasmessa una parola, perciò nel controllo degli errori si entra nell'if solo se end_loop=1
  
  if( (cycle>0)and(cycle<3)){
    minor=1;
  }else minor=0;

  if( (cycle>3)and(cycle<23) ){
    middle=1;
  }else middle=0;

  if(cycle>23){
    major=1;
  }else major=0;

  if( (buff[0]=='@')&&(buff[1]=='p')&&(buff[22]=='#') ) { //è il messaggio di setup dei pin da tenere d'occhio (@p11111111111111111111#) (gli uni possono anche essere zeri)
    Serial.println("in arrivo setup dei pin dei sensori, tieni d'occhio i pin corrispondenti a:");
    
    for(int i=2;i<22;i++){ //riempiamo l'array dei pin da tenere d'occhio con ciò che abbiamo ricevuto da port, che è una lista di 20 uni e/o zeri. se c'è uno zero, non va tenuto d'occhio quel pin     
      pinCom[(i-2)]=buff[i]-48;     
      Serial.print(pinCom[i-2]); //stampa di controllo
    }
     
    for(int i=0;i<64;i++){//ripuliamo il buffer di lettura (23)
      buff[i]=0;
    }
        
    cycle=0; //rimettiamo a zero le variabili di ciclo
     
  }//if

  //messaggio "controlla questo/i pin e dammi il valore "
  if( (buff[0]=='@')&&(buff[1]=='d')&&(buff[22]=='#') ){
    Serial.println("Arduino, controlla questi pin e inviami la misurazione corrispondente:");
    
    for(int i=2;i<22;i++){      
      pin_array[(i-2)]=buff[i]-48;     
      //Serial.print(pin_array[i-2]); 
      sensor[i-2]=pin_array[i-2]; //questo pin è di sensore
      Serial.print(sensor[i-2]);
    }

    for(int i=0;i<64;i++){
      buff[i]=0;
    }
        
    cycle=0;
        
  }//if

  //acknowledge di port
  else if( (buff[0]=='@')&&(buff[1]=='a')&&(buff[2]=='#') ){
    Serial.println("messaggio di acknowledge da Port");
    for(int i=0;i<64;i++){ 
      buff[i]=0;
    }
        
    cycle=0; 
    
  }//if

  //messaggio "smetti di tenere d'occhio questo/i pin" (reset dell'applicazione)
  else if( (buff[0]=='@')&&(buff[1]=='s')&&(buff[22]=='#') ){
    Serial.println("Arduino, smetti di controllare i pin invitati");
    Serial.println("Adesso stai controllando questi pin");

    for(int i=2;i<22;i++){     
      if( (buff[i]-48)==1){
        pinCom[i-2]=0;
        sensor[i-2]=0; //questo pin non appartiene più a un sensore
        att[i-2]=0;
        servoM[i-2]=0;    
      }
      Serial.print(pinCom[i-2]); 
      //sensor[i-2]=0; //questo pin non appartiene più a un sensore
    }

    for(int i=0;i<64;i++){ 
      buff[i]=0;
    }
        
    cycle=0; 
    
  }//if

  //set up degli attuatori (il server sa su quali pin di arduino sono presenti attuatori, e se sono più di uno per attuatore)
  else if( (buff[0]=='@')&&(buff[1]=='t')&&(buff[22]=='#') ){
    Serial.println("in arrivo setup degli attuatori su questi pin:");
    
     for(int i=2;i<22;i++){      
      pin_array[(i-2)]=buff[i]-48;     
      att[i-2]=pin_array[i-2]; //questo pin è di attuatore
      Serial.print(att[i-2]);
    }

    for(int i=0;i<64;i++){ 
      buff[i]=0;
    }
        
    cycle=0; 
    
  }//if

  else if( (buff[0]=='@')&&(buff[1]=='m')&&(buff[22]=='#') ){ //se si usa un servo ci vuole un mess in più per le specificità
    Serial.println("questi pin appartengono a un servomotore:");
    
    for(int i=2;i<22;i++){      
      pin_array[(i-2)]=buff[i]-48;      
      servoM[i-2]=pin_array[i-2];
      Serial.print(servoM[i-2]);
    }

    for(int i=0;i<64;i++){ 
      buff[i]=0;
    }
        
    cycle=0; 
    
  }//if

  //se server gli manda @m9:30# arduino deve mandare un impulso al pin 9 dove sta il servo di 30 gradi
  else if( (buff[0]=='@')&&(buff[1]=='m')&&(buff[3]==':')&&(buff[6]=='#') ){ 
    Serial.println("Arduino, muovi il servo");
              
      server_pin=buff[2]-48; //legge il pin da comandare
      Serial.println(server_pin);
      servo.attach(server_pin);
      grades=((buff[4]-48)*10)+(buff[5]-48); //converte in decimale
      servo.write(grades); 
      delay(1000);
      grades=0;
      server_pin=0;
   
    for(int i=0;i<64;i++){ 
      buff[i]=0;
    }
    
  //todo altri comandi singoli bit tipo led, ecc  
        
    cycle=0; 
    
  }//if

  //sennò si è avuto errore
  else if( ( (end_loop==1)and(cycle==3) )or( (end_loop==1)and(cycle==23) )or(minor==1)or(middle==1)or(major==1) ){ 
    Serial.println("messaggio non riconosciuto");
    for(int i=0;i<64;i++){ 
      buff[i]=0;
    }
            
    cycle=0;
  }//if    fine fase di parsing dei messaggi seriali



  //lettura dei pin in esame
  for(int i=0;i<20;i++){
    if( (pinCom[i]==1)and(sensor[i]==1) ){ //se server gli ha detto di dargli la misurazione sul pin i-esimo
      //leggi il valore sul pin corrispondente con tipo analogread() ecc, e metti il valore letto in mis[i]
      //bisogna fare due routine separate per sensori digitali e analogici, perché in quelli analogici sono necessari calcoli per rendere la misurazione dipendente dal'alimentazione a 5V
      if( (i>=0)&&(i<6) ){ //pin analogici A0____A5 (6 pin)
        sensor_voltage=analogRead(i);
        a_mis[i]=sensor_voltage*voltage/1024; //gestione particolare della temperatura, caso specifico
        Serial.println( (a_mis[i]*1000-500)/10 );
        a_mis[i]=(a_mis[i]*1000-500)/10;
        delay(1500);
      }//if
      //qui if digitale (todo)
    }//fine if sensori

    for(int i=0;i<20;i++){  //converto a int per fare il confronto
      a_mis[i]=(int)(a_mis[i]);
      a_old_mis[i]=(int)(a_old_mis[i]);
    }

    //if attuatori digitali (todo analogici) 
    if( (att[i]==1)and(servoM[i]==1)and(pinCom[i]==1) ){ //se quel pin è di attuatore, in particolare un servo e server gli ha detto di controllarlo
      Serial.println("attuatore in funzione sul pin numero:");
      Serial.println(i);
      delay(1000);
      if(a_mis[0]>21){
        servo.attach(i);
        servo.write(25); //potrebbe indicarlo server questo valore
        delay(1000);
        servo.write(1);
      }
    }//if attuatori
  }//for
  
  //controllo array valori vecchi/nuovi
  for(int i=0;i<20;i++){ //attenzione, con questa tolleranza, se la temperatura sale di un grado per volta, anche se passasse da tipo 20 a 25 gradi arduino non notificherebbe volta volta!
    if( (a_old_mis[i]!=-1)and( (a_old_mis[i]<a_mis[i]-1)or(a_old_mis[i]>a_mis[i]+1)) ){ //problematico il confronto se sono due float
      Serial.println("il sensore ha rilevato un cambiamento!");//da sostituire con l'invio di mis[i] a port 
    }
  }

  for(int i=0;i<20;i++){
    a_old_mis[i]=a_mis[i]; //come detto sopra, se la temperatura sale o scende "dolcemente", anche se alla lunga varia di molto il server non viene notificato! occhio
  }
  
  array_start=0; //non re-inizializzare old_mis
}//loop
