//
// Created by cew05 on 27/02/2024.
//

#ifndef CPP_PROGRAMMINGPROJECT_GAMECORE_H
#define CPP_PROGRAMMINGPROJECT_GAMECORE_H

// include SDL Libraries
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <random>
#include <cmath>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <ctime>
// Define the global variables

struct Text {
    SDL_Rect textRect {};
    SDL_Texture* textTexture {};
    SDL_Surface* textSurface {};
    std::string text;

    void ConstructRect(int x, int y, int w, double reqh, bool fitToHeight = false) {
        int h = int(reqh);

        if (fitToHeight) {                                                                                              // ensures a ratio between width and height
            SDL_QueryTexture(textTexture, nullptr, nullptr, &w, &h);
            w = int(w / (h / reqh));
            h = int (h / (h / reqh));
        }
        x = x - w/2;
        y = y - h/2;
        textRect = {x, y, w, h};
    }

    void CreateTexture(SDL_Renderer* RENDERER, TTF_Font* textFont) {
        textSurface = TTF_RenderText_Blended(textFont, text.c_str(), {255, 255, 255});
        textTexture = SDL_CreateTextureFromSurface(RENDERER, textSurface);
        SDL_FreeSurface(textSurface);
    }

    void Display(SDL_Renderer* RENDERER) {
        SDL_RenderCopy(RENDERER, textTexture, nullptr, &textRect);
    }
};

struct HighScore {
    std::string seed;

    Text dateText, scoreText;

    void SetValues(std::vector<std::string> scoreData) {
        dateText.text = scoreData[0];
        scoreText.text = scoreData[1];
        seed = scoreData[2];
    }

    std::string GetScoreInfoString() const {
        std::string scoreInfo = dateText.text + ", " + scoreText.text + ", " + seed + ", end\n";
        return scoreInfo;
    }
};

class Menu {
    private:
        // vector to store highScores
        std::vector<HighScore> highScores {};

        // SDL and display components
        SDL_Surface* surface {};
        SDL_Rect bgImageRect {};
        SDL_Texture* bgImageTexture{};
        TTF_Font* textFont{};

        int width;
        int height;

        // text widgets
        std::vector<Text> textWidgets {};

    public:
        Menu(int w, int h);
        void CreateTextures(SDL_Renderer* RENDERER);
        void ObtainScoreInfo();
        void SaveScore( HighScore& newScore);
        void Display(SDL_Renderer* RENDERER);

};

#endif //CPP_PROGRAMMINGPROJECT_GAMECORE_H
