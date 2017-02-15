#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
//#include <windows.h>


//Struct, für ein einzelnes Feld
typedef struct
{
	bool moeglichkeiten[8];
	bool besucht[8];
	int nachfolger[8];
	int aktuellerNachfolger;
	int vorherigesx;
	int vorherigesy;
	int nachfolgendesx;
	int nachfolgendesy;
} feld;

typedef struct //Brettstruktur
{
	int n;
	int startx;
	int starty;
	int aktuellesx;
	int aktuellesy;
	bool geschlossen;
	bool benutzereingabe;
	int anzahlzuege;
	int maxfeldnr;
	int **feldzugfolge;
	feld **felder;

	//Backtracking
	int anzahlbacktracking;
	int vorletztesbackx;
	int vorletztesbacky;
	int letztesbackx;
	int letztesbacky;
} brettattrs;

const short moeglichkeitenx[8] = { -2, -2, -1, 1, 2, 2, 1, -1 };
const short moeglichkeiteny[8] = { -1, 1, 2, 2, 1, -1, -2, -2 };
bool debug = false;
//bool debug = true;

/**
Berechnet, ob eine gewisse Position (x,y) innerhalb oder außerhalb eines n*n Brettes liegt
Rückgabewerte: true=liegt innerhalb; false=liegt außerhalb
Übergabeparameter: Position x,y; Feldgröße(nxn)
*/
bool getPositionInnerhalbFeld(int x, int y, int feldgroesse)
{
	return ((x >= 0 && x < feldgroesse) && (y >= 0 && y < feldgroesse));
}

//Führt alle notwendigen Eingaben durch
void getEingaben(brettattrs *board)
{

	printf("Start Springerproblem:\n");
	int n = 0;
	int minn = 5;
	int maxn = 20;
	do
	{
		printf("Bitte Feldgroesse (n x n) eingeben(%d<n<%d): ", minn, maxn);
		scanf("%d", &n);
		if ((n < minn || n > maxn))
		{
			printf("Falsche Eingabe, bitte erneut versuchen!\n");
		}
		else
		{
			board->n = n;
		}
		getchar();
	} while (n < minn || n > maxn);

	//Zugfolge geschlossen oder nicht
	int d = 0;
	if (n % 2 == 1)
	{
		printf("Da n ungerade ist, kann die Zugfolge nicht geschlossen sein!\n");
		board->geschlossen = false;
	}
	else
	{
		char c;
		do
		{
			printf("Soll die berechnete Zugfolge geschlossen sein ? (j oder n): ");
			scanf(" %c", &c);
			switch (c)
			{
			case 'j':
			case '1':
				d = 1;
				board->geschlossen = true;
				break;
			case 'n':
			case '0':
				d = 1;
				board->geschlossen = false;
				break;
			default:
				printf("Keine Guelitige Eingabe. Bitte erneut versuchen!\n");
			}
			getchar();
		} while (d != 1);
	}

	//Benutzereingabe des Startfeldes oder Auswahl eines zufälligen Feldes durch den PC
	d = 0;
	do
	{
		char c;
		printf("Soll der Computer das Startfeld auswaehlen? (j oder n): ");
		scanf(" %c", &c);
		switch (c)
		{
		case 'j':
		case '1':
			board->benutzereingabe = false;
			//Zufälliges Startfeld berechnen  --> abhängig von Zeit, da sonst immer die selben Werte herauskommen
			srand(time(NULL));
			board->startx = rand() % board->n;
			board->starty = rand() % board->n;
			d = 1;
			break;
		case 'n':
		case '0':
			board->benutzereingabe = true;
			int e = 0; //    0
			int x = 0, y = 0;
			do
			{
				printf("\t Bitte Startfeld eingeben(Hinweis: 1 1 entspricht oben links): ");
				scanf("%d %d", &x, &y);
				if (x > 0 && x <= board->n && y > 0 && y <= board->n)
				{
					board->startx = x - 1;
					board->starty = y - 1;
					e = d = 1;
				}
				else
				{
					printf("\tKein gueltiges Startfeld eingegeben! Bitte erneut versuchen!\n");
				}
				//                getchar();
			} while (e != 1);
			d = 1;
			break;
		default:
			printf("Keine gueltige Eingabe. Bitte erneut versuchen!\n");
			break;
		}
		getchar();
	} while (d != 1);
	board->aktuellesx = board->startx;
	board->aktuellesy = board->starty;
}
//Gibt die berechnete Zugfolge aus
void printFeld(brettattrs *board)
{
	for (int i = 0; i < board->n; i++)
	{
		for (int o = 0; o < board->n; o++)
		{
			printf("%10d", board->feldzugfolge[i][o]);
		}
		printf("\n");
	}
}

/**
Gibt die Anzahl der nicht besuchten Felder(leere Felder) zurück
*/
int getAnzahlLeereFelder(brettattrs board)
{
	return (board.n*board.n - board.maxfeldnr);
}

//Berechnet die Möglichkeiten, die ein Springer auf einem bestimmten Feld hat und
//ebenso die Anzahl der darauffolgenden Züge (s.Wandorf)
void getMoeglichkeiten(int x, int y, brettattrs *board)
{
	//Add backtrack Berücksichtigung
	int tempx1, tempx2, tempy1, tempy2;
	for (int i = 0; i < 8; i++)
	{
		tempx1 = x + moeglichkeitenx[i];
		tempy1 = y + moeglichkeiteny[i];

		if (getPositionInnerhalbFeld(tempx1, tempy1, board->n))
		{
			if (board->feldzugfolge[tempx1][tempy1] == 0)
			{
				if (board->felder[x][y].besucht[i] == false)
				{
					{
						board->felder[x][y].moeglichkeiten[i] = true;
						//Anzahl der darauffolgenden Züge berechnen
						board->felder[x][y].nachfolger[i] = 0;
						for (int o = 0; o < 8; o++)
						{
							tempx2 = tempx1 + moeglichkeitenx[o];
							tempy2 = tempy1 + moeglichkeiteny[o];
							if (getPositionInnerhalbFeld(tempx2, tempy2, board->n))
							{
								if (board->feldzugfolge[tempx2][tempy2] == 0)
								{
									board->felder[x][y].nachfolger[i] += 1;
								}
							}
						}
					}
				}

			}
		}
	}
}


//Gibt den Zug(0-7)(siehe moeglichkeitenx/moeglichkeiteny) mit den am wenigsten darauffolgenden Zügen zurück
int getPriorisiertenZug(brettattrs board)
{
	int index = -1;
	int count = 8;  //8, da es maximal 8 Züge gibt
	for (int i = 0; i < 8; i++)
	{
		if (board.felder[board.aktuellesx][board.aktuellesy].nachfolger[i] < count && board.felder[board.aktuellesx][board.aktuellesy].moeglichkeiten[i] == true)
		{
			index = i;
			count = board.felder[board.aktuellesx][board.aktuellesy].nachfolger[i];
		}
	}
	return index;
}


//Gibt die Anzahl der Möglichkeiten für ein Feld zurück
int getAnzahlMoeglichkeitenXY(brettattrs board)
{
	int anzahl = 0;
	for (int i = 0; i <= 7; i++)
		if (board.felder[board.aktuellesx][board.aktuellesy].moeglichkeiten[i]) anzahl++;
	return anzahl;
}


//NUR FÜR DEBUG
void printMoeglichkeiten(int x, int y, brettattrs board)
{
	for (int i = 0; i < 8; i++)
	{
		printf("moeg %d: %s --> %d  Nachfolger\n", i, board.felder[x][y].moeglichkeiten[i] ? "true" : "false", board.felder[x][y].nachfolger[i]);
	}
	printf("\n");
}


//Gibt an, ob die vorliegende Zugfolge geschlossen ist oder nicht
bool getIstGeschlossen(brettattrs board)
{
	int max = board.n * board.n;
	if (getAnzahlLeereFelder(board) == 0)
	{
		for (int i = 0; i < board.n; i++)
		{
			for (int o = 0; o < board.n; o++)
			{
				if (board.feldzugfolge[i][o] == max)
				{
					for (int p = 0; p < 8; p++)
					{
						if (i + moeglichkeitenx[p] == board.startx && o + moeglichkeiteny[p] == board.starty)
						{
							return true;
						}

					}
				}
			}
		}
	}
	return false;
}

int getAnzahlBelegtUmStartFeld(brettattrs board)
{
	int count = 0;
	for (int i = 0; i < 8; i++)
	{
		int tempx = board.startx + moeglichkeitenx[i];
		int tempy = board.starty + moeglichkeiteny[i];
		if (getPositionInnerhalbFeld(tempx, tempy, board.n) && board.feldzugfolge[tempx][tempy] == 0)
		{
			count++;
		}
	}
	return count;
}

void resetBesucht(int x, int y, brettattrs *board)
{
	for (int i = 0; i < 8; i++)
		board->felder[x][y].besucht[i] = false;
}

void moveBack(brettattrs *board)
{
	int altesx = board->aktuellesx;
	int altesy = board->aktuellesy;

	board->vorletztesbackx = board->letztesbackx;
	board->vorletztesbacky = board->letztesbacky;

	board->letztesbackx = board->aktuellesx;
	board->letztesbacky = board->aktuellesy;

	if (board->vorletztesbackx != -1 && board->vorletztesbacky != -1)
		resetBesucht(board->vorletztesbackx, board->vorletztesbacky, board);

	board->aktuellesx = board->felder[altesx][altesy].vorherigesx;
	board->aktuellesy = board->felder[altesx][altesy].vorherigesy;
	board->maxfeldnr -= 1;
	board->anzahlzuege += 1;
	board->anzahlbacktracking += 1;
	board->feldzugfolge[altesx][altesy] = 0;
}

void moveForward(brettattrs *board)
{
	//Nachfolgenden Zug(index in moeglichkeitenx / moeglichkeiteny) bestimmen
	int index = getPriorisiertenZug(*board);
	//Aktuelle Position abspeichern (Ausgangsposition)
	int altesx = board->aktuellesx;
	int altesy = board->aktuellesy;

	board->felder[altesx][altesy].aktuellerNachfolger = index;
	//Neue Position abspeichern
	board->aktuellesx += moeglichkeitenx[index];
	board->aktuellesy += moeglichkeiteny[index];

	board->maxfeldnr += 1;
	board->anzahlzuege += 1;
	//Zugnummer in Schachbrett eintragen
	board->feldzugfolge[board->aktuellesx][board->aktuellesy] = board->maxfeldnr;

	if (board->aktuellesx == board->startx && board->aktuellesy == board->starty)
	{
		board->felder[board->aktuellesx][board->aktuellesy].vorherigesx = board->startx;
		board->felder[board->aktuellesx][board->aktuellesy].vorherigesy = board->starty;
	}
	else
	{
		board->felder[board->aktuellesx][board->aktuellesy].vorherigesx = altesx;
		board->felder[board->aktuellesx][board->aktuellesy].vorherigesy = altesy;
	}

	//Aktuelle Positionen in das vorherige Feld eintragen
	board->felder[altesx][altesy].nachfolgendesx = board->aktuellesx;
	board->felder[altesx][altesy].nachfolgendesy = board->aktuellesy;

	board->felder[altesx][altesy].besucht[index] = true;
	board->felder[altesx][altesy].moeglichkeiten[index] = false;
	//    printf("belegt um startfeld: %d\n", getAnzahlBelegtUmStartFeld(*board));
}


//berechnet die Zugfolge des Springers
void berechneZugfolge(brettattrs *board)
{

	bool exit = false;
	while (exit == false)
	{
		getMoeglichkeiten(board->aktuellesx, board->aktuellesy, board);
		if (debug) printMoeglichkeiten(board->aktuellesx, board->aktuellesy, *board);
		if (debug) printf("x: %d, y: %d\n", board->aktuellesx, board->aktuellesy);
		if (getAnzahlMoeglichkeitenXY(*board) > 0 && getAnzahlLeereFelder(*board) != 0 && getAnzahlBelegtUmStartFeld(*board) != 0)
		{
			if (debug) printf("forw: %d x: %d y: %d\n", getAnzahlMoeglichkeitenXY(*board), board->aktuellesx, board->aktuellesy);
			moveForward(board);
			if (debug) printf("---------\n");
			if (debug) printFeld(board);
		}
		else
		{
			if (debug) printf("back: %d x: %d y: %d\n", getAnzahlMoeglichkeitenXY(*board), board->aktuellesx, board->aktuellesy);
			if (debug) printf(" ---------\n");
			moveBack(board);
			if (debug) printFeld(board);
		}
		//        Überprüfung, ob Berechnung fertig ist oder nicht
		if (board->geschlossen == true && getAnzahlLeereFelder(*board) == 0)
		{
			if (getIstGeschlossen(*board) == true)
			{
				exit = true;
			}
			else
			{
				if (debug) printf("back\n");
				moveBack(board);
				if (debug) printf("---------\n");
				if (debug) printFeld(board);
			}
		}
		else if (board->geschlossen == false && getAnzahlLeereFelder(*board) == 0)
		{
			exit = true;
		}
		if (exit)
		{
			printf("Done...\n\n\n");
		}
		if (debug) scanf("ENTER");
		if (debug) getchar();
	}
}

int main()
{
	brettattrs board;
	//Benutzereingaben
	getEingaben(&board);
	//Speicher allokieren
	board.feldzugfolge = calloc(board.n, sizeof(int *));
	board.felder = calloc(board.n, sizeof(feld *));

	for (int i = 0; i < board.n; i++)
	{
		board.feldzugfolge[i] = calloc(board.n, sizeof(int));
		board.felder[i] = calloc(board.n, sizeof(feld));
	}

	board.maxfeldnr = 1;
	board.feldzugfolge[board.startx][board.starty] = board.maxfeldnr;

	board.letztesbackx = -1;
	board.letztesbacky = -1;

	board.vorletztesbackx = -1;
	board.vorletztesbacky = -1;
	board.anzahlbacktracking = 0;

	int timer = clock();

	berechneZugfolge(&board);

	timer = clock() - timer;

	printFeld(&board);
	printf("\n");
	if (debug) printf("Leere Felder: %d\n", getAnzahlLeereFelder(board));
	printf("Zugfolge berechnet in %dms\n", timer);
	//Speicher freigeben
	for (int i = 0; i < board.n; i++)
	{
		free(board.feldzugfolge[i]);
		free(board.felder[i]);
	}
	free(board.feldzugfolge);
	free(board.felder);
	return 0;
}
