#include "makePage.h"

int txtpage[PAGENUM];
int escpage[PAGENUM];

int *randomPage()
{   
    int i;
    srand(time(NULL));

    for (i = 0; i < PAGENUM; i++)
    {
        txtpage[i] = (rand() % MAXPNUM + 1);
    }
    return txtpage;
}

int *readPage(FILE *fp, char *pageName)
{
    fp = fopen(pageName, "r+");
    fread(&txtpage, sizeof(txtpage), 1, fp);

    return txtpage;
}

int *writePage(FILE *fp)
{
    fp = fopen("page.txt", "w+");
    randomPage();
    fwrite(&txtpage, sizeof(txtpage), 1, fp);
    fclose(fp);

    return txtpage;
}

int *readEscPage(FILE *fp, char *pageName)
{
    fp = fopen(pageName, "r+");
    fread(&escpage, sizeof(escpage), 1, fp);
    fclose(fp);

    return escpage;
}

int *escPage()
{
    int i;
    srand(time(NULL));
    for(int i = 0; i < PAGENUM; i++)
    {
        escpage[i] = (rand() % 2);
    }
    return escpage;
}