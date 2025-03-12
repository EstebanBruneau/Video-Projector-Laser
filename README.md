# Video Projecteur Laser

This project involves the development of a laser video projector system. The system processes images and sends them to microcontrollers for projection.

# Ce qu'il reste à faire
raspi:
Régler l'intensité lumineuse des LED
Clock



Questions:
Parcours et envoie des données (ordre d'envoi)?
Images fixe(parcours haut->bas/bas->haut)? haut->bas et gauche->droite

Code:
split video to image
image to vector -> Attention on est pas en RGB mais en BGR
print vector

Seuil (+ de 128 => 1 sinon 0)

Manipulation de la matrice image (en fonction des besoins du groupe)

Test de perf sur l'esp32 on a entre entre 2  et 8 ms de latences !!!!!

A chaque front montant on envoie 100 pixels 10 ms de décalage pour ce décalage on doit utiliser esp timer

1- convertir les int de couleurs en bin on les envi sur 8 pin on a en plus 3 pin couleurs
(Louis veut mesurer la rapidité de l'envoie des trois)


## Machine à état

On veut faire une machine à état, on a un état et dans main while(true){switch case} pour les état on les définis avec define

Etat 1 : en attente image : c'est un interrupt qui écoute un pin qui est activé à un tour complet du moteur lent

Etat 2 : swap buffer : déclenché après l'état 1, il change les images (1 image est chargé et l'autre est affichée)

Etat 3 : en attente ligne : déclenché après l'état 2 c'est un interrupt en attente d'un changement de signale sur un pin c'est le changement d'état au changement de miroir du moteur rapide 

Etat 4 : affche ligne est déclenché quand l'interrupt attente ligne est déclenché, l'état déclenche 100 fois affiche pixel

Affiche pixel fonction callback appel toutes les 10 ms 100 fois

Affiche pixel envoi 11 bit dans 11 sorties : 8 pour la couleur et 3 pour RGB ensuite il attent 10 ms
et retourne à affiche ligne

A faire: set up interrupt / fonctions apppelé / main witch switch 

## Objectif

Faire clignoter 3 led colorés représentants les couleurs RGB :
- Dans l'IDE espressif récupérer une image
- Découper l'image en matrice (à l'aide de la bibliothèque opencv)
- renvoyer les valeurs RGB sur 3 pins différents 

## Features

- **Image Dimension**: 100x100 pixels (10000 pixels) - **Validated**
- **Frame Rate**: 10 images/second - **Not Validated**
- **Image Size**: 30KB/image (3 bytes/pixel) - **Validated**
- **Data Transmission**: Line-by-line transmission to microcontrollers - **Not Validated**
- **Buffer Function**: Manages timing of data transmission - **Not Validated**

## Requirements

- OpenCV library
- CMake 3.10 or higher
- C++11 or higher

## Installation

1. Clone the repository:
    ```sh
    git clone <repository-url>
    cd <repository-directory>
    ```

2. Set up OpenCV:
    - Ensure OpenCV is installed on your system.
    - Update the `OpenCV_DIR` path in [CMakeLists.txt](CMakeLists.txt) to point to your OpenCV build directory.

3. Build the project:
    ```sh
    mkdir build
    cd build
    cmake ..
    make
    ```

## Usage

1. Load and display an image:
    ```cpp
    cv::Mat img = load_image("CaptureCouleur.PNG");
    show_image(img);
    ```

2. Resize the image:
    ```cpp
    img = resize_image(img, 1000, 1000);
    show_image(img);
    ```

3. Split the image into channels and display:
    ```cpp
    show_split_image(img);
    ```

4. Convert the image to a vector and print:
    ```cpp
    std::vector<std::vector<std::vector<int>>> vec = split_image_to_vector(img, 10, 10);
    print_vector(vec);
    ```

## Code Overview

- **Image Loading**: [`load_image`](main.cpp) function loads an image from the `image` directory.
- **Image Display**: [`show_image`](main.cpp) function displays an image in a window.
- **Image Resizing**: [`resize_image`](main.cpp) function resizes an image to specified dimensions.
- **Image Splitting**: [`split_image`](main.cpp) function splits an image into its color channels.
- **Vector Conversion**: [`split_image_to_vector`](main.cpp) function converts an image to a 3D vector.
- **Vector Printing**: [`print_vector`](main.cpp) function prints a 3D vector.

## Contributing

Contributions are welcome! Please fork the repository and submit a pull request.

## License

This project is licensed under the MIT License.

## Contact

For any inquiries, please contact Emma Dégot and Esteban Bruneau at 

emma.degot@gmail.com 

esteban.bruneau@gmail.com.