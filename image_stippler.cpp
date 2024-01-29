#include <iostream>
#include <algorithm>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <random>

#define STB_IMAGE_IMPLEMENTATION
#include "std_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "std_image_write.h"

std::pair< std::vector<int>, std::vector< std::vector<int> > > getCDFs(unsigned char* image, int w, int h) {
    std::vector<int> rows(h);
    std::vector< std::vector<int> > collumns( h , std::vector<int> (w));  
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int Y = image[y*w + x];
            rows[y] += 255 - Y;
            collumns[y][x] += 255 - Y;
            if (x != w-1) collumns[y][x+1] = collumns[y][x];
        }
        if (y != h-1) rows[y+1] = rows[y];
    }
    std::pair< std::vector<int>, std::vector< std::vector<int> > > result;
    result.first = rows;
    result.second = collumns;
    return result;
}

void stipple_with_random_sampling(unsigned char *image, int w, int h, const char* out_filename = "new_image.png") {
    unsigned char new_image[w*h];
    for (int i = 0; i < w*h; i++)
        new_image[i] = 255;

    std::pair< std::vector<int>, std::vector< std::vector<int> > > cdfs = getCDFs(image, w, h);
    std::vector<int> rows(h);
    std::vector< std::vector<int> > collumns (h, std::vector<int> (w));
    rows = cdfs.first;
    collumns = cdfs.second;
    
    int num_samples = w*h;
    std::mt19937 rng(static_cast<unsigned>(std::time(0)));
    for (int i = 0; i < num_samples; i++) {
        std::uniform_real_distribution<> dist(0, 1);
        double sample_row = dist(rng)*(double) rows[h-1];
        std::vector<int>::iterator row_it = std::lower_bound(rows.begin(), rows.end(), sample_row);
        int row_index = std::distance(rows.begin(), row_it);

        std::uniform_real_distribution<> dist2(0, 1);
        double sample_collumn =dist2(rng)*(double) collumns[row_index][w-1];
        std::vector<int>::iterator collumn_it = std::lower_bound(collumns[row_index].begin(), collumns[row_index].end(), sample_collumn);
        int collumn_index = std::distance(collumns[row_index].begin(), collumn_it);
        new_image[row_index*w + collumn_index] = 0;
    }

    stbi_write_png(out_filename, w, h, 1, new_image, w);
}

void stipple_with_stratified_sampling(unsigned char *image, int w, int h, int n, const char* out_filename = "new_image.png") {
    unsigned char new_image[w*h];
    for (int i = 0; i < w*h; i++)
        new_image[i] = 255;

    std::pair< std::vector<int>, std::vector< std::vector<int> > > cdfs = getCDFs(image, w, h);
    std::vector<int> rows(h);
    std::vector< std::vector<int> > collumns (h, std::vector<int> (w));
    rows = cdfs.first;
    collumns = cdfs.second;
    
    int num_samples = w*h;
    std::mt19937 rng(static_cast<unsigned>(std::time(0)));
    for (int i = 0; i < num_samples/n; i++) {
        for (int j = 0; j < sqrt(n); j++) {
            for (int k = 0; k < sqrt(n); k++) {
                std::uniform_real_distribution<> dist(j, j+1);
                double sample_row = dist(rng)*(double) rows[h-1]/(double) sqrt(n);
                std::vector<int>::iterator row_it = std::lower_bound(rows.begin(), rows.end(), sample_row);
                int row_index = std::distance(rows.begin(), row_it);

                std::uniform_real_distribution<> dist2(k, k+1);
                double sample_collumn =dist2(rng)*(double) collumns[row_index][w-1]/(double) sqrt(n);
                std::vector<int>::iterator collumn_it = std::lower_bound(collumns[row_index].begin(), collumns[row_index].end(), sample_collumn);
                int collumn_index = std::distance(collumns[row_index].begin(), collumn_it);
                new_image[row_index*w + collumn_index] = 0;  
            }
        }      
    }

    stbi_write_png(out_filename, w, h, 1, new_image, w);
}

int main() {
    int w_1, h_1;
    unsigned char *image_1 = stbi_load("image_1.png", &w_1, &h_1, NULL, 1);   
    if (image_1 == nullptr) {
        std::cout << "Invalid image!" << std::endl;
        return 0;
    } 
    stipple_with_random_sampling(image_1, w_1, h_1, "image_1_random_stipple.png");
    stipple_with_stratified_sampling(image_1, w_1, h_1, 100, "image_1_stratified_stipple.png");
    stbi_image_free(image_1);
    
    int w_2, h_2;
    unsigned char *image_2 = stbi_load("image_2.png", &w_2, &h_2, NULL, 1);    
    if (image_2 == nullptr) {
        std::cout << "Invalid image!" << std::endl;
        return 0;
    } 
    stipple_with_random_sampling(image_2, w_2, h_2, "image_2_random_stipple.png");
    stipple_with_stratified_sampling(image_2, w_2, h_2, 256, "image_2_stratified_stipple.png");
    stbi_image_free(image_2);

    return 0;
}