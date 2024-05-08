// Ulazi i izlazi su apsolutni za sve vrijednosti!! Ne vise relativni

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <memory>
#include <semaphore.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

int ulazni_meduspr_br;
int izlazni_meduspr_br;

// Kruzni spremnik u koji se spremaju podaci tj. ulazi
class Meduspremnik
{
private:
    int velicina;
    int pisi_ptr, citaj_ptr;
    char *polje = (char*) malloc(1);

public:
    Meduspremnik(){}
    /* Meduspremnik(int velicina)
    {
        this->velicina = velicina;
        pisi_ptr = citaj_ptr = -1;
        polje = new char[velicina];
        for (int i = 0; i < velicina; i++)
        {
            polje[i] = 45;
        }
    } */

    void Init(int velicina){
        this->velicina = velicina;
        pisi_ptr = -1; 
        citaj_ptr = -1;
        polje = (char*) realloc(polje, velicina);
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
            if (pisi_ptr == citaj_ptr && polje[(citaj_ptr + 1) % velicina] != 45)
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
    sem_t *ulazni_bsem;
    sem_t *izlazni_bsem;
    sem_t *ulazni_osem;
    sem_t *izlazni_osem;
};

void ispis_meduspremnika(Meduspremnik *ulazni, Meduspremnik *izlazni) {
    printf("UMS: ");
    for (int i = 0; i < ulazni_meduspr_br; i++)
    {
        ulazni[i].print();
        printf(" ");
    }
    printf("\n");
    printf("IMS: ");
    for (int i = 0; i < izlazni_meduspr_br; i++)
    {
        izlazni[i].print();
        printf(" ");
    }
    printf("\n\n");
    
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
}

void dohvati_ulaz(struct Ulaz *ulaz, Meduspremnik *ulazni_meduspr, int dretva_id, int tip) {
    (*ulaz).tip = rand() % izlazni_meduspr_br;
    (*ulaz).podatak = ulazni_meduspr->citaj();
    if((*ulaz).podatak == 23){
        printf("Dretva id: %d, Dretva tip: %d\n", dretva_id, tip);
    }
}

void *citaj_obradi_ulazna(void *arg) {
    struct Ulaz ulaz;
    std::unique_ptr<dretve_args> wrp( static_cast<dretve_args*>(arg) );
    while (1)
    {
        dohvati_senzor(&ulaz);
        sem_wait(&wrp->ulazni_bsem[ulaz.tip]);
        obradi_ulaz(&ulaz, wrp->ulazni_meduspr);
        printf(ANSI_COLOR_BLUE "Ulazna dretva %d dohvaca %c i stavlja u meduspremnik %d" ANSI_COLOR_RESET "\n", wrp->dretva_id, ulaz.podatak, ulaz.tip);
        ispis_meduspremnika(wrp->ulazni_meduspr, wrp->izlazni_meduspr);
        sem_post(&wrp->ulazni_bsem[ulaz.tip]);
        sem_post(&wrp->ulazni_osem[ulaz.tip]);
    }
    pthread_exit(NULL);
}

void *citaj_obradi_radna(void *arg) {
    struct Ulaz ulaz;
    std::unique_ptr<dretve_args> wrp( static_cast<dretve_args*>(arg) );
    while (1)
    {
        sem_wait(&wrp->ulazni_osem[wrp->dretva_id]);
        sem_wait(&wrp->ulazni_bsem[wrp->dretva_id]);
        dohvati_ulaz(&ulaz, &wrp->ulazni_meduspr[wrp->dretva_id], wrp->dretva_id, wrp->dretva_tip);
        printf(ANSI_COLOR_RED "Radna dretva %d dohvaca %c iz meduspremnika %d" ANSI_COLOR_RESET "\n", wrp->dretva_id, ulaz.podatak, wrp->dretva_id);
        ispis_meduspremnika(wrp->ulazni_meduspr, wrp->izlazni_meduspr);
        sem_post(&wrp->ulazni_bsem[wrp->dretva_id]);

        sem_wait(&wrp->izlazni_bsem[ulaz.tip]);
        obradi_ulaz(&ulaz, wrp->izlazni_meduspr);
        printf(ANSI_COLOR_RED "Radna dretva %d stavlja %c u meduspremnik %d" ANSI_COLOR_RESET "\n", wrp->dretva_id, ulaz.podatak, ulaz.tip);
        ispis_meduspremnika(wrp->ulazni_meduspr, wrp->izlazni_meduspr);
        sem_post(&wrp->izlazni_bsem[ulaz.tip]);
        sem_post(&wrp->izlazni_osem[ulaz.tip]);
    }
    pthread_exit(NULL);
}

void *citaj_obradi_izlazna(void *arg) {
    struct Ulaz ulaz;
    std::unique_ptr<dretve_args> wrp( static_cast<dretve_args*>(arg) );
    while (1)
    {
        sem_wait(&wrp->izlazni_osem[wrp->dretva_id]);      
        sem_wait(&wrp->izlazni_bsem[wrp->dretva_id]);  
        dohvati_ulaz(&ulaz, &wrp->izlazni_meduspr[wrp->dretva_id], wrp->dretva_id, wrp->dretva_tip);
        printf(ANSI_COLOR_GREEN "Izlazna dretva %d dohvaca %c iz meduspremnika %d" ANSI_COLOR_RESET "\n", wrp->dretva_id, ulaz.podatak, wrp->dretva_id);
        ispis_meduspremnika(wrp->ulazni_meduspr, wrp->izlazni_meduspr);
        sem_post(&wrp->izlazni_bsem[wrp->dretva_id]);

        sleep(rand() % 2 + 2);
        printf(ANSI_COLOR_GREEN "Izlazna dretva %d ispisuje %c" ANSI_COLOR_RESET "\n", wrp->dretva_id, ulaz.podatak);
        ispis_meduspremnika(wrp->ulazni_meduspr, wrp->izlazni_meduspr);
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
    int velicina_spremnika = 6;

    Meduspremnik ulazni_meduspremnik[ulazni_meduspr_br];
    for (int i = 0; i < ulazni_meduspr_br; i++)
    {
        ulazni_meduspremnik[i].Init(velicina_spremnika);
    }
    Meduspremnik izlazni_meduspremnik[izlazni_meduspr_br];
    for (int i = 0; i < izlazni_meduspr_br; i++)
    {
        izlazni_meduspremnik[i].Init(velicina_spremnika);
    }

    printf("Pocetak:\n");
    ispis_meduspremnika(ulazni_meduspremnik, izlazni_meduspremnik);


    sem_t ulazni_bsem[ulazni_meduspr_br];
    sem_t izlazni_bsem[izlazni_meduspr_br];
    sem_t ulazni_osem[ulazni_meduspr_br];
    sem_t izlazni_osem[izlazni_meduspr_br];
    for (int i = 0; i < ulazni_meduspr_br; i++)
    {
        sem_init(&ulazni_bsem[i], 0, 1);
        sem_init(&ulazni_osem[i], 0, 0);
    }
    for (int i = 0; i < izlazni_meduspr_br; i++)
    {
        sem_init(&izlazni_bsem[i], 0, 1);
        sem_init(&izlazni_osem[i], 0, 0);
    }
    
    struct dretve_args *args;
    pthread_t ulazne_dretve[br_ulaznih_dretvi];
    for (int i = 0; i < br_ulaznih_dretvi; i++)
    {
        args = new dretve_args;
        args->dretva_id = i;
        args->dretva_tip = 0;
        args->ulazni_meduspr = ulazni_meduspremnik;
        args->izlazni_meduspr = izlazni_meduspremnik;
        args->ulazni_bsem = ulazni_bsem;
        args->ulazni_osem = ulazni_osem;
        pthread_create(&ulazne_dretve[i], NULL, &citaj_obradi_ulazna, args);
    }
    sleep(15);
    pthread_t radne_dretve[br_radnih_dretvi];
    for (int i = 0; i < br_radnih_dretvi; i++)
    {
        args = new dretve_args;
        args->dretva_id = i;
        args->dretva_tip = 1;
        args->ulazni_meduspr = ulazni_meduspremnik;
        args->izlazni_meduspr = izlazni_meduspremnik;
        args->ulazni_bsem = ulazni_bsem;
        args->izlazni_bsem = izlazni_bsem;
        args->ulazni_osem = ulazni_osem;
        args->izlazni_osem = izlazni_osem;
        pthread_create(&radne_dretve[i], NULL, &citaj_obradi_radna, args);
    }    
    pthread_t izlazne_dretve[br_izlaznih_dretvi];
    for (int i = 0; i < br_izlaznih_dretvi; i++)
    {
        args = new dretve_args;
        args->dretva_id = i;
        args->dretva_tip = 2;
        args->ulazni_meduspr = ulazni_meduspremnik;
        args->izlazni_meduspr = izlazni_meduspremnik;
        args->izlazni_bsem = izlazni_bsem;
        args->izlazni_osem = izlazni_osem;
        pthread_create(&izlazne_dretve[i], NULL, &citaj_obradi_izlazna, args);
    }
    

    for (int i = 0; i < br_ulaznih_dretvi; i++)
    {
        pthread_join(ulazne_dretve[i], NULL);
    }
    for (int i = 0; i < br_radnih_dretvi; i++)
    {
        pthread_join(radne_dretve[i], NULL);
    }
    for (int i = 0; i < br_izlaznih_dretvi; i++)
    {
        pthread_join(izlazne_dretve[i], NULL);
    }
    

    return 0;
}