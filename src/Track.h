//
// Created by cew05 on 21/02/2024.
//

#ifndef CPP_PROGRAMMINGPROJECT_TRACK_H
#define CPP_PROGRAMMINGPROJECT_TRACK_H

#include "GameCore.h"
#include "Player.h"

class Player;





struct ObjectStruct {
    // positional information
    double gamex, gamey;
    double rectx, recty;

    // size information
    int width, height;

    void Construct(double x, double y, int w, int h) {
        gamex = x; rectx = x;
        gamey = y; recty = y;

        width = w;
        height = h;
    }
};





struct TrackSegment : public ObjectStruct{
    int trackIndex = 0;

    bool checkpoint = false;
    bool endLevel = false;
    bool startLevel = false;

    void SetIndex() {
        trackIndex = int(gamex / width);
    }
};





class TrackObject {
    protected:
        // SDL and display components
        SDL_Texture* objectTexture {};
        SDL_Rect selfRect {};
        SDL_Surface* surface {};
        std::string imgPath;

        // Struct
        ObjectStruct objectStruct{};

        // bool vars
        bool collisionEnabled = true;
        bool canDisplay = true;

    public:
        TrackObject(double x, double y, int w, int h);
        void CreateTexture(SDL_Renderer* RENDERER);
        void UpdateRect(Uint64 ELAPSED_TIME, double playerSpeed);
        void Display(SDL_Renderer* RENDERER);
        bool PlayerCollision(Player player) const;
        ObjectStruct* GetStruct();
};





class TrackObstacle : public TrackObject {
    private:
        int trackIndex = 0;
    public:
        TrackObstacle(double x, double y, int w, int h);
        bool PlayerCollided(Player &player);
        int GetTrackIndex() const;
        void SetTrackIndex(int index);
};

// Determining possible position
struct PossiblePosition {
    double x;
    double y;
};





struct SectionInfo {
    int minGenBound;
    int maxGenBound;
    int minLen;
    int maxLen;

    void Construct(int minGB, int maxGB, int minL, int maxL) {
        minGenBound = minGB;
        maxGenBound = maxGB;
        minLen = minL;
        maxLen = maxL;
    }

    bool WithinGenBounds(int rGenValue) const {
        return (minGenBound <= rGenValue && rGenValue < maxGenBound);
    }
};





class Track {
    private:
        // SDL and display components
        SDL_Texture* toplevelTexture {};
        SDL_Texture* ttlBgTexture {};
        SDL_Texture* trackFillerTexture {};
        SDL_Texture* ttlCheckpointTexture {};
        SDL_Texture* backgroundTexture {};
        SDL_Rect trackRect {};
        SDL_Rect backgroundRect {0, 0};
        SDL_Surface* surface {};
        int TILE_WIDTH;
        int TILE_HEIGHT;

        // Vectors housing trackSegments and track obstacles
        std::vector<TrackSegment> trackSegments {};
        std::vector<TrackObstacle> trackObstacles {};
        int maxObstacles = 5;

        // Track Section Generation info
        std::vector<SectionInfo> sectionGenerationInfo {};
        int startOfTrackIndex = 0, endOfTrackIndex = 150;
        int nextCheckpointPosition = endOfTrackIndex / 3;
        int MIN_TRACK_HEIGHT, MAX_TRACK_HEIGHT;

        // Player jump calculation vars
        double jumpHeight{};
        double GRAVITY{};
        double playerSpeed{};

    public:
        Track(int MIN_TRACK_HEIGHT, int MAX_TRACK_HEIGHT, int w, int h);

        // ConstructRect the track body
        void ObtainTrackGenInfo(int DIFFICULTY);
        void ConstructTrack(int DIFFICULTY);
        bool ConstructSegment(PossiblePosition &fromPosition, int sectionLength, const std::string& sectionType = "");
        PossiblePosition CreatePossiblePosition(TrackSegment prevSegment, int minLength, int maxLength) const;

        // Updates to rect and player collision detection
        void UpdateTrackRects(Uint64 ELAPSED_TIME);
        bool CheckForCollision(Player& player);

        // Display
        void CreateTextures(SDL_Renderer* RENDERER);
        void DisplayTrack(SDL_Renderer* RENDERER, SDL_Window* WINDOW);
        void DisplayBackground(SDL_Renderer* RENDERER, SDL_Window* WINDOW);

        // Getters
        TrackSegment GetTrackAtIndex(int trackIndex);
        std::vector<int> GetTrackStartEndIndex();
        std::vector<int> GetTrackWidthHeight();
        bool IsObstacleAtIndex(int trackIndex);

        // Setters
        void SetWidthHeight(int w, int h);
        void SetPlayerJumpCalcVars(double trackJumpHeight, double trackGravity, double trackSpeed);
};

#endif //CPP_PROGRAMMINGPROJECT_TRACK_H
