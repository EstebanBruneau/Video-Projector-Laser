#include <opencv2/opencv.hpp>

//J'ajoute un commentaire pour tester le push
// Load an image from file
cv::Mat load_image(std::string name) {
    // Load an image from file
    cv::Mat image = cv::imread(name, cv::IMREAD_COLOR);

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

// // split image to vector
// std::vector<std::vector<std::vector<int>>> split_image_to_vector(cv::Mat image) {
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

int main() {
    cv::Mat img = load_image("green.png");

    print_vector(split_image_to_vector(img, 10, 10));

    std::cout << "Vector size: " << split_image_to_vector(img, 10, 10)[0][0].size() << std::endl;

    return 0;
}