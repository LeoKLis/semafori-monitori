#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

class Meduspremnik {
private:
    int velicina;
    int pisi_ptr, citaj_ptr;
    char *polje;
public:
    Meduspremnik(int velicina) {
        this->velicina = velicina;
        pisi_ptr = citaj_ptr = -1;
        polje = new char[velicina];
    }

    void pisi(char podatak) {
        if(pisi_ptr == -1){
            polje[0] = podatak;
            pisi_ptr = 1;
            citaj_ptr = 0;
        }
        else {
            polje[pisi_ptr] = podatak;
            pisi_ptr = (pisi_ptr + 1) % velicina;
            if(pisi_ptr == citaj_ptr) citaj_ptr = (citaj_ptr + 1) % velicina;
        }
    }

    char citaj() {
        if(citaj_ptr == -1){
            printf("Pogreska, polje je prazno [-1]...");
            return -1;
        }
        if(citaj_ptr == pisi_ptr) {
            printf("Pogreska, polje je prazno [c=p]...");
            return -1;
        }
        char podatak = polje[citaj_ptr];
        citaj_ptr = (citaj_ptr + 1) % velicina;
        return podatak;
    }

    bool prazan() {
        return citaj_ptr == pisi_ptr;
    }
};

// Tip ulaza je npr. temperatura, tlak, vlaga ili nes od senzora
// Podatak je vrijednost tog tipa
struct Ulaz {
    int tip;
    char podatak;
};

int ulazni_meduspr_vel;
int izlazni_meduspr_vel;

void dohvati_ulaz(struct Ulaz *ulaz){
    (*ulaz).tip = rand() % ulazni_meduspr_vel;
    (*ulaz).podatak = rand() % 26 + 97;    
}

void obradi_ulaz(struct Ulaz *ulaz) {
    // Hehe trosim cikluse
}

void *citaj_obradi_ulaz(void *arg) {
    struct Ulaz ulaz;
    while (1)
    {
        printf("delam");
        sleep(rand() % 6 + 5);
        dohvati_ulaz(&ulaz);
        obradi_ulaz(&ulaz);
    }
    
}

int main(int argc, char *argv[]) {
    if(argc != 4) {
        printf("Krivi broj argumenata [%d], treba 3. Izlazim...\n", argc);
        return 1;
    }

    int br_ulaznih_dretvi = atoi(argv[1]);
    int br_radnih_dretvi = atoi(argv[2]);
    int br_izlaznih_dretvi = atoi(argv[3]);

    ulazni_meduspr_vel = br_radnih_dretvi;
    izlazni_meduspr_vel = br_izlaznih_dretvi;

    Meduspremnik *ulazni_meduspremnik = (Meduspremnik*)malloc(sizeof(Meduspremnik) * ulazni_meduspr_vel);
    for (int i = 0; i < ulazni_meduspr_vel; i++)
    {
        ulazni_meduspremnik[i] = Meduspremnik(6);
    }
    Meduspremnik *izlazni_meduspremnik = (Meduspremnik*)malloc(sizeof(Meduspremnik) * izlazni_meduspr_vel);
    for (int i = 0; i < izlazni_meduspr_vel; i++)
    {
        izlazni_meduspremnik[i] = Meduspremnik(6);
    }
    

    pthread_t thread_id[br_ulaznih_dretvi];

    for (int i = 0; i < br_ulaznih_dretvi; i++)
    {
        pthread_create(&thread_id[i], NULL, &citaj_obradi_ulaz, NULL);
    }    

    //printf("Argumenti: %d, %d, %d\n", br_ulaznih_dretvi, br_radnih_dretvi, br_izlaznih_dretvi);

    return 0;
}