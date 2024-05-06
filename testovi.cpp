#include <stdio.h>

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
            if(pisi_ptr == citaj_ptr + 1) citaj_ptr = (citaj_ptr + 1) % velicina;
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

    void print() {
        for (int i = 0; i < velicina; i++)
        {
            printf("%c ", polje[i]);
        }        
    }
};

int main() {
    Meduspremnik *ms = new Meduspremnik(6);

    ms->pisi('b');
    ms->pisi('a');
    ms->pisi('n');
    ms->pisi('a');
    ms->pisi('n');
    ms->pisi('a');

    ms->print();

    printf("\n%c", ms->citaj());
    printf("%c", ms->citaj());
    printf("%c", ms->citaj());
    printf("%c", ms->citaj());
    printf("%c", ms->citaj());
    printf("%c", ms->citaj());
}