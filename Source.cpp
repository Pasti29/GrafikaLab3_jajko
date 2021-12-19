//*********************************************************************
//
//  PLIK ŹRÓDŁOWY:		Source.cpp
//
//  OPIS:				Program do Laboratorium nr 3 rysujący na
//						ekranie obiekt 3-D i manipuluje widokiem niego.
//
//  AUTOR:				Karol Pastewski, 252798@student.edu.pl
//
//  DATA				30.10.2021r.
//	MODYFIKACJI:
//
//  PLATFORMA:			System operacyjny:  Microsoft Windows 11
//						Środowisko:         Microsoft Visual 2019
//
//  MATERIAŁY			Dokumentacja OpenGL
//	ŹRÓDŁOWE:			Dokumentacja GLUT
//						www.zsk.ict.pwr.wroc.pl
//
//  UŻYTE BIBLIOTEKI	cmath — obsługuje matematyczne wzory i stałe
//  NIESTANDARDOWE
//
//*********************************************************************

#define _USE_MATH_DEFINES

#include <windows.h>
#include <gl/gl.h>
#include <gl/glut.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cmath>


using point3 = float[3];
//typedef GLfloat point3[3];

struct VERTEXES {
	GLfloat point3[3];
	GLfloat color3[3];
};

VERTEXES** array;		// niezainicjalizowana tablica dwuwymiarowa; patrz funkcję 'setArray()'
int N;					// wielkość tablicy zainicjowana w funkcji 'setArray()'
int model = 1;			// 1 - model chmury punków;  2 - model siatki;  3 - model wypełnionych trójkątów
int rotationMode = 0;	// 0 - brak rotacji;  1 - rotacja po x;  2 - rotacja po y;  3 - rotacja po z;  4 - rotacja po x, y i z
int showAxes = 1;		// 0 - brak osi współrzędnych;  1 - rysowanie osi współrzędnych
int objectMode = 1;		// 1 - wyświetlenie jajka;  2 - wyświetlenie czajnika

static GLfloat currentRotation[] = { 0.0, 0.0 ,0.0 };		// aktualny kąt obrotu wokół odpowiednio osi OX, OY i OZ

// Funkcja obliczająca obecny stan obrotu
void calcRotation() {
	switch (rotationMode) {
	case 1:
		currentRotation[0] += 0.5;
		if (currentRotation[0] > 360.0) currentRotation[0] -= 360.0;
		break;

	case 2:
		currentRotation[1] += 0.5;
		if (currentRotation[1] > 360.0) currentRotation[1] -= 360.0;
		break;

	case 3:
		currentRotation[2] += 0.5;
		if (currentRotation[2] > 360.0) currentRotation[2] -= 360.0;
		break;

	case 4:
		currentRotation[0] += 0.5;
		if (currentRotation[0] > 360.0) currentRotation[0] -= 360.0;
		currentRotation[1] += 0.5;
		if (currentRotation[1] > 360.0) currentRotation[1] -= 360.0;
		currentRotation[2] += 0.5;
		if (currentRotation[2] > 360.0) currentRotation[2] -= 360.0;
		break;
	default:
		break;
	}

	glutPostRedisplay();		// odświeżenie zawartości aktualnego okna
}

// Funkcja rysująca osie układu współrzędnych
void drawAxes() {
	point3  x_min = { -5.0, 0.0, 0.0 };
	point3  x_max = { 5.0, 0.0, 0.0 };
	// początek i koniec obrazu osi x

	point3  y_min = { 0.0, -5.0, 0.0 };
	point3  y_max = { 0.0,  5.0, 0.0 };
	// początek i koniec obrazu osi y

	point3  z_min = { 0.0, 0.0, -5.0 };
	point3  z_max = { 0.0, 0.0,  5.0 };
	//  początek i koniec obrazu osi y

	glColor3f(1.0f, 0.0f, 0.0f);  // kolor rysowania osi - czerwony
	glBegin(GL_LINES); // rysowanie osi x

	glVertex3fv(x_min);
	glVertex3fv(x_max);

	glEnd();

	glColor3f(0.0f, 1.0f, 0.0f);  // kolor rysowania - zielony
	glBegin(GL_LINES);  // rysowanie osi y

	glVertex3fv(y_min);
	glVertex3fv(y_max);

	glEnd();

	glColor3f(0.0f, 0.0f, 1.0f);  // kolor rysowania - niebieski
	glBegin(GL_LINES); // rysowanie osi z

	glVertex3fv(z_min);
	glVertex3fv(z_max);

	glEnd();

}


// Funkcja rysująca każdy punkt w tabeli 'array'
void drawPoints() {
	glBegin(GL_POINTS);

	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			glVertex3fv(array[i][j].point3);
		}
	}

	glEnd();
}

// Funkcja rysująca linie pomiędzy punktami w tabeli 'array'
void drawLines() {

	for (int i = 0; i < N - 1; i++) {
		for (int j = 0; j < N - 1; j++) {
			// linia pionowa
			glBegin(GL_LINES);
			glVertex3fv(array[i][j].point3);
			glVertex3fv(array[i + 1][j].point3);
			glEnd();
			// linia pozioma
			glBegin(GL_LINES);
			glVertex3fv(array[i][j].point3);
			glVertex3fv(array[i][j + 1].point3);
			glEnd();

			// linie ukośne
			glBegin(GL_LINES);
			glVertex3fv(array[i][j].point3);
			glVertex3fv(array[i + 1][j + 1].point3);
			glEnd();
			glBegin(GL_LINES);
			glVertex3fv(array[i][j + 1].point3);
			glVertex3fv(array[i + 1][j].point3);
			glEnd();
		}
	}
}

// Funkcja rysująca trójkąty połączone z punktów z tabeli 'array'
//
// GL_TRIANGLE_FAN - pierwszy wierzchołek jest stały, jest on później
// łączony z grupami dwóch kolejnych wierzchołków w trójkąt, czyli np.
// dla 5 wierzchołków wyjdą trójkąty (0, 1, 2) i (0, 3, 4)
void drawTriangles() {
	for (int i = 0; i < N - 1; i++) {
		for (int j = 0; j < N - 1; j++) {
			glBegin(GL_TRIANGLE_FAN);

			glColor3fv(array[i][j].color3);
			glVertex3fv(array[i][j].point3);

			glColor3fv(array[i][j + 1].color3);
			glVertex3fv(array[i][j + 1].point3);

			glColor3fv(array[i + 1][j + 1].color3);
			glVertex3fv(array[i + 1][j + 1].point3);

			glColor3fv(array[i + 1][j].color3);
			glVertex3fv(array[i + 1][j].point3);

			glColor3fv(array[i + 1][j + 1].color3);
			glVertex3fv(array[i + 1][j + 1].point3);

			glEnd();
		}
	}

}

// Funkcja obsługująca rysowanie jajka
void Egg() {

	glColor3f(0.0, 1.0, 1.0);	// ustawienie koloru na biały
	glTranslated(0.0, -(array[(N - 1) / 2][0].point3[1] / 2), 0.0);	// wyśrodkowanie obiektu

	if (model == 1) {
		drawPoints();

	} else if (model == 2) {
		drawLines();
	} else {
		drawTriangles();
	}
}

// Funkcja obsługująca rysowanie czajnika
void Teapot() {
	glColor3f(1.0, 1.0, 1.0);	// ustawienie koloru na biały
	glutWireTeapot(4.0);
}


void renderScene() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// czyszczenie okna aktualnym kolorem czyszczącym

	glLoadIdentity();	// czyszczenie macierzy bieżącej

	glRotatef(currentRotation[0], 1.0, 0.0, 0.0);		// glRotatef(angle, x, y, z) - funkcja obracająca o
	glRotatef(currentRotation[1], 0.0, 1.0, 0.0);		// kąt 'angle' woół osi (0, 0, 0) i punktu
	glRotatef(currentRotation[2], 0.0, 0.0, 1.0);		// (x, y, z)

	if (showAxes == 1) {
		drawAxes();
	}

	if (objectMode == 1) {
		Egg();
	} else {
		Teapot();
	}

	glFlush();		// przekazanie poleceń rysujących do wykonania

	glutSwapBuffers();
}

// Inicjalizacja losowych kolorów dla każdego punktu w tabeli 'array'
void setColors() {
	srand(time(nullptr));		// inicjalizacja generatora losowych liczb
	float red, green, blue;
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			red = (GLfloat)(rand() / (GLfloat)RAND_MAX);		// losowanie liczb
			green = (GLfloat)(rand() / (GLfloat)RAND_MAX);		// zakres liczb: <0.0; 1.0>
			blue = (GLfloat)(rand() / (GLfloat)RAND_MAX);		//
			array[i][j].color3[0] = red;
			array[i][j].color3[1] = green;
			array[i][j].color3[2] = blue;
		}
	}
}

// Inicjalizacja współrzędnych punktów potrzebnych do wyświetlenia jajka
void setVertices() {
	for (int i = 0; i < N; i++) {
		float u = (float)i / (N - 1);
		float uPow5 = pow(u, 5);
		float uPow4 = pow(u, 4);
		float uPow3 = pow(u, 3);
		float uPow2 = pow(u, 2);
		GLfloat x, y, z;

		y = 160 * uPow4 - 320 * uPow3 + 160 * uPow2;

		for (int j = 0; j < N; j++) {
			float v = (float)j / (N - 1);

			x = (-90 * uPow5 + 225 * uPow4 - 270 * uPow3 + 180 * uPow2 - 45 * u) * cos(M_PI * v);
			z = (-90 * uPow5 + 225 * uPow4 - 270 * uPow3 + 180 * uPow2 - 45 * u) * sin(M_PI * v);

			array[i][j].point3[0] = x;
			array[i][j].point3[1] = y;
			array[i][j].point3[2] = z;
		}
	}
}

// Funkcja inicjalizuje tablicę 'array' oraz wywołuje funkcje inicjalizujące
// kolory i współrzędne punktów
void setArray() {
	std::cout << "Podaj liczbe N = ";
	std::cin >> N;
	VERTEXES** newArray = new VERTEXES * [N];
	for (int i = 0; i < N; i++) {
		newArray[i] = new VERTEXES[N];
	}
	array = newArray;
	setColors();
	setVertices();
}


// Funkcja wyświetlająca w konsoli informacje o opcjach programu
void initProgram() {
	std::cout << "Program z laboratorium 3 - modelowanie obiektow 3-D\n";
	setArray();
	std::cout << "Odpowiednie klawisze zmieniaja widok modelu:\n";
	std::cout << "   Wyswietlany obiekt:\n";
	std::cout << "      'f' - jajko (domyslne)\n";
	std::cout << "      'g' - czajnik\n";
	std::cout << "   Model wyswietlania jajka (nie dziala dla obiektu czajnika):\n";
	std::cout << "      '1' - model chmury punktow (domyslne)\n";
	std::cout << "      '2' - model siatki\n";
	std::cout << "      '3' - model wypelnionych trojkatow\n";
	std::cout << "   Tryb rotacji obiektu:\n";
	std::cout << "      'q' - brak rotacji (domyslne)\n";
	std::cout << "      'w' - rotacja po x\n";
	std::cout << "      'e' - rotacja po y\n";
	std::cout << "      'r' - rotacja po z\n";
	std::cout << "      't' - rotacja po x, y i z\n";
	std::cout << "   Wyswietlanie osi wspolrzednych:\n";
	std::cout << "      'a' - nie\n";
	std::cout << "      's' - tak (domyslne)\n";
	std::cout << "   Operowanie programem:\n";
	std::cout << "      'esc' - wyjscie z programu\n";
	std::cout << "Pamietaj, aby aktywnym oknem bylo okno OpenGL'a!!!";
}

// Funkcja obsługująca działanie programu za pomocą klawiszy klawiatury
void keys(unsigned char key, int x, int y) {
	if (objectMode == 1) {
		if (key == '1') model = 1;
		if (key == '2') model = 2;
		if (key == '3') model = 3;
	}
	if (key == 'q') rotationMode = 0;
	if (key == 'w') rotationMode = 1;
	if (key == 'e') rotationMode = 2;
	if (key == 'r') rotationMode = 3;
	if (key == 't') rotationMode = 4;
	if (key == 'a') showAxes = 0;
	if (key == 's') showAxes = 1;
	if (key == 'f') objectMode = 1;
	if (key == 'g') objectMode = 2;
	if (key == (char)27) exit(0);

	renderScene(); // przerysowanie obrazu sceny
}

// Funkcja ma za zadanie utrzymanie stałych proporcji rysowanych
// w przypadku zmiany rozmiarów okna.
// Parametry vertical i horizontal (wysokość i szerokość okna) sś
// przekazywane do funkcji za każdym razem, gdy zmieni się rozmiar okna.
void changeSize(GLsizei horizontal, GLsizei vertical) {

	GLfloat AspectRatio;
	// Deklaracja zmiennej AspectRatio  określającej proporcję
	// wymiarów okna

	if (vertical == 0)  // Zabezpieczenie przed dzieleniem przez 0

		vertical = 1;

	glViewport(0, 0, horizontal, vertical);
	// Ustawienie wielkości okna widoku (viewport)
	// W tym przypadku od (0,0) do (horizontal, vertical)

	glMatrixMode(GL_PROJECTION);
	// Przełączenie macierzy bieżącej na macierz projekcji

	glLoadIdentity();
	// Czyszcznie macierzy bieżącej

	AspectRatio = (GLfloat)horizontal / (GLfloat)vertical;
	// Wyznaczenie współczynnika  proporcji okna.
	// Gdy okno nie jest kwadratem, wymagane jest określenie tak zwanej
	// przestrzeni ograniczającej pozwalającej zachować właściwe
	// proporcje rysowanego obiektu.
	// Do określenia przestrzeni ograniczającej służy funkcja
	// glOrtho(...)

	if (horizontal <= vertical)

		glOrtho(-7.5, 7.5, -7.5 / AspectRatio, 7.5 / AspectRatio, 10.0, -10.0);

	else

		glOrtho(-7.5 * AspectRatio, 7.5 * AspectRatio, -7.5, 7.5, 10.0, -10.0);

	glMatrixMode(GL_MODELVIEW);
	// Przełączenie macierzy bieżącej na macierz widoku modelu

	glLoadIdentity();
	// Czyszczenie macierzy bieżącej

}

// Funkcja ustalająca stan renderowania
void myInit() {

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	// Kolor czyszczący (wypełnienia okna) ustawiono na czarny
	glEnable(GL_DEPTH_TEST);
	// Włączenie mechanizmu usuwania powierzchni niewidocznych

}

// Główny punkt wejścia programu. Program działa w trybie konsoli
int main() {

	initProgram();

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(600, 600);

	glutCreateWindow("Modelowanie obiektow 3-D");

	glutDisplayFunc(renderScene);
	// Określenie, że funkcja renderScene będzie funkcją zwrotną
	// (callback function).  Biedzie ona wywoływana za każdym razem
	// gdy zajdzie potrzeba przerysowania okna

	glutReshapeFunc(changeSize);
	// Dla aktualnego okna ustala funkcję zwrotną odpowiedzialną
	// za zmiany rozmiaru okna

	glutKeyboardFunc(keys);

	glutIdleFunc(calcRotation);

	myInit();
	// Funkcja myInit() (zdefiniowana powyżej) wykonuje wszelkie
	// inicjalizacje konieczne  przed przystąpieniem do renderowania

	glutMainLoop();
	// Funkcja uruchamia szkielet biblioteki GLUT

}
