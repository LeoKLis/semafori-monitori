#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <memory>

class Meduspremnik
{
private:
    int velicina;
    int pisi_ptr, citaj_ptr;
    char *polje;

public:
    Meduspremnik() {
        this->velicina = 6;
        pisi_ptr = citaj_ptr = -1;
        polje = new char[velicina];
        for (int i = 0; i < velicina; i++)
        {
            polje[i] = 96;
        }
        
    }

    Meduspremnik(int velicina)
    {
        this->velicina = velicina;
        pisi_ptr = citaj_ptr = -1;
        polje = new char[velicina];
        for (int i = 0; i < velicina; i++)
        {
            polje[i] = 96;
        }
    }

    void Init(int velicina){
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
            if (pisi_ptr == citaj_ptr && polje[citaj_ptr + 1] != 96)
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
        polje[citaj_ptr] = 96;
        citaj_ptr = (citaj_ptr + 1) % velicina;
        return podatak;
    }

    bool prazan()
    {
        return citaj_ptr == pisi_ptr;
    }

    void print()
    {
        for (int i = 0; i < velicina; i++)
        {
            printf("%c ", polje[i]);
        }
    }
};

struct radne_dretve_args {
    Meduspremnik *ms1;
    Meduspremnik *ms2;
};

void ispis_meduspremnika(Meduspremnik *ulazni, Meduspremnik *izlazni) {
    printf("UMS: ");
    for (int i = 0; i < 6; i++)
    {
        ulazni[i].print();
        printf(" ");
    }
    printf("\n");
    printf("IMS: ");
    for (int i = 0; i < 3; i++)
    {
        izlazni[i].print();
        printf(" ");
    }
    printf("\n");
    
}

void *banana(void *arg){
    std::unique_ptr<radne_dretve_args> wrp( static_cast<radne_dretve_args*>(arg) );
    wrp->ms1[0].pisi('a');
    wrp->ms1[0].print();

    printf("\n");

    wrp->ms2[0].pisi('p');
    wrp->ms2[0].pisi('r');
    wrp->ms2[0].pisi('v');
    wrp->ms2[0].pisi('i');
    wrp->ms2[0].print();
    
    printf("\n");

    wrp->ms2[1].pisi('d');
    wrp->ms2[1].pisi('r');
    wrp->ms2[1].pisi('u');
    wrp->ms2[1].pisi('g');
    wrp->ms2[1].pisi('i');
    wrp->ms2[1].print();

    printf("\n");

    ispis_meduspremnika(wrp->ms1, wrp->ms2);

    pthread_exit(NULL);
}

void *kokos(void *arg){
    std::unique_ptr<radne_dretve_args> wrp(static_cast<radne_dretve_args*>(arg));
    wrp->ms1[1].pisi('x');
    wrp->ms1[1].print();

    printf("\n");

    wrp->ms2[2].pisi('p');
    wrp->ms2[2].pisi('r');
    wrp->ms2[2].pisi('v');
    wrp->ms2[2].pisi('i');
    wrp->ms2[2].print();
    
    printf("\n");

    wrp->ms1[5].pisi('d');
    wrp->ms1[5].pisi('r');
    wrp->ms1[5].pisi('u');
    wrp->ms1[5].pisi('g');
    wrp->ms1[5].pisi('i');
    wrp->ms1[5].print();

    printf("\n");

    ispis_meduspremnika(wrp->ms1, wrp->ms2);

    pthread_exit(NULL);
}

void charinit(char *chs, int velicina){
    chs = (char*) realloc(chs, velicina);
}

int main()
{
    /* Meduspremnik ulazni_meduspremnik[6];
    for (int i = 0; i < 6; i++)
    {
        ulazni_meduspremnik[i].Init(6);
    }
    Meduspremnik izlazni_meduspremnik[3];
    for (int i = 0; i < 3; i++)
    {
        izlazni_meduspremnik[i].Init(6);
    }

    struct radne_dretve_args *args;
    args = new radne_dretve_args;
    args->ms1 = ulazni_meduspremnik;
    args->ms2 = izlazni_meduspremnik;
    pthread_t thr1;
    pthread_create(&thr1, NULL, &banana, args);

    args = new radne_dretve_args;
    args->ms1 = ulazni_meduspremnik;
    args->ms2 = izlazni_meduspremnik;
    pthread_t thr2;
    pthread_create(&thr2, NULL, &kokos, args);


    pthread_join(thr2, NULL);
    pthread_join(thr2, NULL); */

    char *arr = (char*) malloc(1);

    charinit(arr, 6);

    for (int i = 0; i < 6; i++)
    {
        arr[i] = 'a';
        printf("%c", arr[i]);
    }
    

    return 0;

}