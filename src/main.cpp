#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

struct Options {
    std::string default_charset = "q.,:;i1tfLCG08MW#%B@$&*+=-^~;:/|";
    bool default_easefunc = true;
    bool default_invert = false;
    int scale = 1;
    float contrast = 1.0f;
};

struct Image {
    int width;
    int height;
    std::vector<float> brightness;
    std::string filepath;
};

Image loadImage(const std::string& filepath) {
    int channel = 0;
    int width = 0, height = 0;

    unsigned char* data = stbi_load(
        filepath.c_str(),
        &width,
        &height,
        &channel, 
        3
    );

    Image result;
    result.width = width;
    result.height = height;
    result.brightness.reserve(width * height);
    result.filepath = filepath;

    for (int i = 0; i < width * height; i++) {
        int idx = i * 3;
        int R = data[idx], G = data[idx + 1], B = data[idx + 2];
        result.brightness.push_back(1.0f - (0.299f * R + 0.587f * G + 0.114 * B) / 255);
        // https://stackoverflow.com/questions/596216/formula-to-determine-perceived-brightness-of-rgb-color
    }

    return result;
}

float smoothstep(float x) {
    return x * x * x * (x * (6.0f * x - 15.0f) + 10.0f);
}

void printImage(const Image& img, std::string charset, bool easefunc, bool invert) {
    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            float value = img.brightness[y * img.width + x];
            if (easefunc) value = smoothstep(value);
            if (invert) value = 1.0f - value;

            int guess = std::roundf((charset.size() - 1) * value);
            std::cout << charset[guess];
        }
        std::cout << '\n';
    }
}

void writeImage(const Image& img, std::string charset, bool easefunc, bool invert) {
    std::string filepath = img.filepath + ".txt";
    std::ofstream output(filepath);

    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            float value = img.brightness[y * img.width + x];
            if (easefunc) value = smoothstep(value);
            if (invert) value = 1.0f - value;

            int guess = std::roundf((charset.size() - 1) * value);
            output << charset[guess] << ' ';
        }
        output << '\n';
    }
    output.close();
}

void executeCommand(const std::string& command, Options& options) {
    std::stringstream parser(command);
    std::string type, filepath, charset = options.default_charset;
    bool easefunc = options.default_easefunc, invert = options.default_invert;

    parser >> type >> filepath >> charset;
    parser >> easefunc >> invert;

    if (charset == "0") {
        charset = options.default_charset;
    }
    for (int i = 0; i < charset.size(); i++) {
        if (charset[i] == 'q') charset[i] = ' ';
    }

    std::string filepathFix;
    filepathFix.reserve(filepath.size());
    for (const char& c : filepath) {
        if (c == '\'') continue;
        filepathFix.push_back(c);
    }

    if (type == "print") {
        std::cout << "] Attempting to print\n";
        Image img = loadImage(filepathFix);
        printImage(img, charset, easefunc, invert);
        std::cout << "] Print complete\n";
    } 
    else if (type == "write") {
        std::cout << "] Attempting to write\n";
        Image img = loadImage(filepathFix);
        writeImage(img, charset, easefunc, invert);
        std::cout << "] Write complete\n";
    }
    else if (type == "set_default_charset") {
        options.default_charset = filepath;
    }
    else if (type == "set_default_easefunc") {
        options.default_easefunc = filepath[0] - '0';
    }
    else if (type == "set_default_invert") {
        options.default_invert = filepath[0] - '0';
    }
    else if (type == "reset_options") {
        options.default_charset = "q.,:;i1tfLCG08MW#%B@$&*+=-^~;:/|";
        options.default_easefunc = true;
        options.default_invert = false;
        options.scale = 1;
        options.contrast = 1.0f;
    }
    else {
        std::cout << "] Incorrect command: '" << type << "'\n";
    }
}

void loadOptions(Options& options) {
    std::ifstream input("options.txt");
    std::string type, value;

    while (input >> type >> value) {
        if (type == "default_charset:") {
            options.default_charset = value;
        } 
        else if (type == "default_easefunc:") {
            options.default_easefunc = value[0] - '0';
        } 
        else if (type == "default_invert:") {
            options.default_invert = value[0] - '0';
        } 
        else if (type == "scale:") {
            options.scale = std::stoi(value);
        } 
        else if (type == "contrast:") {
            options.contrast = std::stof(value);
        }
    }
    input.close();
}

void unloadOptions(Options& options) {
    std::ofstream output("options.txt");

    output << "4: q.#@\n";
    output << "8: q.,:;=+*#@\n";
    output << "12: q.,:-=+*#%8@\n";
    output << "16: q.,:;i1tfLCG08@\n";
    output << "20: q.,:;i1tfLCG08MW#%B@\n";
    output << "24: q.,:;i1tfLCG08MW#%B@$&*+\n";
    output << "28: q.,:;i1tfLCG08MW#%B@$&*+=-^~\n";
    output << "32: q.,:;i1tfLCG08MW#%B@$&*+=-^~;:/|\n";
    output << "default_charset: " << options.default_charset << '\n';
    output << "default_easefunc: " << options.default_easefunc << '\n';
    output << "default_invert: " << options.default_invert << '\n';
    output << "scale: " << options.scale << '\n';
    output << "contrast: " << options.contrast << '\n'; 

    output.close();
}

int main() {
    system("clear");
    
    Options options;
    loadOptions(options);

    std::string input;
    while (true) {
        std::cout << "--CHARCOAL ASCII IMAGE CONVERTER--\n";
        std::cout << "h | help\n";
        std::cout << "o | options\n";
        std::cout << "c | clear\n";
        std::cout << "q | quit\n";
        std::cout << "=> ";
        std::getline(std::cin, input);

        if (input == "h") {
            std::cout << ") print {filepath} {charset} {easefunc} {invert} | image to console\n";
            std::cout << ") write {filepath} {charset} {easefunc} {invert} | image to file (filepath.txt)\n";
            std::cout << ") print {filepath}                               | image to console (default)\n";
            std::cout << ") write {filepath}                               | image to file (default)\n";
            std::cout << ") set_default_charset {charset}                  | change default_charset option\n";
            std::cout << ") set_default_easefunc {easefunc}                | change default_easefunc option\n";
            std::cout << ") set_default_invert {invert}                    | change default_invert option\n";
            std::cout << ") reset_options                                  | change options to the basic template\n";    
            std::cout << ") {filepath} | File path with the files extension (name.png, name.jpg)\n";
            std::cout << ") {charset}  | Character set, 0 for default, q for blank space\n";
            std::cout << ") {easefunc} | 0 for linear, 1 for smoothstep\n";
            std::cout << ") {invert}   | 0 for false, 1 for true\n";
        }
        else if (input == "o") {
            std::cout << "default_charset: " << options.default_charset << '\n';
            std::cout << "default_easefunc: " << options.default_easefunc << '\n';
            std::cout << "default_invert: " << options.default_invert << '\n';
            std::cout << "scale: " << options.scale << '\n';
            std::cout << "contrast: " << options.contrast << '\n';
        }
        else if (input == "c") {
            system("clear");
        }
        else if (input == "q") {
            unloadOptions(options);
            return 0;
        }
        else {
            executeCommand(input, options);
        }
    }
}