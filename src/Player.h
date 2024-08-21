//
// Created by cew05 on 22/02/2024.
//

#ifndef CPP_PROGRAMMINGPROJECT_PLAYER_H
#define CPP_PROGRAMMINGPROJECT_PLAYER_H

#include <utility>

#include "GameCore.h"
#include "Track.h"

class Track;
struct TrackSegment;






class Player
{
    private:
        double rectx, recty;                                                                                            // These values determine where the player is displayed in the window
        double gamex, gamey;                                                                                            // These values are used internally as references to a "true" position
        double offsetx, offsety;                                                                                        // These values are used in the functions determining gamex/y values to provide offset
        double checkx{}, checky{};                                                                                      // These values store the position of the furthest checkpoint that the player reached

        // jump and gameplay vars
        double speedx, speedy;
        int jumpHeight{};
        double gravity{};

        // vars for position calculating
        bool jumpHeld = false;
        unsigned timeSinceOnTrack = 0;
        bool forcedVoid = false;

        // vars for score tracking
        int lives = 3;

        // SDL and Display vars
        SDL_Texture* playerTexture{};
        SDL_Rect playerRect {};
        SDL_Surface* surface{};

        int width;
        int height;

        // player animations vars (determines how the player rect moves for an "animation" to indivate events)
        std::string displayType = "start";

    public:
        Player(int w, int h, int MIN_TRACK_HEIGHT);

        // Player movement
        void MovementUpdate(Track &track, Uint64 tickchange, Uint64 ELAPSED_TIME);
        bool AtEndOfTrack(Track &track);
        bool WithinTrackBounds(Track track);
        bool CheckVoidOut(int GAME_WINDOW_HEIGHT, Uint64 &ELAPSED_TIME);
        void Jump(bool jumpInput);

        // Display
        void CreateTexture(SDL_Renderer* RENDERER);
        void Display(Track &track, Uint64 ELAPSED_TIME, SDL_Renderer* RENDERER);

        // Getters
        int GetLives() const;
        bool OnScreen(int xMin, int xMax) const;
        std::vector<double> GetGamePosition();
        std::vector<double> GetPlayerSpeed();
        std::vector<int> GetPlayerWidthHeight();
        SDL_Texture* GetTexture();

        // Setters
        void ForceVoidOut();
        void ResetPlayerPosition(int MIN_TRACK_HEIGHT);
        void AddPlayerLives(int addLives);
        void SetPlayerSpeed(double horizontalSpeed);
        void SetPlayerJumpCalcVars(int playerJumpHeight, double playerGravity);
};


#endif //CPP_PROGRAMMINGPROJECT_PLAYER_H
