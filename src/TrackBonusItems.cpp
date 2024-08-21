//
// Created by cew05 on 29/03/2024.
//

#include "TrackBonusItems.h"




Coin::Coin(double x, double y, int w, int h) : TrackObject(x, y, w, h) {
    // set coin texture path
    imgPath = "../Resources/Images/Coin/100Coin_ratio_1-1.png";
}

bool Coin::PlayerPickup(Player &player, int &scoreBoard) {
    // This function defines specialised behaviour for coins when collision with the player is detected
    if (!PlayerCollision(player)) return false;

    collisionEnabled = false;                                                                                           // prevent player recollecting coins
    canDisplay = false;
    scoreBoard += score;

    return true;
}



Collectable::Collectable(double x, double y, int w, int h) : TrackObject(x, y, w, h) {
    // set collectable texture path:
    imgPath = "../Resources/Images/Coin/SpecialCoin_Star.png";
}


bool Collectable::PlayerPickup(Player &player, int &scoreBoard) {
    // this function defines specialised behaviour for collectables when collision with the player is detected
    if (!PlayerCollision(player)) return false;

    collisionEnabled = false;
    canDisplay = false;
    collected = true;
    scoreBoard += score;

    return true;
}

bool Collectable::GetCollectedState() const {
    return collected;
}




void ScoreCollectables::ConstructCoins(Track track) {
    /*
     * This function will attempt to place a coin on every trackSegment. There is a 25% chance of this succeeding,
     * at which point a coin object is created and positioned appx 3 TILE HEIGHTS above the track.
     */
    coins = {};                                                                                                         // Empty coins vector to remove prev coins
    int hCoin = track.GetTrackWidthHeight()[1];                                                                         // Determine w h of coins
    int wCoin = hCoin * 4/5;

    // Determine distance from track top level that coin sits at
    int yDist = (track.GetTrackWidthHeight()[1] * 5/2) + (hCoin/2);

    // through each track index in the main body of the track
    for (int ti = track.GetTrackStartEndIndex()[0]; ti < track.GetTrackStartEndIndex()[1]; ti++) {
        TrackSegment trackSegment = track.GetTrackAtIndex(ti);

        // Add coin at 25% chance
        if (25 > std::rand() % 100 + 0) {
            Coin newCoin(trackSegment.gamex, trackSegment.gamey - yDist, wCoin, hCoin);
            coins.push_back(newCoin);
        }
    }
}

void ScoreCollectables::UpdateCoins(Player player, Uint64 ELAPSED_TIME) {
    // this function moves the rect position of all coins stored in the vector
    for (Coin &coin: coins) {
        coin.UpdateRect(ELAPSED_TIME, player.GetPlayerSpeed()[0]);                                           // update all coins rect positions
        coin.PlayerPickup(player, scoreBoard.score);                                                              // check for player within coin bounds
    }
}




void ScoreCollectables::ConstructSpecials(Track track) {
    /*
     * This function will place a single collectable item within each third of the track. the collectable is given a 5%
     * chance to spawn on any given track, which repeatedly attempts generation until one succeeds. Upon success, the
     * generation region moves to the next third of the track.
     */
    collectables = {};                                                                                                  // empty any previous collectables
    int hColl = track.GetTrackWidthHeight()[1];                                                                         // Determine w h of collectables
    int wColl = track.GetTrackWidthHeight()[0];

    // creates a collectible in each third of the track
    int regionSize = (track.GetTrackStartEndIndex()[1] - track.GetTrackStartEndIndex()[0]) / 3;
    int startIndex = track.GetTrackStartEndIndex()[0], endIndex = startIndex + regionSize;
    int placed = 0;
    bool collectablePlaced = false;

    while (placed < 3) {
        for (int ti = startIndex; ti < endIndex; ti++) {
            TrackSegment trackSegment = track.GetTrackAtIndex(ti);
            if (trackSegment.trackIndex == track.GetTrackStartEndIndex()[0]) continue;

            // Add collectable at 5% chance
            if (5 > std::rand() % 100 + 0) {
                // Determine distance from track top level that collectable sits at
                int yDist = (track.GetTrackWidthHeight()[1]* 1/2) + (hColl / 2);

                if (track.IsObstacleAtIndex(trackSegment.trackIndex)) {
                    yDist += track.GetTrackWidthHeight()[1];
                }
                Collectable newCollectable(trackSegment.gamex, trackSegment.gamey - yDist, wColl, hColl);
                collectables.push_back(newCollectable);


                placed++;
                collectablePlaced = true;
                break;
            }
        }

        if (collectablePlaced) {
            // next region:
            startIndex += regionSize;
            endIndex += regionSize;
            collectablePlaced = false;
        }

    }
}

void ScoreCollectables::UpdateSpecials(Player player, Uint64 ELAPSED_TIME) {
    // this function moves the rect position of all collectables stored in the vector
    for (Collectable &collectable : collectables) {
        collectable.UpdateRect(ELAPSED_TIME, player.GetPlayerSpeed()[0]);

        collectable.PlayerPickup(player, scoreBoard.score);
    }
}


bool ScoreCollectables::CollectablesObtained() {
    // return true if all collectables have been obtained
    bool collected = true;
    for (const Collectable& collectable : collectables) {
        if (!collectable.GetCollectedState()) collected = false;
    }

    return collected;
}



void ScoreCollectables::AddScore(int addScore) {
    scoreBoard.score  += addScore;
}

int ScoreCollectables::GetScore() const {
    return scoreBoard.score;
}


void ScoreCollectables::CreateTextures(SDL_Renderer* RENDERER) {
    /*
     * Creates the textures for all coin and collectable objects. Additionally, creates the textures for the lives and
     * score tracking labels.
     */
    for (Coin &coin : coins) {
        coin.CreateTexture(RENDERER);
    }

    for (Collectable &collectable : collectables) {
        collectable.CreateTexture(RENDERER);
    }

    std::string fontPath = "../Resources/CookieCrisp/CookieCrisp-L36ly.ttf";
    textFont = TTF_OpenFont(fontPath.c_str(), 200);

    surface = TTF_RenderText_Blended(textFont, "Lives: ", textCol);
    livesLabel = SDL_CreateTextureFromSurface(RENDERER, surface);
    SDL_FreeSurface(surface);

    surface = TTF_RenderText_Blended(textFont, "Score: ", textCol);
    scoreLabel = SDL_CreateTextureFromSurface(RENDERER, surface);
    SDL_FreeSurface(surface);
}

void ScoreCollectables::DisplayItems(SDL_Renderer *RENDERER, int xMin, int xMax) {
    // Attempt to display all coin and collectable items where the rect position is within the window boundaries
    for (Coin coin: coins) {
        ObjectStruct *coinStruct = coin.GetStruct();
        // only display if within the game screen
        if (coinStruct->rectx <= xMax + coinStruct->width/2.0 && coinStruct->rectx >= xMin - coinStruct->width/2.0) {
            coin.Display(RENDERER);
        }
    }

    for (Collectable collectable : collectables) {
        ObjectStruct *collStruct = collectable.GetStruct();
        // only display if within the game screen
        if (collStruct->rectx <= xMax + collStruct->width/2.0 && collStruct->rectx >= xMin - collStruct->width/2.0) {
            collectable.Display(RENDERER);
        }
    }
}

void ScoreCollectables::DisplayScoreLives(SDL_Renderer* RENDERER, SDL_Window* WINDOW, Player player) {
    /*
     * This function displays the lives and score information in the top left of the window. The score texture is
     * updated only when the score is updated. The lives are shown through taking the player texture as a lives icon,
     * the number of this icon indicating the amount of lives left.
     */

    int GAME_WINDOW_HEIGHT, GAME_WINDOW_WIDTH;
    SDL_GetWindowSize(WINDOW, &GAME_WINDOW_WIDTH, &GAME_WINDOW_HEIGHT);                                           // fetch window size

    livesRect = {0, 0, GAME_WINDOW_WIDTH/12, GAME_WINDOW_HEIGHT/20};
    SDL_RenderCopy(RENDERER, livesLabel, nullptr, &livesRect);

    // DisplayTrack lives icons
    for (int l = 0; l < player.GetLives(); l++) {
        livesRect.x += livesRect.w + livesRect.w/20;
        livesRect.w = livesRect.h;

        SDL_RenderCopy(RENDERER, player.GetTexture(), nullptr, &livesRect);
    }

    // DisplayTrack score label
    scoreRect = {0, GAME_WINDOW_HEIGHT / 20, GAME_WINDOW_WIDTH/12, GAME_WINDOW_HEIGHT/20};
    SDL_RenderCopy(RENDERER, scoreLabel, nullptr, &scoreRect);

    // Get score texture
    if (scoreBoard.score != scoreBoard.prevScore) {
        surface = TTF_RenderText_Blended(textFont, std::to_string(scoreBoard.score).c_str(), textCol);
        scoreValue = SDL_CreateTextureFromSurface(RENDERER, surface);
        SDL_FreeSurface(surface);

        scoreBoard.prevScore = scoreBoard.score;
    }

    // align with score label:
    int w, h;
    double reqh = GAME_WINDOW_HEIGHT/20.0;
    SDL_QueryTexture(scoreValue, nullptr, nullptr, &w, &h);
    double ratio = h / reqh;                                                                                            // get scale factor to retain w:h ratio
    h = int(h / ratio);                                                                                                 // apply scale factor to width height
    w = int(w / ratio);

    //Display Track score
    scoreRect = {GAME_WINDOW_WIDTH/11, GAME_WINDOW_HEIGHT /20, w, h};
    SDL_RenderCopy(RENDERER, scoreValue, nullptr, &scoreRect);
}