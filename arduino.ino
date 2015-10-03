//ho presupposto le misurazioni dei sensori solo analogiche per risparmiare memoria sugli array

int pinCom[20]; //array che contiene i pin di interesse per l'applicazione particolare
int cycle=0; //variabile di ciclo
int buff[64]; //buffer di lettura. il buffer integrato che raccoglie i dati trasmessi della seriale contiene 64 bit, perciò anche lui ne ha al max 64
int end_loop=0;
int minor=0; //per controllare se la lunghezza di un messaggio è minore o maggiore di quella minima o massima consentita dal protocollo, o compresa tra le due
int middle=0;
int major=0;
float a_old_misT[6];//analogica-temperatura   N.B. se volessimo misurare anche tramite i pin digitali sarebbe fino a 20
float a_misT[6]; //analogica
float a_old_misL[6];//analogica-luminosità
float a_misL[6]; //analogica
int array_start=1; //serve a inizializzare old_mis in loop solo una volta
const float voltage=5.0; //tensione di alimentazione data da arduino
int sensor_voltageL=0; //usato in lettura analogica
int sensor_voltageT=0;
float f_voltage=0;
#include <Servo.h>
Servo servo; //oggetto servomotore
int sensor[20]; //array che dice se il pin i-esimo è un sensore
int att[20];//o un attuatore
int servoM[20];//e un servo (vanno dati i gradi, è specifica la trattazione)
int pin_array[20]; //array di ciclo per i for                                       //pinCom[], sensor[], att[] e servoM[] sono usati nel setup
int server_pin=-1; //pin deciso da server per fare cose (es. comando attuatore)
int grades=0; //gradi servo comandati da server
int temperature[6]; /*se 1, indica che il sensore sul pin i-esimo è di temperatura (ciò comporta una trattazione specifica)-->comando @tmp:(pin-iesimo)#
                   N.B. ho supposto un solo sensore di ogni tipo per ogni scheda (es. max 1 sensore di temperatura, ecc) perché questo consente di non dover utilizzare un array di 20
                   variabili "tipo-del-sensore", ma ne basta una sola. la memoria è limitata!*/
int lum[6];
int led=0;
int led_pin=-1;
int pin_count=0; //conta il numero di pin attivi

void setup() {
  Serial.begin(9600); 
}

/*bisogna leggere la stringa in arrivo mettedo dentro un buffer i byte letti serialmente. 
a quel punto si processa ciò che abbiamo messo nel buffer e si esegue l'azione corrispondente
ho supposto messaggi codificati su 7 byte massimo
se arduino non riconosce nessuno dei messaggi codificati negli if una volta che sono stati ricevuti 7 byte da port allora va segnalato errore, e per prevenire il blocco va
inserito del codice che riazzera il buffer di lettura (altrimenti si hanno sovrapposizioni imprevedibili nel buffer di lettura e nessun comando corrisponde più)*/

void loop() {

  end_loop=0;

  if(array_start==1){
    for(int i=0;i<6;i++){
      a_old_misT[i]=-1.0;//valore "illegale"
      a_old_misL[i]=-1.0;
      //Serial.print(old_mis[i]); //di controllo
      a_misT[i]=0.0;
      a_misL[i]=0.0;
      temperature[i]=0;
      lum[i]=0;
    }
  }

  while( Serial.available()>0 ){
    buff[cycle]=Serial.read(); 
    cycle++;
    //Serial.println(cycle); //di controllo
    delay(5); /*!FONDAMENTALE!-->il MC di arduino con questo baud rate è più rapido della seriale, perciò senza il delay si potrebbe uscire dal while prima della fine della trasmissione
              di una parola causando errore, ma è solo che il successivo byte non è arrivato in tempo! per sicurezza si può anche aumentare il delay, dipende da quanto è veloce 
              port ad inviare i byte di messaggio*/
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

  //"il pin i-esimo verrà usato nell'applicazione"-->@pin:(pin i-esimo su due byte (uno per cifra))# esempio @pin:04#
  if( (buff[0]=='@')&&(buff[1]=='p')&&(buff[2]=='i')&&(buff[3]=='n')&&(buff[4]==':')&&(buff[7]=='#') ) { 
    Serial.println("Arduino, utilizza questo pin nell'applicazione:");    
    server_pin=((buff[5]-48)*10)+(buff[6]-48); //converte in decimale
    Serial.println(server_pin);
    pinCom[server_pin]=1; //pone a 1 la locazione corrispondente di pinCom  
    server_pin=0;  
     
    for(int i=0;i<64;i++){//ripuliamo il buffer di lettura (23)
      buff[i]=0;
    }
        
    cycle=0; //rimettiamo a zero le variabili di ciclo
     
  }//if

  //messaggio "dimmi i pin che stai controllando"  @pinreq#
  else if( (buff[0]=='@')&&(buff[1]=='p')&&(buff[2]=='i')&&(buff[3]=='n')&&(buff[4]=='r')&&(buff[5]=='e')&&(buff[6]=='q')&&(buff[7]=='#') ) { 
    Serial.println("Arduino, comunicami i pin che stai utilizzando:");    
    for(int i=0;i<20;i++){
      if(pinCom[i]==1){
        Serial.print("-");
        Serial.print(i);
        pin_count=1;
      }
    }
    if(pin_count==0){
        Serial.println("nessun pin collegato");
      }   
     
    for(int i=0;i<64;i++){//ripuliamo il buffer di lettura (23)
      buff[i]=0;
    }
        
    cycle=0; //rimettiamo a zero le variabili di ciclo
    pin_count=0;
     
  }//if

  //acknowledge di port
  else if( (buff[0]=='@')&&(buff[1]=='a')&&(buff[2]=='#') ){
    Serial.println("messaggio di acknowledge da Port");
    for(int i=0;i<64;i++){ 
      buff[i]=0;
    }
        
    cycle=0; 
    
  }//if

  //messaggio "smetti di tenere d'occhio questo pin"-->@stp:(pin i-esimo su 2 byte)#
  else if( (buff[0]=='@')&&(buff[1]=='s')&&(buff[2]=='t')&&(buff[3]=='p')&&(buff[4]==':')&&(buff[7]=='#') ){
    Serial.println("Arduino, smetti di controllare i pin inviati");
    Serial.println("Adesso stai controllando questi pin");
    server_pin=((buff[5]-48)*10)+(buff[6]-48); //converte in decimale
    temperature[server_pin]=0; //non c'è più un sensore di temperatura sulla scheda
    lum[server_pin]=0;
    pinCom[server_pin]=0; //pone a 1 la locazione corrispondente di pinCom 
    sensor[server_pin]=0;
    att[server_pin]=0;
    servoM[server_pin]=0;
    a_misT[server_pin]=0.0; 
    a_misL[server_pin]=0.0;
    for(int i=0;i<20;i++){
      Serial.print(pinCom[i]);
    }
    server_pin=0;  
                  
    for(int i=0;i<64;i++){ 
      buff[i]=0;
    }
        
    cycle=0; 
    
  }//if

  //messaggio "questo pin è un sensore"-->@sen:(pin su 2 byte)#
  else if( (buff[0]=='@')&&(buff[1]=='s')&&(buff[2]=='e')&&(buff[3]=='n')&&(buff[4]==':')&&(buff[7]=='#') ) { 
    Serial.println("Arduino, questo pin appartiene ad un sensore:");                            
    server_pin=((buff[5]-48)*10)+(buff[6]-48); //converte in decimale
    Serial.println(server_pin);
    sensor[server_pin]=1;
    pinMode(server_pin, INPUT);
    server_pin=0;
   
    for(int i=0;i<64;i++){ 
      buff[i]=0;
    } 
        
    cycle=0;
     
  }//if

  //set up degli attuatori-può servire per avere un attuatore sempre in funzioni (es. motore "rotante")-->@att:(pin su 2 byte)#
  else if( (buff[0]=='@')&&(buff[1]=='a')&&(buff[2]=='t')&&(buff[3]=='t')&&(buff[4]==':')&&(buff[7]=='#') ){
    Serial.println("questo pin appartiene a un attuatore:");                           
    server_pin=((buff[5]-48)*10)+(buff[6]-48); //converte in decimale
    Serial.println(server_pin);
    pinMode(server_pin, OUTPUT);
    att[server_pin]=1;
    server_pin=0;
   
    for(int i=0;i<64;i++){ 
      buff[i]=0;
    } 
        
    cycle=0; 
    
  }//if

  //può essere una specificazione di attuatore, come un servo-->@ser:(pin)#
  else if( (buff[0]=='@')&&(buff[1]=='s')&&(buff[2]=='e')&&(buff[3]=='r')&&(buff[4]==':')&&(buff[7]=='#') ){ //se si usa un servo ci vuole un mess in più per le specificità
    Serial.println("questo pin appartiene a un servomotore:");                         
    server_pin=((buff[5]-48)*10)+(buff[6]-48); //converte in decimale
    Serial.println(server_pin);
    servoM[server_pin]=1;
    server_pin=0;
   
    for(int i=0;i<64;i++){ 
      buff[i]=0;
    } 
        
    cycle=0;
    
  }//if

  //questo pin è un led  @led:num#
  else if( (buff[0]=='@')&&(buff[1]=='l')&&(buff[2]=='e')&&(buff[3]=='d')&&(buff[4]==':')&&(buff[7]=='#') ){ //se si usa un servo ci vuole un mess in più per le specificità
    Serial.println("questo pin appartiene a un led:");                         
    led_pin=((buff[5]-48)*10)+(buff[6]-48); //converte in decimale
    pinCom[led_pin]=1;
    Serial.println(led_pin);
    led=1;  //serve per sapere se c'è attivo un led sulla scheda, e quale è il pin (led_pin)
    pinMode(led_pin, OUTPUT);
    
    server_pin=0;
   
    for(int i=0;i<64;i++){ 
      buff[i]=0;
    } 
        
    cycle=0;
    
  }//if

  //accendi il led su questo pin  @lon:pin#
  else if( (buff[0]=='@')&&(buff[1]=='l')&&(buff[2]=='o')&&(buff[3]=='n')&&(buff[4]==':')&&(buff[7]=='#') ){ //se si usa un servo ci vuole un mess in più per le specificità
    Serial.println("accendi il led:");                         
    led_pin=((buff[5]-48)*10)+(buff[6]-48); //converte in decimale
    Serial.println(led_pin);
    pinMode(led_pin, OUTPUT);
    delay(50);
    digitalWrite(led_pin, HIGH);
    
    server_pin=0;
   
    for(int i=0;i<64;i++){ 
      buff[i]=0;
    } 
        
    cycle=0;
    
  }//if

  //spengi il led
  else if( (buff[0]=='@')&&(buff[1]=='l')&&(buff[2]=='o')&&(buff[3]=='f')&&(buff[4]==':')&&(buff[7]=='#') ){ //se si usa un servo ci vuole un mess in più per le specificità
    Serial.println("spengi il led:");                         
    led_pin=((buff[5]-48)*10)+(buff[6]-48); //converte in decimale
    Serial.println(led_pin);
    pinMode(led_pin, OUTPUT);
    led=0; //non ci sono più led
    digitalWrite(led_pin, LOW);
    led_pin=-1; //riportato al valore illegale
    
    server_pin=0;
   
    for(int i=0;i<64;i++){ 
      buff[i]=0;
    } 
        
    cycle=0;
    
  }//if
  
  //se server gli manda @m09:30# arduino deve mandare un impulso al pin 9 dove sta il servo di 30 gradi
  else if( (buff[0]=='@')&&(buff[1]=='m')&&(buff[4]==':')&&(buff[7]=='#') ){ 
    Serial.println("Arduino, muovi il servo");
              
      server_pin=((buff[2]-48)*10)+(buff[3]-48); //converte in decimale
      Serial.println(server_pin);
      servo.attach(server_pin);
      pinMode(server_pin, OUTPUT);
      grades=((buff[5]-48)*10)+(buff[6]-48); //converte in decimale
      servo.write(grades); 
      delay(1000);
      grades=0;
      server_pin=0;
   
    for(int i=0;i<64;i++){ 
      buff[i]=0;
    } 
        
    cycle=0; 
    
  }//if

  //se riceve @tmp:(pin i-esimo)# significa che quel pin è di un sensore di temperatura
  else if( (buff[0]=='@')&&(buff[1]=='t')&&(buff[2]=='m')&&(buff[3]=='p')&&(buff[4]==':')&&(buff[7]=='#') ){ 
    Serial.println("Arduino, il pin appartiene a un sensore di temperatura");
              
      server_pin=((buff[5]-48)*10)+(buff[6]-48); //converte in decimale
      Serial.println(server_pin);
      temperature[server_pin]=1;
      server_pin=0;
   
    for(int i=0;i<64;i++){ 
      buff[i]=0;
    }
    
  //todo altri comandi singoli bit tipo led, ecc  
        
    cycle=0; 
    
  }//if

  //sensore di luminosità (termoresistenza)  @lum:pin#
  else if( (buff[0]=='@')&&(buff[1]=='l')&&(buff[2]=='u')&&(buff[3]=='m')&&(buff[4]==':')&&(buff[7]=='#') ){ 
    Serial.println("Arduino, il pin appartiene a un sensore di luminosita'");
              
      server_pin=((buff[5]-48)*10)+(buff[6]-48); //converte in decimale
      Serial.println(server_pin);
      pinMode(server_pin, INPUT);
      lum[server_pin]=1;
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
      /*leggi il valore sul pin corrispondente con tipo analogread() ecc, e metti il valore letto in mis[i]
      bisogna fare due routine separate per sensori digitali e analogici, perché in quelli analogici sono necessari calcoli per rendere la misurazione dipendente dal'alimentazione a 5V*/
      if( (i>=0)&&(i<6) ){ //pin analogici A0____A5 (6 pin)       
        if(temperature[i]==1){ //temperature vale 1 finché server non comunica altrimenti. ho supposto 1 solo sensore per ogni tipo su ogni scheda (per cui basta una variabile tipodelsensore invece che un array!)
          sensor_voltageT=analogRead(i);
          a_misT[i]=sensor_voltageT*voltage/1024; //gestione particolare, caso specifico
          a_misT[i]=(a_misT[i]*1000-500)/10;
        }

        if(lum[i]==1){
          sensor_voltageL=analogRead(i);
          a_misL[i]=sensor_voltageL;
        }
        
        //todo altri if oltre a temperatura (luminosità, distanza, ecc)

        if(a_misT[i]!=0){
          Serial.print("misT: ");
          Serial.println(a_misT[i]);  
        } 

        if(a_misL[i]!=0){
          Serial.print("misL: ");
          Serial.println(a_misL[i]);
        }

        delay(500);  //500
      }//if
      //qui if digitale (todo)
    }//fine if sensori

    for(int i=0;i<6;i++){  //converto a int per fare il confronto (a causa dell'imprecisione dei float)
      a_misT[i]=(int)(a_misT[i]);
      a_old_misT[i]=(int)(a_old_misT[i]);
      
      a_misL[i]=(int)(a_misL[i]);
      a_old_misL[i]=(int)(a_old_misL[i]);
    }

    //if attuatori digitali (todo analogici) 
    if( (att[i]==1)and(servoM[i]==1)and(pinCom[i]==1) ){ //se quel pin è di attuatore, in particolare un servo e server gli ha detto di controllarlo
      Serial.println("attuatore in funzione sul pin numero:");
      Serial.println(i);
      delay(500);
    }//if 
    //todo if altri attuatori (led ecc)
  }//for
  
  //controllo array valori vecchi/nuovi
  for(int i=0;i<6;i++){ //attenzione, con questa tolleranza, se la temperatura sale di un grado per volta, anche se passasse da tipo 20 a 25 gradi arduino non notificherebbe volta volta!
    if( (a_old_misT[i]!=-1)and( (a_old_misT[i]<a_misT[i]-1)or(a_old_misT[i]>a_misT[i]+1)) ){ //problematico il confronto se sono due float
      Serial.println("il sensore ha rilevato un cambiamento!");//da sostituire con l'invio di mis[i] a port 
    }

    if( (a_old_misL[i]!=-1)and( (a_old_misL[i]<a_misL[i]-1)or(a_old_misL[i]>a_misL[i]+1)) ){ //problematico il confronto se sono due float
      Serial.println("il sensore ha rilevato un cambiamento!");//da sostituire con l'invio di mis[i] a port 
    }
  }

  for(int i=0;i<6;i++){
    a_old_misT[i]=a_misT[i]; //come detto sopra, se la temperatura sale o scende "dolcemente", anche se alla lunga varia di molto il server non viene notificato! occhio
    a_old_misL[i]=a_misL[i];
  }
  
  array_start=0; //non re-inizializzare old_mis
}//loop
