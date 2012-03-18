/*
*	Copyright 2012, Andrea Mazzotti <amm263@gmail.com>
*
*	Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
*	provided that the above copyright notice and this permission notice appear in all copies.
*
*	THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
*	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR 
*	ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS 
* 	OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING 
*	OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*
*/


#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct nodo {
	int ID_Nodo;
	int N_Archi_Locali;
	int *vettore_adiacenza;
	struct nodoLista *testa;
	struct nodoLista *ultimo;
};

struct nodoLista {
	struct nodo *nodo;
	struct nodoLista *next;
};

//VARIABILI GLOBALI
int *vettore_archi=NULL; // Vettore dove vengono appoggiati i dati appena letti dal file.
int N_Nodi=0; // Numero di nodi in totale nel grafo.
int N_Archi=0; // N_Archi/2 è il numero di archi nel grafo. Ne conta esattamente il doppio per esigenze di costruzione del vettore_archi
int *pila_comuni=NULL; //Non è propriamente una pila, ma per affetto continua a chiamarsi così.
int *clique=NULL; //Vettore in cui viene salvata la clique più grande trovata.
int N_Clique=0; //Qui viene salvata la dimensione della Clique massima trovata di volta in volta.
struct nodo **vettore_nodi=NULL;
// FUNZIONI


/*	Questa funzione inizializza il vettore_archi e lo riempie, leggendo il file di input.
	Ha qualche problema con files che continuano con righe vuote e che contengono altro testo.
	Fare attenzione quindi ai dati sottoposti.
	La path del file sarà ad esempio C:\\Users\Utente\Documenti\grafo.txt su sistemi Windows
	oppure /home/utente/documenti/grafo.txt su sistemi Unix */
void fileread()
{
	char ch;
	char file_path[500]; //Stringa contenente la path del file.
	FILE *fp;
	int controllo=1;
	int i=0;
	int temp=0;
	while(controllo)
    {
        i=0;
        printf("Inserisci la path completa del file: ");
        while ((ch=getche())!='\n' && ch!='\r')
        {
            file_path[i]=ch;
            i++;
        }
        file_path[i]='\0'; //Chiudo la stringa.
        printf("\n");
        fp=fopen(file_path,"r"); //Apro il file
        if (fp==NULL)
        {
            controllo=1;
            printf("\nErrore, il file non esiste o e' corrotto\n\n");
            fclose(fp);
            file_path[500]="";
        }
        else
        {
            controllo=0;
        }
    }
    fscanf(fp,"%d", &N_Nodi);
    vettore_archi=(int *)malloc(N_Nodi*N_Nodi*sizeof(int)+sizeof(int)); // Non sto a contare prima tutte le righe del file, sicuramente il vettore non sarà più grande di n*n+1.
	if (vettore_archi==NULL)
	{
		printf("Errore, non sono riuscito ad allocare vettore_archi in memoria");
		exit(1);
	}
    i=0;
    while (!feof(fp))
    {
        fscanf(fp, "%d", &temp);
        vettore_archi[i]=temp;
        i++;
    }
    N_Archi=i; //A questo punto il numero di archi è dato da i/2, il vettore_archi è riempito fino a i-1.
	/* DEBUG, VISUALIZZA IL VETTOREARCHI
    for (i=0;i<N_Archi;i++)
    {
        printf(" %d",vettore_archi[i]);
        i++;
        printf(",%d |",vettore_archi[i]);

    }
    printf("\nArchi: %d\n\n",N_Archi/2);
     FINE DEBUG */
}

/*Questa funzione si preoccupa di inizializzare la macrostruttura, creando il vettore principale ed una struttura per ogni nodo*/
void initprimarystruct()
{
	int i;
	int y;
	vettore_nodi=(struct nodo **)malloc(N_Nodi*sizeof(struct nodo *));
	if (vettore_nodi==NULL)
	{
		printf("Errore, non sono riuscito ad allocare vettore_nodi in memoria");
		exit(1);
	}
	for (i=0;i<N_Nodi;i++)
	{
		vettore_nodi[i]=(struct nodo *)malloc(sizeof(struct nodo));
		if (vettore_nodi[i]==NULL)
		{
			printf("Non sono riuscito ad allocare vettore_nodi[%d] in memoria",i);
			exit(1);
		}
		vettore_nodi[i]->ID_Nodo=i+1;
		vettore_nodi[i]->N_Archi_Locali=0;
		vettore_nodi[i]->testa=NULL;
		vettore_nodi[i]->ultimo=NULL;
		vettore_nodi[i]->vettore_adiacenza=(int *)malloc(N_Nodi*sizeof(int));
		if (vettore_nodi[i]->vettore_adiacenza==NULL)
		{
			printf("Non sono riuscito ad allocare vettore_adiacenza di vettore_nodi[%d]",i);
			exit(1);
		}
		for (y=0;y<N_Nodi;y++)
		{
			vettore_nodi[i]->vettore_adiacenza[y]=0;
		}
	}
	/* DEBUG
    printf("Ho inizializzato la struttura primaria, inserendo i nodi: ");
    for (i=0;i<N_Nodi;i++)
    {
        printf("%d, ",vettore_nodi[i]->ID_Nodo);
    }
    printf("\n\n");
    /* FINE DEBUG */
}

/*Questa funzione carica i dati nella macrostruttura, liste di adiacenza per nodo comprese.*/

void dataload()
{
	int i;
	int nodo;
	int adiacente;
	struct nodoLista *nuovo=NULL;
	for (i=0;i<N_Archi;i++)
	{
		nodo=vettore_archi[i];
		i++;
		adiacente=vettore_archi[i];
		// Considero un nodo e poi il suo adiacente, carico l'adiacente nella lista del nodo.
		nuovo=(struct nodoLista *)malloc(sizeof(struct nodoLista));
		if (nuovo==NULL)
		{
			printf("Non sono riuscito ad allocare nodoLista di %d,%d",nodo,adiacente);
			exit(1);
		}
		nuovo->next=NULL;
		nuovo->nodo=vettore_nodi[adiacente-1];
		if (vettore_nodi[nodo-1]->testa==NULL)
		{
			vettore_nodi[nodo-1]->testa=nuovo;
			vettore_nodi[nodo-1]->ultimo=nuovo;
		}
		else
		{
			vettore_nodi[nodo-1]->ultimo->next=nuovo;
			vettore_nodi[nodo-1]->ultimo=nuovo;
		}
		//Ho caricato il nodoLista, ora metto la corrispondenza in vettore_adiacenza
		vettore_nodi[nodo-1]->vettore_adiacenza[adiacente-1]=1;
		//Incremento il contatore archi per questo nodo
		vettore_nodi[nodo-1]->N_Archi_Locali++;
		//Considero ora l'adiacente e faccio l'inverso.
		nuovo=(struct nodoLista *)malloc(sizeof(struct nodoLista));
		if (nuovo==NULL)
		{
			printf("Non sono riuscito ad allocare nodoLista di %d,%d",adiacente,nodo);
			exit(1);
		}
		nuovo->next=NULL;
		nuovo->nodo=vettore_nodi[nodo-1];
		if (vettore_nodi[adiacente-1]->testa==NULL)
		{
			vettore_nodi[adiacente-1]->testa=nuovo;
			vettore_nodi[adiacente-1]->ultimo=nuovo;
		}
		else
		{
			vettore_nodi[adiacente-1]->ultimo->next=nuovo;
			vettore_nodi[adiacente-1]->ultimo=nuovo;
		}
		//Ho caricato il nodoLista, ora metto la corrispondenza in vettore_adiacenza
		vettore_nodi[adiacente-1]->vettore_adiacenza[nodo-1]=1;
		//Incremento il contatore archi per questo nodo
		vettore_nodi[adiacente-1]->N_Archi_Locali++;
	}
}

/* 	Funzione per inserire i nodi nella pila.
	Restituisce un int per mantenere felice GCC.	*/
int push_pila(int nodo)
{
	int i;
	for (i=0;i<N_Nodi;i++)
	{
		if (pila_comuni[i]==-2)
		{
			pila_comuni[i]=nodo;
			return 0;
		}
	}
	return 1;
}

/* 	Funzione che toglie un nodo ad un determinato indice dalla pila e sposta tutti gli altri a sinistra di una posizione.
	Restituisce un int per mantenere felice GCC.*/
int pop_pila(int indice)
{
	int i;
	if (indice==(N_Nodi-1))
	{
		pila_comuni[indice]=-2;
	}
	else
	{
		for (i=indice;i<N_Nodi;i++)
		{
			if (pila_comuni[i]==-2)
				break;
			if (i==(N_Nodi-1))
			{
				pila_comuni[i]=-2;
				break;
			}
			pila_comuni[i]=pila_comuni[i+1];
		}
	}
}

/* 	Funziona che pulisce la pila.
	La pila è riempita di "-2".
	Restituisce un int per mantenere felice GCC. */
int empty_pila()
{
	int i;
	for (i=0;i<N_Nodi;i++)
	{
		if (pila_comuni[i]==-2)
			break;
		pila_comuni[i]=-2;
	}
	return 0;
}

/*  Funzione che copia pila_comuni nel vettore clique.
	La pila viene copiata solo se la sua dimensione è maggiore dell'attuale clique.
	Non mi preoccupo quindi di pulirla.
	Restituisce un int per mantenere felice GCC. */
int copy_pila()
{
	int i;
	for (i=0;i<N_Nodi;i++)
	{
		if (pila_comuni[i]==-2)
			break;
		clique[i]=pila_comuni[i];
	}
	return 0;
}

/* CliqueME */
void cliqueme()
{
	int i;
	int y;
	int z;
	int adiacente;
	int nodo_da_pila;
	int indice_da_pila;
	struct nodoLista *list_jumper=NULL;
	struct nodoLista *list_jumper_adiacente=NULL;
	//Creo il vettore clique
	clique=(int *)malloc(N_Nodi*sizeof(int));
	if (clique==NULL)
	{
		printf("Non sono riuscito ad allocare il vettore clique in memoria");
		exit(1);
	}
	//Creo la pila
	pila_comuni=(int *)malloc(N_Nodi*sizeof(int));
	if (pila_comuni==NULL)
	{
		printf("Non sono riuscito ad allocare il vettore pila_comuni in memoria");
		exit(1);
	}
	//Li inizializzo
	for (i=0;i<N_Nodi;i++)
	{
		clique[i]=-2;
		pila_comuni[i]=-2;
	}
	//Considero un nodo
	printf("\nAnalisi in corso dei seguenti nodi:\n");
	for (i=0;i<N_Nodi;i++)
	{
		if (vettore_nodi[i]->N_Archi_Locali>=(N_Clique-1))
		{
		printf("%d ",i+1);
		list_jumper=(vettore_nodi[i]->testa);
		while (list_jumper!=NULL)
		{
			//Inserisco il nodo in pila
			push_pila(i+1);
			//Inserisco anche l'adiacente considerato
			push_pila(list_jumper->nodo->ID_Nodo);
			list_jumper_adiacente=list_jumper->nodo->testa;
			while (list_jumper_adiacente!=NULL)
			{
				adiacente=list_jumper_adiacente->nodo->ID_Nodo;
				if ((adiacente!=(i+1)) && (vettore_nodi[i]->vettore_adiacenza[adiacente-1]==1) && (vettore_nodi[adiacente-1]->N_Archi_Locali>=(N_Clique-1)))
					push_pila(adiacente);
				list_jumper_adiacente=list_jumper_adiacente->next;
			}
			//Ho riempito la pila_comuni con il nodo in considerazione, il suo adiacente e tutti quelli che entrambi hanno in comune.
			z=2;
			for (y=2;y<N_Nodi;y++)
			{
				if (pila_comuni[y]==-2)
					break;
				//Prendo in considerazione un nodo inserito nella pila dei comuni
				nodo_da_pila=pila_comuni[y];
				//Elimino quelli che non sono in comune anche con lui
				for (z=y+1;z<N_Nodi;z++)
				{
					if (pila_comuni[z]==-2)
						break;
					indice_da_pila=pila_comuni[z];
					if (vettore_nodi[nodo_da_pila-1]->vettore_adiacenza[indice_da_pila-1]!=1)
					{
						pop_pila(z);
						z--;
					}
				}
			}
			if (z>N_Clique)
			{
					copy_pila();
					N_Clique=z;
			}
			list_jumper=list_jumper->next;
			empty_pila();
		}
		}
		else
			printf("Not%d ",i+1);
	}
}

int main()
{
	int i=0;
	time_t inizio=0;
	fileread();
	initprimarystruct();
	dataload();
	inizio=time(NULL);
	cliqueme();
	printf("\n\nTempo: %ds La CliqueME e' di %d nodi, coposta da: ",(time(NULL)-inizio),N_Clique);
	while (clique[i]!=-2)
	{
		printf("%d ",clique[i]);
		i++;
	}
	return 0;
}
