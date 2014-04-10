// gcc -ggdb `pkg-config --cflags opencv` test-webcam.c `pkg-config --libs opencv` -o test-webcam
// ./test-webcam

#include <stdlib.h>
#include <stdio.h>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

// here put the number of the webcam stream to use 
// with Linux, a webcam stream is a file beginning by /dev/video
#define WEBCAM 1

int main()
{
	// on crée un objet pour stocker le flux
	CvCapture* capture;   // on ouvre le flux vidéo/caméra

	// pour ouvrir une vidéo
	//capture = cvCreateFileCapture("/path/to/your/video/test.avi");  

	// pour lire à partir d'une webcam
	capture = cvCreateCameraCapture( WEBCAM );
	// le paramètre est au choix: le numéro du flux, ou CV_CAP_ANY
	// - sous Linux, voir dans /dev pour le N° (un flux vidéo est un fichier /dev/videoN avec N le N° du flux)
	// - CV_CAP_ANY = le 1er flux dispo
	// pour vérifier qu'une webcam marche, il est possible de regarder son flux avec VLC
	// ouvrir VLC -> CTRL + S -> périphérique de capture, sélectionner la webcam et faire "Lire"

	// on vérifie que le flux est bien ouvert
	if (capture == NULL) {
		printf("Ouverture du flux vidéo impossible !\n");
		return 1;
	}

	// on crée un objet IplImage pour avoir un lien vers la prochaine image
	IplImage* image;

	// on déclare un char pour stocker les touches claviers appuyées (pour pouvoir quitter)
	char key;

	// Boucle tant que l'utilisateur n'appuie pas sur la touche q (ou Q)
	while(key != 'q' && key != 'Q') {
 
		// on récupère l'image à partir du flux de la webcam
		image = cvQueryFrame(capture);
	 
		// On affiche l'image dans la fenêtre
		cvShowImage("Webcam to file - test", image);
	 
		// On attend 10ms
		key = cvWaitKey(10);
 	}

 	// on ferme proprement le rpogramme en libérant la mémoire
 	cvDestroyAllWindows();   // on supprime toutes les fenêtres
 	cvReleaseCapture(&capture);  // pour libérer la webcam

	return 0;
}
