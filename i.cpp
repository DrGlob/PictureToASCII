#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <optional>
#include <filesystem>

using namespace std;

struct ImageData {
    unsigned char* data;
    int width;
    int height;
    int channels;

    std::optional<int> get_brightness(int x, int y) const {
        if (x < 0 || x >= width || y < 0 || y >= height) {
            return std::nullopt;
        }
        
        if (channels < 1) {
            return std::nullopt;
        }
        
        unsigned char* pixel = &data[(y * width + x) * channels];
        
        if (channels >= 3) {
            return static_cast<int>(0.299 * pixel[0] + 0.587 * pixel[1] + 0.114 * pixel[2]);
        } else {
            return static_cast<int>(pixel[0]);
        }
    }
};

ImageData load_image(const char* filename) {
    ImageData img;
    img.data = stbi_load(filename, &img.width, &img.height, &img.channels, 0);
    if (!img.data) {
        std::cerr << "Error: Cannot load image " << filename << std::endl;
        img.width = img.height = img.channels = 0;
    } else {
        std::cout << "Loaded image: " << img.width << "x" << img.height 
                  << " channels: " << img.channels << std::endl;
    }
    return img;
}

bool create_directory_if_not_exists(const string& path) {
    try {
        if (!filesystem::exists(path)) {
            return filesystem::create_directories(path);
        }
        return true;
    } catch (const exception& e) {
        cerr << "Error creating directory: " << e.what() << endl;
        return false;
    }
}

void convert_picture(const ImageData& image) {
    const string ascii_chars = " `.,-':<>;+!*/?%&98#";
    const double coef = 255.0 / (ascii_chars.length() - 1);
    

    const int skip_width = 4;   // Пропуск по горизонтали
    const int skip_height = 8;  // Пропуск по вертикали
    
    // Создаем папку output если ее нет
    if (!create_directory_if_not_exists("output")) {
        cerr << "Warning: Using current directory for output" << endl;
    }
    
    string output_path = "output/ASCII_Art.txt";
    ofstream file(output_path);
    if (!file.is_open()) {
        output_path = "ASCII_Art.txt";
        file.open(output_path);
        if (!file.is_open()) {
            cerr << "Error: Cannot open output file" << endl;
            return;
        }
    }
    
    cout << "Processing image " << image.width << "x" << image.height << "..." << endl;
    

    for (int y = 0; y < image.height - 1; y += skip_height) {
        string s = "";
        for (int x = 0; x < image.width - 1; x += skip_width) {
            try {
                auto brightness_opt = image.get_brightness(x, y);
                if (brightness_opt) {
                    int brightness = brightness_opt.value();
                    int index = ascii_chars.length() - static_cast<int>(brightness / coef) - 1;
                    
                    index = max(0, min(static_cast<int>(ascii_chars.length() - 1), index));
                    
                    s += ascii_chars[index];
                } else {
                    s += " ";
                }
            } catch (const exception& e) {
                s += " ";
            }
        }
        if (!s.empty()) {
            file << s << "\n";
        }
    }
    
    file.close();
    cout << "ASCII art successfully saved to: " << output_path << endl;
}



void free_image(ImageData& image) {
    if (image.data) {
        stbi_image_free(image.data);
        image.data = nullptr;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <image_file>" << endl;
        cerr << "Example: " << argv[0] << " image.jpg" << endl;
        return 1;
    }

    const char* filename = argv[1];
    ImageData image = load_image(filename);
    
    if (!image.data) {
        return 1;
    }

    convert_picture(image);
    free_image(image);

    return 0;
}