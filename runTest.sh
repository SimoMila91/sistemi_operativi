source config/config.sh


echo "\n\n\n"

for i in  1 2 3 4 5 6 
do 
    echo "SIMULAZIONE $i..."
    make run -> log$i.txt
    echo "Done!\n"
done

echo "LA CONFIGURAZIONE 'Custom' SEMBRA FUNZIONARE\n\n\n"



