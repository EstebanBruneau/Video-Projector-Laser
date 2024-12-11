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

int seuil(int pixel, int plages) {
    int plage = 256 / plages;
    int res = 0;
    for (int i = 0; i < plages; i++) {
        if (pixel >= i * plage / plages && pixel < (i + 1) * plage / plages) {
            res = i * plage / plages;
        }
    }
    return res;
}

std::vector<std::vector<std::vector<int>>> split_image_to_vector(const cv::Mat& image, int plage) {
    std::vector<std::vector<std::vector<int>>> channels(image.rows, std::vector<std::vector<int>>(image.cols, std::vector<int>(image.channels())));
    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {
            for (int k = 0; k < image.channels(); k++) {
                int res = seuil(image.at<cv::Vec3b>(i, j)[k], plage);
                channels[i][j][k] = res;
            }
        }
    }
    return channels;
}

//redimensionne l'image
cv::Mat resize_image(cv::Mat image, int width, int height) {
    cv::Mat resized_image;
    cv::resize(image, resized_image, cv::Size(width, height));
    return resized_image;
}

std::vector<std::vector<std::vector<int>>> process(cv::Mat frame, char** argv) {
    cv::Mat img = frame;
    int height = std::stoi(argv[2]);
    int width = std::stoi(argv[3]);
    cv::Mat imgResized = resize_image(img, height, width);
    return split_image_to_vector(imgResized, std::stoi(argv[4]));
}

int main(int argc, char** argv) {
    cv::VideoCapture video("../Video/Video.mp4");
    if (!video.isOpened()) {
        throw std::runtime_error("Could not open video file");
    }
    cv::Mat frame;
    while (true) {
        video >> frame;
        if (frame.empty()) {
            break;
        }
        std::vector<std::vector<std::vector<int>>> channels = process(frame, argv);
        std::cout << "channels: " << channels.size() << std::endl;
        if (cv::waitKey(30) >= 0) {
            break;
        }
    }
    return 0;
}