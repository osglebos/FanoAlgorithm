#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <conio.h>
#include <math.h>
#include <windows.h>
#define MAX_CODE 44
#define MAX_POW 100000

using namespace std;

struct Letter
{
	double freq;
	char let;
	Letter *next;
};

struct CODE
{
	char *code;
	char let;
};

struct TREE
{
	double freq;
	char let;
	TREE *left;
	TREE *right;
};

#pragma region Check

int CheckSum(char *Filename)
{
	int sum = 0;
	int c = 0;
	FILE *File = fopen(Filename, "rt");

	if (File == NULL)
	{
		fprintf(stderr, "Could not open %s file\n", Filename);
		return -1;
	}

	while ((c = getc(File)) != EOF) sum += c;

	fclose(File);
	return sum;
}

int Check3(char *Filename)
{
	int sum = 0;
	int c = 0;
	FILE *File = fopen(Filename, "rt");

	if (File == NULL)
	{
		fprintf(stderr, "Could not open %s file\n", Filename);
		return -1;
	}

	while ((c = getc(File)) != EOF) sum++;

	fclose(File);
	return sum;
}

int Check(char *inFilename, char *outFilename)
{
	FILE *inFile = fopen(inFilename, "rt");
	FILE *outFile = fopen(outFilename, "rt");

	if (inFile == NULL)
	{
		fprintf(stderr, "Could not open %s file\n", inFilename);
		return 0;
	}
	if (outFile == NULL)
	{
		fprintf(stderr, "Could not open %s file\n", outFilename);
		return 0;
	}
	char fr, to;
	while (((fr = getc(inFile)) != EOF) && ((to = getc(outFile)) != EOF))
		if (fr != to)
			return 0;

	fclose(inFile);
	fclose(outFile);
	return 1;
}

#pragma endregion

#pragma region Lists

void AddLetter(Letter **L, double freq, int let)
{
	Letter *newElement = new Letter;
	if (newElement == 0)
	{
		fprintf(stderr, "Could not allocate %u bytes for the sovature\n", sizeof(Letter));
		return;
	}
	while (*L != NULL)
		if ((*L)->freq > freq)
			L = &((*L)->next);
		else
			break;
	newElement->let = let;
	newElement->freq = freq;
	newElement->next = (*L);
	*L = newElement;
}

void PushBack(Letter **L, double freq, int let)
{
	Letter *newElement;
	newElement = new Letter;
	if (newElement == 0)
	{
		fprintf(stderr, "Could not allocate %u bytes for the sovature\n", sizeof(Letter));
		return;
	}
	while (*L != NULL)
		L = &((*L)->next);

	newElement->let = let;
	newElement->freq = freq;
	newElement->next = NULL;
	*L = newElement;
}

void BubbleSort(Letter *letters)
{
	for (Letter *i = letters; i != NULL; i = i->next)
	{
		for (Letter *j = i; j != NULL; j = j->next)
		{
			if (j->freq > i->freq)
			{
				Letter tmp = *i;
				i->freq = j->freq;
				i->let = j->let;
				j->freq = tmp.freq;
				j->let = tmp.let;
			}
		}
	}
}

Letter *FindLetter(Letter *letters, char c)
{
	for (Letter *i = letters; i != NULL; i = i->next)
		if (c == i->let)
			return i;
	return NULL;
}

char *FindCode(char c, CODE *Codes, int codes_size)
{
	for (int i = 0; i < codes_size; i++) 
		if (c == Codes[i].let) return Codes[i].code;
	return Codes[0].code;
}
#pragma endregion 

#pragma region Tree

TREE *Build(Letter *a)
{
	if (a != NULL)
	{
		TREE *head = new TREE;
		if (a->next == NULL)
		{
			head->freq = a->freq;
			head->left = NULL;
			head->right = NULL;
			head->let = a->let;
		}
		else
		{
			Letter *first = a;
			Letter *second = a;
			int count = 0;
			head->let = '-';
			head->freq = 0;
			for (Letter *j = a; j != NULL; j = j->next)	head->freq += j->freq;

			for (second = a; second->next->next != NULL; second = second->next)
			{
				double ffreq = 0, sfreq = 0;
				for (Letter *j = a; j != second->next->next; j = j->next) ffreq += j->freq;
				for (Letter *j = second->next->next; j != NULL; j = j->next) sfreq += j->freq;

				if (ffreq >= sfreq)	break;
			}

			Letter *tmp = second->next;
			second->next = NULL;
			second = tmp;

			head->left = Build(first);
			head->right = Build(second);
		}
		return head;
	}
	else return NULL;
}

void DisplayTurn(TREE *T, int dep)
{
	if (T != NULL)
	{
		if (T->right != NULL)
			DisplayTurn(T->right, dep + 1);

		for (int i = 0; i <= dep * 3; i++)printf(" ");
		printf("%1.6lf (%c)\n", T->freq, T->let);

		if (T->left != NULL)
			DisplayTurn(T->left, dep + 1);
	}
}
#pragma endregion

#pragma region Encode

void PrintCodesToFile(FILE *inF, FILE *outF, Letter *a, CODE *Codes, int codes_size)
{
	int c = 0;
	fprintf(outF, "%i\n", codes_size);

	for (Letter *i = a; i != NULL; i = i->next) 
		fprintf(outF, "'%c' %lf\n", i->let, i->freq);

	fseek(inF, 0, SEEK_SET);
	while ((c = getc(inF)) != EOF)	
		fprintf(outF, "%s", FindCode(c, Codes, codes_size));
	fseek(inF, 0, SEEK_SET);
}

Letter *CountFreq(FILE *F)
{
	Letter *letters = NULL;
	int c = 0;
	int sum = 0;

	while ((c = getc(F)) != EOF) {
		Letter *tmp = FindLetter(letters, c);
		if (tmp != NULL)
		{
			tmp->freq++;
			BubbleSort(letters);
		}
		else AddLetter(&letters, 1, c);
		sum++;
	}

	for (Letter *i = letters; i != NULL; i = i->next)
		i->freq = floor(i->freq * MAX_POW / sum) / MAX_POW;
			
	fseek(F, 0, SEEK_SET);
	return letters;
}

void TraverseTREE(TREE *T, int deph, CODE *Codes, char *str, int *num, int codes_size)
{
	if (T != NULL)
	{
		if (!T->left || !T->right)
		{
			char *tmp_code = new char[deph + 1];
			for (int i = 0; i < deph + 1; i++) tmp_code[i] = str[i];
			tmp_code[deph] = 0;

			if ((*num) < codes_size)
			{
				Codes[(*num)].let = T->let;
				Codes[(*num)].code = _strdup(tmp_code);
			}
			else printf("Error in calculating Codes.\n");

			*num = (*num)++;
		}
		else
		{
			str[deph] = '1';
			if (T->right != NULL)
				TraverseTREE(T->right, deph + 1, Codes, str, num, codes_size);
			str[deph] = '0';
			if (T->left != NULL)
				TraverseTREE(T->left, deph + 1, Codes, str, num, codes_size);

		}
	}
}

int EncodeFunc(char *inFilename, char *outFilename) {
	Letter *letters = NULL, *letters_dup = NULL;
	TREE *root = NULL;
	CODE *Codes;
	int codes_size = 0, num = 0;
	char *str = new char[MAX_CODE];
	FILE *inFile = fopen(inFilename, "rt");
	FILE *outFile = fopen(outFilename, "wt");

	if (inFile == NULL)
	{
		fprintf(stderr, "Could not open %s file\n", inFilename);
		return 0;
	}
	if (outFile == NULL)
	{
		fprintf(stderr, "Could not open %s file\n", outFilename);
		return 0;
	}
	letters = CountFreq(inFile);
	for (Letter *i = letters; i != NULL; i = i->next)
	{
		PushBack(&letters_dup, i->freq, i->let);
		codes_size++;
	}

	Codes = new CODE[codes_size];
	root = Build(letters);
	DisplayTurn(root, 0);
	TraverseTREE(root, 0, Codes, str, &num, codes_size);
	for (Letter *i = letters_dup; i != NULL; i = i->next) if (i->freq != 0) printf("'%c' : %lf : %s\n", i->let, i->freq, FindCode(i->let, Codes, codes_size));
	PrintCodesToFile(inFile, outFile, letters_dup, Codes, codes_size);

	fclose(inFile);
	fclose(outFile);
	return 1;
}
#pragma endregion

#pragma region Decode
Letter *Readfreq(FILE *inF)
{
	Letter *letters = NULL;
	int count = 0;

	fseek(inF, 0, SEEK_SET);
	fscanf(inF, "%i\n", &count);

	for (int i = 0; i < count; i++)
	{
		char let;
		double freq;
		fscanf(inF, "'%c' %lf\n", &let, &freq);
		PushBack(&letters, freq, let);
	}
	return letters;
}

void Decode(TREE *root, FILE *iF, FILE *oF)
{
	char c;
	TREE *T = root;

	while ((c = getc(iF)) != EOF)
	{
		if (c == '0' && T->left != NULL)
			T = T->left;
		else if (c == '1' && T->right != NULL)
			T = T->right;

		if (T->left == NULL && T->right == NULL)
		{
			fprintf(oF, "%c", T->let);
			T = root;
		}
	}
}

int DecodeFunc(char *inFilename, char *outFilename)
{
	TREE *root = NULL;
	Letter *letters = NULL;
	FILE *inFile = fopen(inFilename, "rt");
	FILE *outFile = fopen(outFilename, "wt");

	if (inFile == NULL)
	{
		fprintf(stderr, "Could not open %s file\n", inFilename);
		return 0;
	}
	if (outFile == NULL)
	{
		fprintf(stderr, "Could not open %s file\n", outFilename);
		return 0;
	}

	letters = Readfreq(inFile);

	for (Letter *i = letters; i != NULL; i = i->next)
		printf("%c %lf\n", i->let, i->freq);

	root = Build(letters);
	DisplayTurn(root, 0);
	Decode(root, inFile, outFile);

	fclose(inFile);
	fclose(outFile);
	return 1;
}
#pragma endregion

int main(void)
{
	bool exit = false;
	char key;
	char *fromwhere = new char[15];
	char *towhere = new char[15];

	while (!exit)
	{
		printf("\n\n");
		printf("esc | Exit\n");
		printf("1   | Encode Func\n");
		printf("2   | Start File\n");
		printf("3   | Decode Func\n");
		printf("4   | Check Func\n");

		key = _getch();
		switch (key)
		{
		case 27:
			printf("Exit.\n");
			exit = true;
			break;

		case '1':
			strcpy(fromwhere, "start.txt");
			strcpy(towhere, "coverted.fan");
			if (EncodeFunc(fromwhere, towhere) == 1) printf("File %s sucsessfuly encdoded to %s.\n", fromwhere, towhere);
			else printf("Something went wrong.\n");
			break;

		case '2':
		{
			FILE *inFile = fopen("start.txt", "wt");
			fprintf(inFile, "mama papa!");
			fclose(inFile);
			printf("File sucsessfuly had been created.\n");
		}
		break;

		case '3':
			strcpy(fromwhere, "coverted.fan");
			strcpy(towhere, "finish.txt");
			if (DecodeFunc(fromwhere, towhere) == 1) printf("File %s sucsessfuly decoded to %s.\n", fromwhere, towhere);
			else printf("Something went wrong.\n");
			break;

		case '4':
		{
			strcpy(fromwhere, "start.txt");
			strcpy(towhere, "finish.txt");

			int from = CheckSum(fromwhere), to = CheckSum(towhere);
			printf("Amount of every symbol code:         ");
			if (from == to) printf("Everything is grate(%i==%i)!!!\n", from, to);
			else printf("Something went wrong(%i!=%i).\n", from, to);

			printf("Character-by-character comparison:   ");
			if (Check(fromwhere, towhere) == 1) printf("Everything is grate!!!\n");
			else printf("Something went wrong.\n");

			from = Check3(fromwhere), to = Check3(towhere);
			printf("Comparing the number of characters:  ");
			if (from == to) printf("Everything is grate(%i==%i)!!!\n", from, to);
			else printf("Something went wrong.\n");

			break;
		}

		default:
			printf("Wrong symbol\n");
			break;
		}
	}
	return 0;
}
