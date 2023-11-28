Università degli Studi di Torino - Dipartimento di Informatica 

Progetto di Sistemi Operativi [ version NORMAL ] - 2022/2023 

Autori: Simone Milanesio - Rachele Musso


[ AVVIO PROGETTO ]

Inserire i seguenti comandi da terminale: 

 - source config/config.sh 
 - make run 




[ DUMP GIORNALIERO ]

Al trascorrere di ogni giorno, deve essere visualizzato un report provvisorio contenente:

  • Totale delle merci suddivise per tipologia e stato (disponibile, consegnato, etc. Si veda la Sezione 5.1 per la
    descrizione dello stato di una merce)
  • Numero di navi:
    – in mare con un carico a bordo,
    – in mare senza un carico,
    – in porto, facendo operazioni di carico/scarico.
  • Per ogni porto si indichi
    – la quantità di merce presente, spedita, e ricevuta
    – il numero di banchine occupate/totali


[ DUMP FINALE ]

La simulazione termina in una delle seguenti circostanze
• dopo un tempo simulato di SO_DAYS
• quando per ogni tipo di merce
– l’offerta `e pari a zero oppure
– la richiesta `e pari a zero.
Il report finale deve indicare:
• Numero di navi ancora in mare con un carico a bordo
• Numero di navi ancora in mare senza un carico
• Numero di navi che occupano una banchina
• Totale delle merci suddivise per tipologia e stato (disponibile, consegnato, etc. Si veda la Sezione 5.1 per la
descrizione dello stato di una merce)
• Per ogni porto si indichi la quantit`a di merce
– presente, spedita, e ricevuta.
5
• Per ogni tipo di merce si indichi:
– la quantit`a totale generata dall’inizio della simulazione e quanta di essa
∗ `e rimasta ferma in porto
∗ `e scaduta in porto
∗ `e scaduta in nave
∗ `e stata consegnata da qualche nave.
– Si indichi il porto che
∗ ha offerto la quantit`a maggiore della merce
∗ e quello che ha richiesto la quantit`a maggiore di merce