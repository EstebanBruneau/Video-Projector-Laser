#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdexcept>
#include <vector>


// split image
std::vector<cv::Mat> split_image(cv::Mat image) {
    std::vector<cv::Mat> channels;
    cv::split(image, channels);
    return channels;
}

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

// Show an image
void show_image(cv::Mat image) {
    
    // Create a window
    cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE);

    // Show the image inside the window
    cv::imshow("Display window", image);

    // Wait for a keystroke in the window
    cv::waitKey(0);
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

// show video
void show_video(cv::VideoCapture video, int width, int height) {
    cv::Mat frame;
    while (true) {
        video >> frame;
        if (frame.empty()) {
            break;
        }
        cv::imshow("Display window", resize_image(frame, width, height));
        if (cv::waitKey(30) >= 0) {
            break;
        }
    }
}

int main(int argc, char** argv) {

    //load video
    cv::VideoCapture video("../Video/Video.mp4");
    if (!video.isOpened()) {
        throw std::runtime_error("Could not open video file");
    }

    //show video
    show_video(video, 100, 100);

    return 0;

}