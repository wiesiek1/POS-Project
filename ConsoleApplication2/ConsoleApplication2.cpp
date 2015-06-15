#include "stdafx.h"
#include "D:/Designs/ConsoleApplication/opencv2/highgui/highgui.hpp"
#include "D:/Designs/ConsoleApplication/opencv2/imgcodecs.hpp"
#include "D:/Designs/ConsoleApplication/opencv2/imgproc/imgproc.hpp"
#include "IniWriter.h"
#include "IniReader.h"
#include <iostream>
#include <stdio.h>
#include <Windows.h>
#include <process.h>
#include <iostream>
#include <sstream> 
#include <string> 
#include "iostream"

using namespace std;
using namespace cv;

/**
* \brief For debug purposes - controles memory leak.
*/
void OutputHeading(const char * explanation)
{
	_RPT1(_CRT_WARN, "\n\n%s:\n**************************************\
					 					 ************************************\n", explanation);
}

/**
* \brief Histogram equalization
*/
Mat equalization(Mat obraz)
/**
* Equalizes image's histogram. It uses the image as an input a outputs a processed one.
*/
{
	Mat ycrcb;
	Mat finito;
	cvtColor(obraz, ycrcb, CV_BGR2YCrCb);

	vector<Mat> channels;
	split(ycrcb, channels);

	equalizeHist(channels[0], channels[0]);
	merge(channels, ycrcb);
	cvtColor(ycrcb, finito, CV_YCrCb2BGR);
	return finito;
}

/**
* \brief Image changing function
*/
Mat changesize(Mat obraz)
{
	Mat finito;
	Size size(40, 40);
	resize(obraz, finito, size);
	return finito;
}

/**
* \brief Image matrix creating function
*/
Mat createOne(vector<Mat> & images, int cols, int min_gap_size)
{
	/**
	* This function takes a whole vector of loaded images and creates a matrix of images based on
	* specified number of columns and minimal gap size between them
	*/
	int max_width = 0;
	int max_height = 0;
	for (int i = 0; i < images.size(); i++) {
		max_height = max(max_height, images[i].rows);
		max_width = max(max_width, images[i].cols);
	}
	// number of images in y direction
	int rows = cols;
	//int rows = std::ceil(images.size() / cols);

	// create our result-matrix
	Mat result = Mat::zeros(rows*max_height + (rows - 1)*min_gap_size,
		cols*max_width + (cols - 1)*min_gap_size, images[0].type());
	size_t i = 0;
	int current_height = 0;
	int current_width = 0;
	for (int y = 0; y < rows; y++) {
		for (int x = 0; x < cols; x++) {
			if (i >= images.size()) // shouldn't happen, but let's be safe
				return result;
			// get the ROI in our result-image
			cv::Mat to(result,
				cv::Range(current_height, current_height + images[i].rows),
				cv::Range(current_width, current_width + images[i].cols));
			// copy the current image to the ROI
			images[i++].copyTo(to);
			current_width += max_width + min_gap_size;
		}
		// next line - reset width and update height
		current_width = 0;
		current_height += max_height + min_gap_size;
	}
	return result;
}

/**
* \brief Main image processing function
*/
void exe(string wejscie , string wyjscie)
/**
* This function takes strings which specify input and output path. It loads all images
* and processes them based on equalization, resize and createone functions.
*/
{
	Mat src, fin_img; //obraz i obraz wynikowy
	vector<Mat> obrazki1, obrazki2; //matryce obrazkow, pierwsza oryginaly, druga zmienione

	int i = 1; //warunek poczatkowy
	while (1) //glowna petla
	{
		stringstream ss;
		ss << i;
		string tmp = ss.str(); //konwersja zmiennej i na string
		string tmp2 = wejscie + tmp + ".png"; //modyfikacja sciezki o nazwe pliku wejsciowego
		string tmp3 = wyjscie + tmp + "h.png"; //modyfikacja sciezki o nazwe pliku wyjsciowego

		src = imread(tmp2, 1); //wczytaj obrazek
		if (!src.data) break; //przerwij petle jesli nic sie nie stalo

		fin_img = equalization(src); //wyrownaj histogram obrazka
		imwrite(tmp3, fin_img); //zapisz wynik pracy

		obrazki1.push_back(changesize(src)); //dodaj wczytany obrazek do matrycy miniatur
		obrazki2.push_back(changesize(fin_img)); //dodaj przerobiony obrazek do matrycy miniatur
		i++; //kolejna iteracja
	}

	int maxi = ceil(sqrt(i - 1)); //oblicz bok matrycy
	Mat miniatury = createOne(obrazki1, maxi, 1); //stworz obrazek z matrycy 1
	Mat miniatury2 = createOne(obrazki2, maxi, 1); //stworz obrazek z matrycy 2

	imwrite(wejscie + "thumb.png", miniatury); //zapisz matryce 1
	imwrite(wyjscie + "thumbh.png", miniatury2); //zapisz matryce 2

	namedWindow("Miniatury In", WINDOW_AUTOSIZE);// Create a window for display.
	imshow("Miniatury In", miniatury);                   // Show our image inside it.

	namedWindow("Miniatury Out", WINDOW_AUTOSIZE);// Create a window for display.
	imshow("Miniatury Out", miniatury2);                   // Show our image inside it.
}

void __cdecl ThreadProc(void * Args)
{
	cout << "Rozpoczynam kompresje w watku!" << endl;
	exe("D:/Designs/ConsoleApplication/Debug/", "D:/Designs/ConsoleApplication/Debug/");
	cout << "Zakonczylem kompresje w watku!" << endl;
	_endthread();
}

/**
* \brief Main program function.
*/
int _tmain(int argc, _TCHAR* argv[])
{
	/**
	* Creates the ini file (for debug purposes), then reads from it and iniciates the conversion
	* in a separete thread.
	*/
	CIniWriter iniWriter(".\\Michauke.ini");
	iniWriter.WriteString("Setting", "Input", "D:/Designs/ConsoleApplication/Debug/");   //wpisac sciezke obrazkow
	iniWriter.WriteString("Setting", "Output", "D:/Designs/ConsoleApplication/Debug/");

	CIniReader iniReader(".\\Michauke.ini");
	string Input = iniReader.ReadString("Setting", "Input", "");
	string Output = iniReader.ReadString("Setting", "Output", "");
	cout << "Sciezka wejsciowa: " << Input << endl;
	cout << "Sciezka wyjsciowa: " << Output << endl;

	HANDLE hThread = (HANDLE)_beginthread(ThreadProc, 0, NULL); // tworze michasiowy watek
	DWORD result = WaitForSingleObject(hThread, 5000); // czekaj na zakonczenie michasia watku, daje jej 5 sekund

	if (result == WAIT_TIMEOUT)
	{
		cout << "Timeout error." << Output << endl;
	}

	OutputHeading("Memory check");
	_CrtCheckMemory();
	_ASSERTE(_CrtCheckMemory());

	cin.get();
	return 1;
}