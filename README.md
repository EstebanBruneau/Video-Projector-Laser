# Video Projecteur Laser

This project involves the development of a laser video projector system. The system processes images and sends them to microcontrollers for projection.

## Features

- **Image Dimension**: 100x100 pixels (10000 pixels) - **Validated**
- **Frame Rate**: 10 images/second - **Not Validated**
- **Image Size**: 30KB/image (3 bytes/pixel) - **Not Validated**
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