//
// Created by cew05 on 29/03/2024.
//

#ifndef CPP_PROGRAMMINGPROJECT_TRACKBONUSITEMS_H
#define CPP_PROGRAMMINGPROJECT_TRACKBONUSITEMS_H

#include "GameCore.h"
#include "Track.h"

class Track;

class Coin : public TrackObject {
    private:
        // managing score
        int score = 100;

    public:
        Coin(double x, double y, int w, int h);
        bool PlayerPickup(Player &player, int &scoreBoard);
};



class Collectable : public TrackObject {
    private:
        // managing score
        bool collected = false;
        int score = 1000;

    public:
        Collectable(double x, double y, int w, int h);
        bool PlayerPickup(Player &player, int &scoreBoard);
        bool GetCollectedState() const;
};


class ScoreCollectables {
    private:
        // vectors to house Coins, ScoreCollectables
        std::vector<Coin> coins {};
        std::vector<Collectable> collectables {};

        // score vars:
        struct {
            int prevScore = 0;
            int score = 0;
        } scoreBoard;

        // SDL and Display vars
        SDL_Color textCol = {255, 255, 255};
        TTF_Font* textFont {};
        SDL_Surface* surface {};
        SDL_Rect scoreRect {};
        SDL_Rect livesRect {};
        SDL_Texture* scoreLabel {};
        SDL_Texture* scoreValue {};
        SDL_Texture* livesLabel {};

    public:
        // Coins
        void ConstructCoins(Track track);
        void UpdateCoins(Player player, Uint64 ELAPSED_TIME);

        // Collectables
        void ConstructSpecials(Track track);
        void UpdateSpecials(Player player, Uint64 ELAPSED_TIME);
        bool CollectablesObtained();

        // Scoreboard and Lives
        void AddScore(int addScore);
        int GetScore() const;

        // Display
        void CreateTextures(SDL_Renderer* RENDERER);
        void DisplayItems(SDL_Renderer* RENDERER, int xMin, int xMax);
        void DisplayScoreLives(SDL_Renderer* RENDERER, SDL_Window* WINDOW, Player player);
};


#endif //CPP_PROGRAMMINGPROJECT_TRACKBONUSITEMS_H
