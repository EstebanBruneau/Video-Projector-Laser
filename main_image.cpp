#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdexcept>
#include <vector>

// Load an image from file
cv::Mat load_image(const std::string& name) {
    // Check if the name is empty
    if (name.empty()) {
        throw std::logic_error("Image name cannot be empty");
    }

    // Load an image from file
    cv::Mat image = cv::imread("../image/" + name, cv::IMREAD_COLOR);

    // Check if the image was loaded successfully
    if (image.empty()) {
        std::cerr << "Could not open or find the image" << std::endl;
        return cv::Mat();
    }
    return image;
}

// Show an image
void show_image(cv::Mat image) {
    
    // Create a window
    cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE);

    // Show the image inside the window
    cv::imshow("Display window", image);

    // Wait for a keystroke in the window
    cv::waitKey(0);
}

// split image
std::vector<cv::Mat> split_image(cv::Mat image) {
    std::vector<cv::Mat> channels;
    cv::split(image, channels);
    return channels;
}

// split image to vector
// std::vector<std::vector<std::vector<int>>> split_image_to_vector(const cv::Mat& image) {
//     std::vector<std::vector<std::vector<int>>> channels;
//     for (int i = 0; i < image.rows; i++) {
//         std::vector<std::vector<int>> row;
//         for (int j = 0; j < image.cols; j++) {
//             std::vector<int> pixel;
//             for (int k = 0; k < image.channels(); k++) {
//                 pixel.push_back(image.at<cv::Vec3b>(i, j)[k]);
//             }
//             row.push_back(pixel);
//         }
//         channels.push_back(row);
//     }
//     return channels;
// }

// split image to vector with specified size
std::vector<std::vector<std::vector<int>>> split_image_to_vector(cv::Mat image, int num_rows, int num_cols) {
    std::vector<std::vector<std::vector<int>>> channels(num_rows, std::vector<std::vector<int>>(num_cols, std::vector<int>(image.channels())));
    for (int i = 0; i < num_rows; i++) {
        for (int j = 0; j < num_cols; j++) {
            for (int k = 0; k < image.channels(); k++) {
                channels[i][j][k] = image.at<cv::Vec3b>(i, j)[k];
            }
        }
    }
    return channels;
}

// std::vector<std::vector<std::vector<int>>> split_image_to_vector(const cv::Mat& image) {
//     std::vector<std::vector<std::vector<int>>> channels(image.rows, std::vector<std::vector<int>>(image.cols, std::vector<int>(image.channels())));
//     for (int i = 0; i < image.rows; i++) {
//         for (int j = 0; j < image.cols; j++) {
//             for (int k = 0; k < image.channels(); k++) {
//                 channels[i][j][k] = image.at<cv::Vec3b>(i, j)[k];
//                 //std::cout<< "pixel" << channels[i][j][k] << std::endl;
//             }
//         }
//     }
//     return channels;
// }

// print std::vector<std::vector<std::vector<int>>>
void print_vector(std::vector<std::vector<std::vector<int>>> channels) {
    for (int i = 0; i < channels.size(); i++) {
        for (int j = 0; j < channels[i].size(); j++) {
            for (int k = 0; k < channels[i][j].size(); k++) {
                std::cout << channels[i][j][k] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
}

// show split image
void show_split_image(cv::Mat image) {
    std::vector<cv::Mat> channels = split_image(image);
    for (int i = 0; i < channels.size(); i++) {
        show_image(channels[i]);
    }
}

//redimensionne l'image
cv::Mat resize_image(cv::Mat image, int width, int height) {
    cv::Mat resized_image;
    cv::resize(image, resized_image, cv::Size(width, height));
    return resized_image;
}

int main(int argc, char** argv) {
    cv::Mat img = load_image(argv[1]);
    cv::Mat imgResized=resize_image(img, 100, 100);
    show_image(imgResized);
    print_vector(split_image_to_vector(imgResized,100,100));

    /*show_image(img);
    img=resize_image(img, 1000, 1000);
    show_image(img);*/

    //print_vector(split_image_to_vector(img, 10, 10));

    //std::cout << "Vector size: " << split_image_to_vector(img, 10, 10)[0][0].size() << std::endl;

    return 0;
}