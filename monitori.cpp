#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

struct ListElement {
    int podatak;
    ListElement *sljedeci_element;

    ListElement(int podatak) {
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
    LinkedList(){
        head = nullptr;
        tail = nullptr;
        velicina = 0;
    }

    void umetni(int podatak) {
        ListElement *el = new ListElement(podatak);
        if(head == nullptr){
            head = el;
            tail = el;
        }
        else{
            tail->sljedeci_element = el;
            tail = el;
        }
        velicina++;
    }

    int citaj(int index) {
        if(index >= velicina){
            return -1;
        }
        ListElement *temp = head;
        for (int i = 0; i < index; i++)
        {
            temp = temp->sljedeci_element;
        }
        return temp->podatak;
    }

    int brisi(int index) {
        if(index >= velicina || velicina == 0) {
            return -1;
        }
        ListElement *temp1 = head;
        ListElement *temp2 = nullptr;
        for (int i = 0; i < index; i++)
        {
            temp2 = temp1;
            temp1 = temp1->sljedeci_element;
        }
        if(temp1->sljedeci_element == nullptr){
            tail = temp2;
            tail->sljedeci_element = nullptr;
            delete temp1;
            velicina--;
            return 0;
        }
        else if(head == temp1){
            head = temp1->sljedeci_element;
            delete temp1;
            velicina--;
            return 0;
        }
        else if(head == tail){
            head = nullptr;
            tail = nullptr;
            delete temp1;
            velicina--;
            return 0;
        }
        else {
            temp2->sljedeci_element = temp1->sljedeci_element;
            temp1->sljedeci_element = nullptr;
            delete temp1;
            velicina--;
            return 0;
        }
        return -1;
    }

    void print() {
        ListElement *temp = head;
        while(temp != nullptr){
            printf("%d ", temp->podatak);
            temp = temp->sljedeci_element;
        }
    }

    int velicina() {
        return velicina;
    }
};

pthread_mutex_t monitor;
pthread_cond_t red_citaca;
pthread_cond_t red_pisaca;
pthread_cond_t red_brisaca;

int br_citaca_ceka = 0;
int br_citaca_cita = 0;

int br_brisaca_brise = 0;
int br_brisaca_ceka = 0;

int br_pisaca_ceka = 0;
int br_pisaca_pise = 0;

LinkedList *list = new LinkedList();

void *dretva_pisac(void *arg){
    while(1){
    int podatak = rand() % 100 + 1;
    pthread_mutex_lock(&monitor);
    printf("Pisac %d zeli dodati vrijednost %d", (int*) arg, podatak);
    br_pisaca_ceka++;
    while (br_brisaca_brise + br_brisaca_ceka > 0 && br_pisaca_ceka > 0)
    {
        pthread_cond_wait(&red_pisaca, &monitor);
    }
    br_pisaca_pise++;
    br_pisaca_ceka--;
    list->umetni(podatak);
    printf("Pisac %d zapocinje dodavanje vrijednosti %d na kraj liste", (int*) arg, podatak);
    pthread_mutex_unlock(&monitor);

    sleep(rand() % 6 + 5);

    pthread_mutex_lock(&monitor);
    br_pisaca_pise--;
    if(br_pisaca_pise == 0 && br_brisaca_ceka > 0){
        pthread_cond_signal(&red_brisaca);
        pthread_cond_signal(&red_pisaca);
    }
    printf("Pisac %d vise ne koristi listu", (int*) arg);
    pthread_mutex_unlock(&monitor);

    sleep(rand() % 6 + 5);
    }
}

void *dretva_citac(void *arg){
    while(1){
        int index = rand() % list->velicina();
        pthread_mutex_lock(&monitor);
        printf("Citac %d zeli citati element %d liste\n", (int*) arg, index);
        br_citaca_ceka++;
        while(br_brisaca_brise + br_brisaca_ceka > 0){
            pthread_cond_wait(&red_citaca, &monitor);
        }
        br_citaca_cita++;
        br_citaca_ceka--;
        int rezultat = list->citaj(index);
        printf("Citac %d cita element %d liste (vrijednost %d)", (int*) arg, index, rezultat);
        pthread_mutex_unlock(&monitor);

        sleep(rand() % 6 + 5);

        pthread_mutex_lock(&monitor);
        br_citaca_cita--;
        if(br_citaca_cita == 0 && br_brisaca_ceka > 0){
            pthread_cond_signal(&red_brisaca);
        }
        printf("Citac vise ne koristi listu");
        pthread_mutex_unlock(&monitor);

        sleep(rand() % 6 + 5);
    }
}

void *dretva_brisac(void *arg){

}

int main(){

    pthread_mutex_init(&monitor, NULL);
    pthread_cond_init(&red_citaca, NULL);
    pthread_cond_init(&red_pisaca, NULL);
    pthread_cond_init(&red_brisaca, NULL);

    pthread_t pisaci[4];
    for (int i = 0; i < 4; i++)
    {
        pthread_create(&pisaci[i], NULL, &dretva_pisac, &i);
    }

    pthread_t citaci[10];
    for (int i = 0; i < 10; i++)
    {
        pthread_create(&citaci[i], NULL, &dretva_citac, &i);
    }

    pthread_t brisaci[2];
    for (int i = 0; i < 2; i++)
    {
        pthread_create(&brisaci[i], NULL, &dretva_brisac, &i);
    }

    for (int i = 0; i < 4; i++)
    {
        pthread_join(pisaci[i], NULL);
    }
    for (int i = 0; i < 10; i++)
    {
        pthread_join(citaci[i], NULL);
    }
    for (int i = 0; i < 2; i++)
    {
        pthread_join(brisaci[i], NULL);
    }
    

}