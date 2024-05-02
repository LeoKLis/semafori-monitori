#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct ulazni_meduspr
{
    int velicina;
};


void *citaj_obradi_ulaz() {
    while (1)
    {
        


        sleep(5);
    }
    
}

int main(int argc, char *argv[]) {
    if(argc != 4) {
        printf("Nedovoljan broj argumenata [%d]. Izlazim...\n", argc);
        return 1;
    }

    int br_ulaznih_dretvi = atoi(argv[1]);
    int br_radnih_dretvi = atoi(argv[2]);
    int br_izlaznih_dretvi = atoi(argv[3]);

    int ulazni_meduspr = br_radnih_dretvi;
    int izlazni_meduspr = br_izlaznih_dretvi;

    pthread_t thread_id[br_ulaznih_dretvi];

    for (int i = 0; i < br_ulaznih_dretvi; i++)
    {
        pthread_create(&thread_id[i], NULL, citaj_obradi_ulaz, NULL);
    }
    

    //printf("Argumenti: %d, %d, %d\n", br_ulaznih_dretvi, br_radnih_dretvi, br_izlaznih_dretvi);

    return 0;
}