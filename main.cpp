#include <opencv2/opencv.hpp>

int main() {
    // Load an image from file
    cv::Mat image = cv::imread("path_to_image.jpg");

    // Check if the image was loaded successfully
    if (image.empty()) {
        std::cerr << "Could not open or find the image" << std::endl;
        return -1;
    }
  
    // Create a window
    cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE);

    // Show the image inside the window
    cv::imshow("Display window", image);

    // Wait for a keystroke in the window
    cv::waitKey(0);

    return 0;
}