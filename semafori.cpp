// VS Code format: {BasedOnStyle: Google, UseTab: Never, IndentWidth: 8,
// TabWidth: 8, CloumnLimit: 80, BreakBeforeBraces: Stroustrup,
// AllowShortIfStatementsOnASingleLine: false}

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_RESET "\x1b[0m"

int broj_ulaznih_meduspremnika;
int broj_izlaznih_meduspremnika;

// Meduspremnik je struktura koja sprema podatke
// Spremanje se odvija kruzno (slicno cirkularnom redu)
class Meduspremnik {
       private:
        int velicina;
        int pisi_ptr, citaj_ptr;
        char *polje = (char *)malloc(1);

       public:
        Meduspremnik() {}

        void Init(int velicina)
        {
                this->velicina = velicina;
                pisi_ptr = -1;
                citaj_ptr = -1;
                polje = (char *)realloc(polje, velicina);
                for (int i = 0; i < velicina; i++) {
                        polje[i] = 45;
                }
        }

        void pisi(char podatak)
        {
                if (pisi_ptr == -1) {
                        polje[0] = podatak;
                        pisi_ptr = 1;
                        citaj_ptr = 0;
                }
                else {
                        polje[pisi_ptr] = podatak;
                        if (pisi_ptr == citaj_ptr &&
                            polje[(citaj_ptr + 1) % velicina] != 45)
                                citaj_ptr = (citaj_ptr + 1) % velicina;
                        pisi_ptr = (pisi_ptr + 1) % velicina;
                }
        }

        char citaj()
        {
                if (citaj_ptr == -1) {
                        printf("Pogreska, polje je prazno [-1]...\n");
                        return 23;
                }
                if (polje[citaj_ptr] < 97 || polje[citaj_ptr] > 122) {
                        printf("Pogreska, polje je prazno [-1]...\n");
                        return 23;
                }
                char podatak = polje[citaj_ptr];
                polje[citaj_ptr] = 45;
                citaj_ptr = (citaj_ptr + 1) % velicina;
                return podatak;
        }

        bool prazan() { return polje[citaj_ptr] == 45; }

        void print()
        {
                for (int i = 0; i < velicina; i++) {
                        printf("%c", polje[i]);
                }
        }
};
// Ulaz je struktura podatka koji dretve citaju i obraduju
// Tip ulaza je npr. temperatura, tlak, vlaga, ...
// Podatak je vrijednost tog tipa (char u mojem slucaju)
struct Ulaz {
        int tip_informacije;
        char podatak;
};

struct Dretve_args {
        Meduspremnik *ulazni_meduspremnik, *izlazni_meduspremnik;
        int dretva_id, dretva_tip;
        sem_t *ulazni_bsem, *izlazni_bsem, *ulazni_osem, *izlazni_osem;

        Dretve_args(int dretva_id, int dretva_tip, Meduspremnik *ulazni_meduspr,
                    Meduspremnik *izlazni_meduspr, sem_t *ulazni_bsem,
                    sem_t *ulazni_osem, sem_t *izlazni_bsem,
                    sem_t *izlazni_osem)
        {
                this->dretva_id = dretva_id;
                this->dretva_tip = dretva_tip;
                this->ulazni_meduspremnik = ulazni_meduspr;
                this->izlazni_meduspremnik = izlazni_meduspr;
                this->ulazni_bsem = ulazni_bsem;
                this->izlazni_bsem = izlazni_bsem;
                this->ulazni_osem = ulazni_osem;
                this->izlazni_osem = izlazni_osem;
        }
};

void ispis_meduspremnika(Meduspremnik *ulazni, Meduspremnik *izlazni)
{
        printf("UMS: ");
        for (int i = 0; i < broj_ulaznih_meduspremnika; i++) {
                ulazni[i].print();
                printf(" ");
        }
        printf("\n");
        printf("IMS: ");
        for (int i = 0; i < broj_izlaznih_meduspremnika; i++) {
                izlazni[i].print();
                printf(" ");
        }
        printf("\n\n");
}

void dohvati_od_senzora(struct Ulaz *ulaz)
{
        sleep(rand() % 6 + 5);
        (*ulaz).tip_informacije = rand() % broj_ulaznih_meduspremnika;
        (*ulaz).podatak = rand() % 26 + 97;
}

void obradi_ulaz(struct Ulaz *ulaz, Meduspremnik *meduspr)
{
        sleep(rand() % 2 + 2);
        (*ulaz).podatak += 1;
        (*ulaz).podatak -= 1;
        meduspr[(*ulaz).tip_informacije].pisi((*ulaz).podatak);
}

void dohvati_ulaz(struct Ulaz *ulaz, Meduspremnik *ulazni_meduspr)
{
        (*ulaz).tip_informacije = rand() % broj_izlaznih_meduspremnika;
        (*ulaz).podatak = ulazni_meduspr->citaj();
}

void *ulazna_dretva(void *arg)
{
        struct Ulaz ulaz;
        Dretve_args *argumenti = (Dretve_args *)arg;
        while (1) {
                dohvati_od_senzora(&ulaz);
                sem_wait(&argumenti->ulazni_bsem[ulaz.tip_informacije]);
                obradi_ulaz(&ulaz, argumenti->ulazni_meduspremnik);
                printf(ANSI_COLOR_BLUE
                       "Ulazna dretva %d dohvaca %c i stavlja u meduspremnik "
                       "%d" ANSI_COLOR_RESET "\n",
                       argumenti->dretva_id, ulaz.podatak,
                       ulaz.tip_informacije);
                ispis_meduspremnika(argumenti->ulazni_meduspremnik,
                                    argumenti->izlazni_meduspremnik);
                sem_post(&argumenti->ulazni_bsem[ulaz.tip_informacije]);
                sem_post(&argumenti->ulazni_osem[ulaz.tip_informacije]);
        }
        pthread_exit(NULL);
}

void *radna_dretva(void *arg)
{
        struct Ulaz ulaz;
        Dretve_args *argumenti = (Dretve_args *)arg;
        while (1) {
                sem_wait(&argumenti->ulazni_osem[argumenti->dretva_id]);
                sem_wait(&argumenti->ulazni_bsem[argumenti->dretva_id]);
                dohvati_ulaz(
                    &ulaz,
                    &argumenti->ulazni_meduspremnik[argumenti->dretva_id]);
                printf(ANSI_COLOR_RED
                       "Radna dretva %d dohvaca %c iz meduspremnika "
                       "%d" ANSI_COLOR_RESET "\n",
                       argumenti->dretva_id, ulaz.podatak,
                       argumenti->dretva_id);
                ispis_meduspremnika(argumenti->ulazni_meduspremnik,
                                    argumenti->izlazni_meduspremnik);
                sem_post(&argumenti->ulazni_bsem[argumenti->dretva_id]);

                sem_wait(&argumenti->izlazni_bsem[ulaz.tip_informacije]);
                obradi_ulaz(&ulaz, argumenti->izlazni_meduspremnik);
                printf(ANSI_COLOR_RED
                       "Radna dretva %d stavlja %c u meduspremnik "
                       "%d" ANSI_COLOR_RESET "\n",
                       argumenti->dretva_id, ulaz.podatak,
                       ulaz.tip_informacije);
                ispis_meduspremnika(argumenti->ulazni_meduspremnik,
                                    argumenti->izlazni_meduspremnik);
                sem_post(&argumenti->izlazni_bsem[ulaz.tip_informacije]);
                sem_post(&argumenti->izlazni_osem[ulaz.tip_informacije]);
        }
        pthread_exit(NULL);
}

void *izlazna_dretva(void *arg)
{
        struct Ulaz ulaz;
        Dretve_args *argumenti = (Dretve_args *)arg;
        while (1) {
                sem_wait(&argumenti->izlazni_osem[argumenti->dretva_id]);
                sem_wait(&argumenti->izlazni_bsem[argumenti->dretva_id]);
                dohvati_ulaz(
                    &ulaz,
                    &argumenti->izlazni_meduspremnik[argumenti->dretva_id]);
                printf(ANSI_COLOR_GREEN
                       "Izlazna dretva %d dohvaca %c iz meduspremnika "
                       "%d" ANSI_COLOR_RESET "\n",
                       argumenti->dretva_id, ulaz.podatak,
                       argumenti->dretva_id);
                ispis_meduspremnika(argumenti->ulazni_meduspremnik,
                                    argumenti->izlazni_meduspremnik);
                sem_post(&argumenti->izlazni_bsem[argumenti->dretva_id]);

                sleep(rand() % 2 + 2);
                printf(ANSI_COLOR_GREEN
                       "Izlazna dretva %d ispisuje %c" ANSI_COLOR_RESET "\n",
                       argumenti->dretva_id, ulaz.podatak);
                ispis_meduspremnika(argumenti->ulazni_meduspremnik,
                                    argumenti->izlazni_meduspremnik);
        }
        pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
        if (argc != 4) {
                printf("Krivi broj argumenata [%d], treba 3. Izlazim...\n",
                       argc - 1);
                return 1;
        }

        int broj_ulaznih_dretvi = atoi(argv[1]);
        int broj_radnih_dretvi = atoi(argv[2]);
        int broj_izlaznih_dretvi = atoi(argv[3]);
        broj_ulaznih_meduspremnika = broj_radnih_dretvi;
        broj_izlaznih_meduspremnika = broj_izlaznih_dretvi;
        int velicina_spremnika = 6;

        Meduspremnik ulazni_meduspremnik[broj_ulaznih_meduspremnika];
        for (int i = 0; i < broj_ulaznih_meduspremnika; i++) {
                ulazni_meduspremnik[i].Init(velicina_spremnika);
        }
        Meduspremnik izlazni_meduspremnik[broj_izlaznih_meduspremnika];
        for (int i = 0; i < broj_izlaznih_meduspremnika; i++) {
                izlazni_meduspremnik[i].Init(velicina_spremnika);
        }

        printf("Pocetak:\n");
        ispis_meduspremnika(ulazni_meduspremnik, izlazni_meduspremnik);

        sem_t ulazni_bsem[broj_ulaznih_meduspremnika];
        sem_t izlazni_bsem[broj_izlaznih_meduspremnika];
        sem_t ulazni_osem[broj_ulaznih_meduspremnika];
        sem_t izlazni_osem[broj_izlaznih_meduspremnika];
        for (int i = 0; i < broj_ulaznih_meduspremnika; i++) {
                sem_init(&ulazni_bsem[i], 0, 1);
                sem_init(&ulazni_osem[i], 0, 0);
        }
        for (int i = 0; i < broj_izlaznih_meduspremnika; i++) {
                sem_init(&izlazni_bsem[i], 0, 1);
                sem_init(&izlazni_osem[i], 0, 0);
        }

        struct Dretve_args *argumenti;
        pthread_t ulazne_dretve[broj_ulaznih_dretvi];
        for (int i = 0; i < broj_ulaznih_dretvi; i++) {
                argumenti = new Dretve_args(i, 0, ulazni_meduspremnik,
                                            izlazni_meduspremnik, ulazni_bsem,
                                            ulazni_osem, nullptr, nullptr);
                pthread_create(&ulazne_dretve[i], NULL, &ulazna_dretva,
                               argumenti);
        }
        sleep(30);
        pthread_t radne_dretve[broj_radnih_dretvi];
        for (int i = 0; i < broj_radnih_dretvi; i++) {
                argumenti = new Dretve_args(
                    i, 1, ulazni_meduspremnik, izlazni_meduspremnik,
                    ulazni_bsem, ulazni_osem, izlazni_bsem, izlazni_osem);
                pthread_create(&radne_dretve[i], NULL, &radna_dretva,
                               argumenti);
        }
        pthread_t izlazne_dretve[broj_izlaznih_dretvi];
        for (int i = 0; i < broj_izlaznih_dretvi; i++) {
                argumenti = new Dretve_args(
                    i, 2, ulazni_meduspremnik, izlazni_meduspremnik, nullptr,
                    nullptr, izlazni_bsem, izlazni_osem);
                pthread_create(&izlazne_dretve[i], NULL, &izlazna_dretva,
                               argumenti);
        }

        for (int i = 0; i < broj_ulaznih_dretvi; i++) {
                pthread_join(ulazne_dretve[i], NULL);
        }
        for (int i = 0; i < broj_radnih_dretvi; i++) {
                pthread_join(radne_dretve[i], NULL);
        }
        for (int i = 0; i < broj_izlaznih_dretvi; i++) {
                pthread_join(izlazne_dretve[i], NULL);
        }

        return 0;
}