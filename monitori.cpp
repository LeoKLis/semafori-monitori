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
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_RESET "\x1b[0m"

struct ListElement {
        int podatak;
        ListElement *sljedeci_element;

        ListElement(int podatak)
        {
                this->podatak = podatak;
                sljedeci_element = nullptr;
        }
};
class LinkedList {
       private:
        ListElement *head;
        ListElement *tail;
        int velicina;

       public:
        LinkedList()
        {
                head = nullptr;
                tail = nullptr;
                velicina = 0;
        }

        void inicijaliziraj()
        {
                ListElement *el = new ListElement(-1);
                if (head == nullptr) {
                        head = el;
                        tail = el;
                }
                else {
                        tail->sljedeci_element = el;
                        tail = el;
                }
                velicina++;
        }

        void umetni(int podatak) { tail->podatak = podatak; }

        int citaj(int index)
        {
                if (index >= velicina) {
                        return -1;
                }
                ListElement *temp = head;
                for (int i = 0; i < index; i++) {
                        temp = temp->sljedeci_element;
                }
                return temp->podatak;
        }

        int brisi(int index)
        {
                if (index >= velicina || velicina == 0) {
                        return 0;
                }
                ListElement *temp1 = head;
                ListElement *temp2 = nullptr;
                for (int i = 0; i < index; i++) {
                        temp2 = temp1;
                        temp1 = temp1->sljedeci_element;
                }
                if (head == tail) {
                        head = nullptr;
                        tail = nullptr;
                        delete temp1;
                        velicina--;
                        return 1;
                }
                else if (head == temp1) {
                        head = temp1->sljedeci_element;
                        delete temp1;
                        velicina--;
                        return 1;
                }
                else if (temp1->sljedeci_element == nullptr) {
                        tail = temp2;
                        tail->sljedeci_element = nullptr;
                        delete temp1;
                        velicina--;
                        return 1;
                }
                else {
                        temp2->sljedeci_element = temp1->sljedeci_element;
                        temp1->sljedeci_element = nullptr;
                        delete temp1;
                        velicina--;
                        return 1;
                }
                return 0;
        }

        void print()
        {
                if (head == nullptr) {
                        printf("-");
                }
                ListElement *temp = head;
                while (temp != nullptr) {
                        if (temp->podatak == -1) {
                                printf("null ");
                        }
                        else {
                                printf("%d ", temp->podatak);
                        }
                        temp = temp->sljedeci_element;
                }
        }

        int get_velicina() { return velicina; }
};

sem_t ceka_x_elemenata;

pthread_mutex_t monitor;
pthread_cond_t red_citaca;
pthread_cond_t red_pisaca;
pthread_cond_t red_brisaca;

int br_citaca_ceka = 0;
int br_citaca_cita = 0;

int br_brisaca_ceka = 0;
int br_brisaca_brise = 0;

int br_pisaca_ceka = 0;
int br_pisaca_pise = 0;

LinkedList *list = new LinkedList();

enum tip_dretve { PISAC, CITAC, BRISAC };
enum tip_radnje { ZELI, KRECE, ZAVRSAVA, OTKAZANO };
void ispis(tip_dretve td, tip_radnje tr, int *broj_tipa_dretve, int index,
           int vrijednost)
{
        switch (td) {
                case PISAC:
                        switch (tr) {
                                case ZELI:
                                        printf(ANSI_COLOR_GREEN
                                               "Pisac %d zeli umetnuti "
                                               "vrijednost %d" ANSI_COLOR_RESET
                                               "\n",
                                               broj_tipa_dretve, vrijednost);
                                        printf(
                                            "aktivnih: citaca=%d, pisaca=%d, "
                                            "brisaca=%d\n",
                                            br_citaca_cita, br_pisaca_pise,
                                            br_brisaca_brise);
                                        printf(
                                            "cekaju: citaca=%d, pisaca=%d, "
                                            "brisaca=%d\n",
                                            br_citaca_ceka, br_pisaca_ceka,
                                            br_brisaca_ceka);
                                        list->print();
                                        break;
                                case KRECE:
                                        printf(ANSI_COLOR_GREEN
                                               "Pisac %d zapocinje dodavanje "
                                               "vrijednosti %d na kraj "
                                               "liste" ANSI_COLOR_RESET "\n",
                                               broj_tipa_dretve, vrijednost);
                                        printf(
                                            "aktivnih: citaca=%d, pisaca=%d, "
                                            "brisaca=%d\n",
                                            br_citaca_cita, br_pisaca_pise,
                                            br_brisaca_brise);
                                        printf(
                                            "cekaju: citaca=%d, pisaca=%d, "
                                            "brisaca=%d\n",
                                            br_citaca_ceka, br_pisaca_ceka,
                                            br_brisaca_ceka);
                                        list->print();
                                        break;
                                case ZAVRSAVA:
                                        printf(ANSI_COLOR_GREEN
                                               "Pisac %d dodao vrijednost %d i "
                                               "vise ne koristi "
                                               "listu " ANSI_COLOR_RESET "\n",
                                               broj_tipa_dretve, vrijednost);
                                        printf(
                                            "aktivnih: citaca=%d, pisaca=%d, "
                                            "brisaca=%d\n",
                                            br_citaca_cita, br_pisaca_pise,
                                            br_brisaca_brise);
                                        printf(
                                            "cekaju: citaca=%d, pisaca=%d, "
                                            "brisaca=%d\n",
                                            br_citaca_ceka, br_pisaca_ceka,
                                            br_brisaca_ceka);
                                        list->print();
                                        break;
                                default:
                                        break;
                        }
                        break;
                case CITAC:
                        switch (tr) {
                                case ZELI:
                                        printf(ANSI_COLOR_BLUE
                                               "Citac %d zeli citati element "
                                               "%d liste" ANSI_COLOR_RESET "\n",
                                               broj_tipa_dretve, vrijednost);
                                        printf(
                                            "aktivnih: citaca=%d, pisaca=%d, "
                                            "brisaca=%d\n",
                                            br_citaca_cita, br_pisaca_pise,
                                            br_brisaca_brise);
                                        printf(
                                            "cekaju: citaca=%d, pisaca=%d, "
                                            "brisaca=%d\n",
                                            br_citaca_ceka, br_pisaca_ceka,
                                            br_brisaca_ceka);
                                        list->print();
                                        break;
                                case KRECE:
                                        printf(ANSI_COLOR_BLUE
                                               "Citac %d krece sa citanjem "
                                               "elementa %d liste (vrijednost "
                                               "%d)" ANSI_COLOR_RESET "\n",
                                               broj_tipa_dretve, index,
                                               vrijednost);
                                        printf(
                                            "aktivnih: citaca=%d, pisaca=%d, "
                                            "brisaca=%d\n",
                                            br_citaca_cita, br_pisaca_pise,
                                            br_brisaca_brise);
                                        printf(
                                            "cekaju: citaca=%d, pisaca=%d, "
                                            "brisaca=%d\n",
                                            br_citaca_ceka, br_pisaca_ceka,
                                            br_brisaca_ceka);
                                        list->print();
                                        break;
                                case ZAVRSAVA:
                                        printf(ANSI_COLOR_BLUE
                                               "Citac %d procitao element %d "
                                               "(vrijednost %d) i vise ne "
                                               "koristi listu" ANSI_COLOR_RESET
                                               "\n",
                                               broj_tipa_dretve, index,
                                               vrijednost);
                                        printf(
                                            "aktivnih: citaca=%d, pisaca=%d, "
                                            "brisaca=%d\n",
                                            br_citaca_cita, br_pisaca_pise,
                                            br_brisaca_brise);
                                        printf(
                                            "cekaju: citaca=%d, pisaca=%d, "
                                            "brisaca=%d\n",
                                            br_citaca_ceka, br_pisaca_ceka,
                                            br_brisaca_ceka);
                                        list->print();
                                        break;
                                case OTKAZANO:
                                        printf(ANSI_COLOR_MAGENTA
                                               "Citanje otkazano (citac %d), "
                                               "element liste je izvan "
                                               "polja." ANSI_COLOR_RESET "\n",
                                               broj_tipa_dretve);
                                        break;
                                default:
                                        break;
                        }
                        break;
                case BRISAC:
                        switch (tr) {
                                case ZELI:
                                        printf(ANSI_COLOR_RED
                                               "Brisac %d zeli izbrisati "
                                               "element %d (vrijednost %d) "
                                               "liste" ANSI_COLOR_RESET "\n",
                                               broj_tipa_dretve, index,
                                               vrijednost);
                                        printf(
                                            "aktivnih: citaca=%d, pisaca=%d, "
                                            "brisaca=%d\n",
                                            br_citaca_cita, br_pisaca_pise,
                                            br_brisaca_brise);
                                        printf(
                                            "cekaju: citaca=%d, pisaca=%d, "
                                            "brisaca=%d\n",
                                            br_citaca_ceka, br_pisaca_ceka,
                                            br_brisaca_ceka);
                                        list->print();
                                        break;
                                case KRECE:
                                        printf(
                                            ANSI_COLOR_RED
                                            "Brisac %d krece sa brisanjem "
                                            "elementa %d iz liste (vrijednost "
                                            "%d)" ANSI_COLOR_RESET "\n",
                                            broj_tipa_dretve, index,
                                            list->citaj(index));
                                        printf(
                                            "aktivnih: citaca=%d, pisaca=%d, "
                                            "brisaca=%d\n",
                                            br_citaca_cita, br_pisaca_pise,
                                            br_brisaca_brise);
                                        printf(
                                            "cekaju: citaca=%d, pisaca=%d, "
                                            "brisaca=%d\n",
                                            br_citaca_ceka, br_pisaca_ceka,
                                            br_brisaca_ceka);
                                        list->print();
                                        break;
                                case ZAVRSAVA:
                                        printf(ANSI_COLOR_RED
                                               "Brisac %d obrisao element %d i "
                                               "vise ne koristi "
                                               "listu" ANSI_COLOR_RESET "\n",
                                               broj_tipa_dretve, index);
                                        printf(
                                            "aktivnih: citaca=%d, pisaca=%d, "
                                            "brisaca=%d\n",
                                            br_citaca_cita, br_pisaca_pise,
                                            br_brisaca_brise);
                                        printf(
                                            "cekaju: citaca=%d, pisaca=%d, "
                                            "brisaca=%d\n",
                                            br_citaca_ceka, br_pisaca_ceka,
                                            br_brisaca_ceka);
                                        list->print();
                                        break;
                                case OTKAZANO:
                                        printf(ANSI_COLOR_MAGENTA
                                               "Brisanje otkazano (brisac %d), "
                                               "element liste je izvan "
                                               "polja." ANSI_COLOR_RESET "\n",
                                               broj_tipa_dretve);
                                        break;
                                default:
                                        break;
                        }
                        break;
                default:
                        break;
        }
        printf("\n\n");
}

void *dretva_pisac(void *arg)
{
        while (1) {
                int podatak = rand() % 100 + 1;
                pthread_mutex_lock(&monitor);
                br_pisaca_ceka++;
                ispis(PISAC, ZELI, (int *)arg, 0, podatak);
                while (br_brisaca_brise + br_brisaca_ceka > 0 ||
                       br_pisaca_pise == 1) {
                        pthread_cond_wait(&red_pisaca, &monitor);
                }
                br_pisaca_pise = 1;
                br_pisaca_ceka--;
                list->inicijaliziraj();
                ispis(PISAC, KRECE, (int *)arg, 0, podatak);
                pthread_mutex_unlock(&monitor);

                sleep(rand() % 6 + 5);
                list->umetni(podatak);
                if (list->get_velicina() == 6) {
                        sem_post(&ceka_x_elemenata);
                }

                pthread_mutex_lock(&monitor);
                br_pisaca_pise = 0;
                if (br_pisaca_ceka > 0) {
                        pthread_cond_signal(&red_pisaca);
                }
                if (br_brisaca_ceka > 0) {
                        pthread_cond_signal(&red_brisaca);
                }
                ispis(PISAC, ZAVRSAVA, (int *)arg, 0, podatak);
                pthread_mutex_unlock(&monitor);

                sleep(rand() % 4 + 5);
        }
        pthread_exit(NULL);
}

void *dretva_citac(void *arg)
{
        while (1) {
                int index = rand() % list->get_velicina();
                pthread_mutex_lock(&monitor);
                br_citaca_ceka++;
                ispis(CITAC, ZELI, (int *)arg, index, list->citaj(index));
                while (br_brisaca_brise + br_brisaca_ceka > 0) {
                        pthread_cond_wait(&red_citaca, &monitor);
                }
                br_citaca_cita++;
                br_citaca_ceka--;
                pthread_mutex_unlock(&monitor);

                bool moze_citati = list->get_velicina() > index;
                if (moze_citati) {
                        int rezultat = list->citaj(index);
                        ispis(CITAC, KRECE, (int *)arg, index,
                              list->citaj(index));
                        sleep(rand() % 6 + 5);
                }

                pthread_mutex_lock(&monitor);
                br_citaca_cita--;
                if (br_citaca_cita == 0 && br_brisaca_ceka > 0) {
                        pthread_cond_signal(&red_brisaca);
                }
                if (moze_citati) {
                        ispis(CITAC, ZAVRSAVA, (int *)arg, index,
                              list->citaj(index));
                }
                else {
                        ispis(CITAC, OTKAZANO, (int *)arg, index,
                              list->citaj(index));
                }
                pthread_mutex_unlock(&monitor);

                sleep(rand() % 4 + 5);
        }
        pthread_exit(NULL);
}

void *dretva_brisac(void *arg)
{
        while (1) {
                int index = rand() % list->get_velicina();
                pthread_mutex_lock(&monitor);
                br_brisaca_ceka++;
                ispis(BRISAC, ZELI, (int *)arg, index, list->citaj(index));
                while (br_brisaca_brise == 1 || br_citaca_cita > 0 ||
                       br_pisaca_pise > 0) {
                        pthread_cond_wait(&red_brisaca, &monitor);
                }
                br_brisaca_brise = 1;
                br_brisaca_ceka--;
                pthread_mutex_unlock(&monitor);

                bool moze_obrisati = list->get_velicina() > index;
                if (moze_obrisati) {
                        ispis(BRISAC, KRECE, (int *)arg, index,
                              list->citaj(index));
                        sleep(rand() % 6 + 5);
                        list->brisi(index);
                }

                pthread_mutex_lock(&monitor);
                br_brisaca_brise = 0;
                if (br_pisaca_ceka > 0) {
                        pthread_cond_signal(&red_pisaca);
                }
                if (br_brisaca_ceka > 0) {
                        pthread_cond_signal(&red_brisaca);
                }
                if (br_citaca_ceka > 0) {
                        pthread_cond_signal(&red_citaca);
                }

                if (moze_obrisati) {
                        ispis(BRISAC, ZAVRSAVA, (int *)arg, index,
                              list->citaj(index));
                }
                else {
                        ispis(BRISAC, OTKAZANO, (int *)arg, index,
                              list->citaj(index));
                }
                pthread_mutex_unlock(&monitor);

                sleep(rand() % 6 + 5);
        }
        pthread_exit(NULL);
}

int main()
{
        sem_init(&ceka_x_elemenata, 0, 0);
        pthread_mutex_init(&monitor, NULL);
        pthread_cond_init(&red_citaca, NULL);
        pthread_cond_init(&red_pisaca, NULL);
        pthread_cond_init(&red_brisaca, NULL);

        pthread_t pisaci[4];
        long pisaci_id_arr[4];
        for (int i = 0; i < 4; i++) {
                pisaci_id_arr[i] = i;
                pthread_create(&pisaci[i], NULL, &dretva_pisac,
                               (void *)pisaci_id_arr[i]);
        }

        sem_wait(&ceka_x_elemenata);
        pthread_t citaci[10];
        long citaci_id_arr[10];
        for (int i = 0; i < 10; i++) {
                citaci_id_arr[i] = i;
                pthread_create(&citaci[i], NULL, &dretva_citac,
                               (void *)citaci_id_arr[i]);
        }

        pthread_t brisaci[2];
        long brisaci_id_arr[2];
        for (int i = 0; i < 2; i++) {
                brisaci_id_arr[i] = i;
                pthread_create(&brisaci[i], NULL, &dretva_brisac,
                               (void *)brisaci_id_arr[i]);
        }

        for (int i = 0; i < 4; i++) {
                pthread_join(pisaci[i], NULL);
        }
        for (int i = 0; i < 10; i++) {
                pthread_join(citaci[i], NULL);
        }
        for (int i = 0; i < 2; i++) {
                pthread_join(brisaci[i], NULL);
        }

        return 0;
}