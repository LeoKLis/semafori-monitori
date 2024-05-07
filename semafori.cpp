// Ulazi, izlazi i sve funkcije su relativne za tipove dretvi
// Ako je dretva ulazna, onda je ulazni_meduspr = null, a izlazni je ulazni_meduspremnik
// Ako je dretva radna, onda je ulazni_meduspr - ulazni_meduspremnik[i], a izlazni je izlazni_meduspremnik

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <memory>
#include <semaphore.h>

int ulazni_meduspr_br;
int izlazni_meduspr_br;

// Kruzni spremnik u koji se spremaju podaci tj. ulazi
class Meduspremnik
{
private:
    int velicina;
    int pisi_ptr, citaj_ptr;
    char *polje;

public:
    Meduspremnik(int velicina)
    {
        this->velicina = velicina;
        pisi_ptr = citaj_ptr = -1;
        polje = new char[velicina];
        for (int i = 0; i < velicina; i++)
        {
            polje[i] = 45;
        }
        
    }

    void pisi(char podatak)
    {
        if (pisi_ptr == -1)
        {
            polje[0] = podatak;
            pisi_ptr = 1;
            citaj_ptr = 0;
        }
        else
        {
            polje[pisi_ptr] = podatak;
            if (pisi_ptr == citaj_ptr && polje[citaj_ptr + 1] != 45)
                citaj_ptr = (citaj_ptr + 1) % velicina;
            pisi_ptr = (pisi_ptr + 1) % velicina;
        }
    }

    char citaj()
    {
        if (citaj_ptr == -1)
        {
            printf("Pogreska, polje je prazno [-1]...\n");
            return 23;
        }
        if(polje[citaj_ptr] < 97 || polje[citaj_ptr] > 122){
            printf("Pogreska, polje je prazno [-1]...\n");
            return 23;
        }
        char podatak = polje[citaj_ptr];
        polje[citaj_ptr] = 45;
        citaj_ptr = (citaj_ptr + 1) % velicina;
        return podatak;
    }

    bool prazan()
    {
        return polje[citaj_ptr] == 45;
    }

    void print()
    {
        for (int i = 0; i < velicina; i++)
        {
            printf("%c", polje[i]);
        }
    }
};
// Tip ulaza je npr. temperatura, tlak, vlaga ili nes od senzora
// Podatak je vrijednost tog tipa
struct Ulaz {
    int tip;
    char podatak;
};

// Nazivi ulazni i izlazni meduspremnik su relativni za tip dretve
struct dretve_args {
    Meduspremnik *ulazni_meduspr;
    Meduspremnik *izlazni_meduspr;
    int dretva_id;
    int dretva_tip;
};

void ispis_meduspremnika(Meduspremnik *ulazni, Meduspremnik *izlazni) {
    printf("UMS: ");
    for (int i = 0; i < ulazni_meduspr_br; i++)
    {
        ulazni->print();
        printf(" ");
    }
    printf("\n");
    printf("IMS: ");
    for (int i = 0; i < izlazni_meduspr_br; i++)
    {
        izlazni->print();
        printf(" ");
    }
    printf("\n");
    
}

void dohvati_senzor(struct Ulaz *ulaz){
    sleep(rand() % 6 + 5);
    (*ulaz).tip = rand() % ulazni_meduspr_br;
    (*ulaz).podatak = rand() % 26 + 97;    
}

void obradi_ulaz(struct Ulaz *ulaz, Meduspremnik *meduspr) {
    sleep(rand() % 2 + 2);
    (*ulaz).podatak += 1;
    (*ulaz).podatak -= 1;
    meduspr[(*ulaz).tip].pisi((*ulaz).podatak);
    // ulazni_meduspr[(*ulaz).tip].print(); printf("\n");
}

void dohvati_ulaz(struct Ulaz *ulaz, Meduspremnik *ulazni_meduspr) {
    (*ulaz).tip = rand() % izlazni_meduspr_br;
    (*ulaz).podatak = ulazni_meduspr->citaj();
}

void *citaj_obradi_ulazna(void *arg) {
    struct Ulaz ulaz;
    std::unique_ptr<dretve_args> wrp( static_cast<dretve_args*>(arg) );
    while (1)
    {
        dohvati_senzor(&ulaz);
        obradi_ulaz(&ulaz, wrp->izlazni_meduspr);
        printf("Ulazna dretva %d: dohvati %c i stavi u meduspremnik %d", wrp->dretva_id, ulaz.podatak, ulaz.tip);
        ispis_meduspremnika(wrp->ulazni_meduspr, wrp->izlazni_meduspr);
    }
    pthread_exit(NULL);
}

void *citaj_obradi_radna(void *arg) {
    struct Ulaz ulaz;
    std::unique_ptr<dretve_args> wrp( static_cast<dretve_args*>(arg) );
    while (1)
    {
        if(!wrp->ulazni_meduspr[wrp->dretva_id].prazan()){
            dohvati_ulaz(&ulaz, &wrp->ulazni_meduspr[wrp->dretva_id]);
            obradi_ulaz(&ulaz, wrp->izlazni_meduspr);
            wrp->izlazni_meduspr[ulaz.tip].pisi(ulaz.podatak);
            printf("Radna dretva %d: dohvati %c i stavi u meduspremnik %d", wrp->dretva_id, ulaz.podatak, ulaz.tip);
            ispis_meduspremnika(wrp->ulazni_meduspr, wrp->izlazni_meduspr);
        }
    }
    pthread_exit(NULL);
}

void *citaj_obradi_izlazna(void *arg) {
    struct Ulaz ulaz;
    std::unique_ptr<dretve_args> wrp( static_cast<dretve_args*>(arg) );
    while (1)
    {
        if(!wrp->ulazni_meduspr[wrp->dretva_id].prazan()) {
            sleep(rand() % 2 + 2);
            dohvati_ulaz(&ulaz, &wrp->ulazni_meduspr[wrp->dretva_id]);
            printf("Izlazna dretva %d: dohvati i ispisi %c", wrp->dretva_id, ulaz.podatak);
            ispis_meduspremnika(wrp->ulazni_meduspr, wrp->izlazni_meduspr);
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if(argc != 4) {
        printf("Krivi broj argumenata [%d], treba 3. Izlazim...\n", argc);
        return 1;
    }


    int br_ulaznih_dretvi = atoi(argv[1]);
    int br_radnih_dretvi = atoi(argv[2]);
    int br_izlaznih_dretvi = atoi(argv[3]);
    ulazni_meduspr_br = br_radnih_dretvi;
    izlazni_meduspr_br = br_izlaznih_dretvi;


    Meduspremnik *ulazni_meduspremnik = (Meduspremnik*)malloc(sizeof(Meduspremnik) * ulazni_meduspr_br);
    for (int i = 0; i < ulazni_meduspr_br; i++)
    {
        ulazni_meduspremnik[i] = Meduspremnik(6);
    }
    Meduspremnik *izlazni_meduspremnik = (Meduspremnik*)malloc(sizeof(Meduspremnik) * izlazni_meduspr_br);
    for (int i = 0; i < izlazni_meduspr_br; i++)
    {
        izlazni_meduspremnik[i] = Meduspremnik(6);
    }
    printf("Pocetak:\n");
    ispis_meduspremnika(ulazni_meduspremnik, izlazni_meduspremnik);

    sem_t ulazni_semafor[ulazni_meduspr_br];
    sem_t izlazni_semafor[izlazni_meduspr_br];

    pthread_t ulazne_dretve[br_ulaznih_dretvi];
    for (int i = 0; i < br_ulaznih_dretvi; i++)
    {
        struct dretve_args *args;
        args = (dretve_args*)malloc(sizeof(dretve_args));
        args->dretva_id = i;
        args->dretva_tip = 0;
        args->ulazni_meduspr = ulazni_meduspremnik;
        args->izlazni_meduspr = izlazni_meduspremnik;
        pthread_create(&ulazne_dretve[i], NULL, &citaj_obradi_ulazna, args);
    }  
    pthread_t radne_dretve[br_radnih_dretvi];
    for (int i = 0; i < br_ulaznih_dretvi; i++)
    {
        struct dretve_args *args;
        args = (dretve_args*)malloc(sizeof(dretve_args));\
        args->dretva_id = i;
        args->dretva_tip = 1;
        args->ulazni_meduspr = ulazni_meduspremnik;
        args->izlazni_meduspr = izlazni_meduspremnik;
        pthread_create(&radne_dretve[i], NULL, &citaj_obradi_radna, args);
    }    
    pthread_t izlazne_dretve[br_izlaznih_dretvi];
    for (int i = 0; i < br_izlaznih_dretvi; i++)
    {
        struct dretve_args *args;
        args = (dretve_args*)malloc(sizeof(dretve_args));
        args->dretva_id = i;
        args->dretva_tip = 2;
        args->ulazni_meduspr = ulazni_meduspremnik;
        args->izlazni_meduspr = izlazni_meduspremnik;
        args->izlazni_meduspr = nullptr;
        pthread_create(&izlazne_dretve[i], NULL, &citaj_obradi_izlazna, args);
    }
    

    for (int i = 0; i < br_ulaznih_dretvi; i++)
    {
        pthread_join(ulazne_dretve[i], NULL);
    }
    for (int i = 0; i < br_ulaznih_dretvi; i++)
    {
        pthread_join(radne_dretve[i], NULL);
    }
    for (int i = 0; i < br_izlaznih_dretvi; i++)
    {
        pthread_join(izlazne_dretve[i], NULL);
    }
    

    return 0;
}