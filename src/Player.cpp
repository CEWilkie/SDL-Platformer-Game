//
// Created by cew05 on 22/02/2024.
//

#include "Player.h"

Player::Player(int w, int h, int MIN_TRACK_HEIGHT){
    // Set width, height
    width = w;
    height = h;

    // set position for game, offset values
    gamex = offsetx = width * 6;
    gamey = offsety = MIN_TRACK_HEIGHT - height;

    // Set initial checkpoint
    checkx = gamex;
    checky = gamey;

    // Set initial rect position
    rectx = 0;
    recty = MIN_TRACK_HEIGHT - height;

    // Set speed values
    speedx = width / 240.0, speedy = 0;

    // Initialise rect
    playerRect = {0, 0, width, height};
}






void Player::MovementUpdate(Track &track, Uint64 tickchange, Uint64 ELAPSED_TIME){
    /*
     * This function updates the players game x and y position values and prevents the player from falling through
     * the track.
     */

    timeSinceOnTrack += tickchange;                                                                                     // used for calculating y position

    // Check if player on track (and not being forced into void)
    if (!forcedVoid && WithinTrackBounds(track)) {                                                                      // player is on track so prevent falling
            timeSinceOnTrack = 0;
            speedy = 0;
            offsety = gamey;
    }

    // Increment game position
    gamex = speedx * double(ELAPSED_TIME) + offsetx;
    gamey = (0.5 * gravity * std::pow(timeSinceOnTrack, 2)) - (speedy * timeSinceOnTrack) + offsety;               // produced quadratic curve (from t^2) to model player height
}






bool Player::WithinTrackBounds(Track track) {
    /*
     * This function determines if the player is within the x boundaries of a track segment, that is either behind it,
     * under it or infront of it. If the bottom of the player is within 5 distance from the top of the track, and is
     * falling, then the player will be deemed to be within the bounds.
     */
    int trackIndex = int(gamex/track.GetTrackWidthHeight()[0]);
    std::vector<TrackSegment> checkTracks = {track.GetTrackAtIndex(trackIndex - 1),                            // get previous, current and next track to check for player collision
                                             track.GetTrackAtIndex(trackIndex),
                                             track.GetTrackAtIndex(trackIndex + 1)};

    return std::any_of(checkTracks.begin(), checkTracks.end(), [&](TrackSegment track){
            if (std::abs(gamex - track.gamex) > width) return false;                                                 // player gamex is not within track bounds

            if (track.checkpoint && track.gamex > checkx) {                                                             // update checkpoint position as player is within bounds
                checkx = track.gamex;
                checky = track.gamey - height;

                printf("CHECKPOINT | x: %f, y: %f\n", checkx, checky);
            }

            // Get vertical distance and ensure player is falling onto track not jumping up into it
            int verticalDist = int((track.gamey - track.height/2.0) - (gamey + height / 2.0));
            double dy_dt = gravity * timeSinceOnTrack - speedy;                                                         //dy_dt is the rate of change of player's height. if + then falling

            if (std::abs(verticalDist) <= 5 && dy_dt >= 0) {                                                         // player is within vertical bounds and falling
                gamey = track.gamey - height;
                return true;
            }

            return false;                                                                                               // player is outside vertical bounds
    });
}



bool Player::CheckVoidOut(int GAME_WINDOW_HEIGHT, Uint64 &ELAPSED_TIME) {
    /*
     * This function determines if the player has fallen below the screen view. If this is true, it returns the player
     * to the previously reached checkpoint, and updates the ELAPSED TIME of the level to return all track objects to
     * their previous positions.
     */
    if (gamey < GAME_WINDOW_HEIGHT) return false;                                                                       // player is not off screen so return false

    forcedVoid = false;

    // return to checkpoint track:
    double dx = checkx - gamex;
    ELAPSED_TIME += (std::abs(int(dx/speedx)) < ELAPSED_TIME) ? int(dx / speedx) : - ELAPSED_TIME;
    gamey = checky;
    offsety = gamey;
    gamex = checkx;

    // Reset player y modelling values
    speedy = 0;
    timeSinceOnTrack = 0;

    // minus life
    lives -= 1;

    printf("VO | GOTO x: %f, y: %f\n", gamey, gamex);

    return true;
}

void Player::Jump(bool jumpInput) {
    /*
     * This function manages the jump inputs and calculates the player's vertical speed value. the vertical speed is
     * dependant upon how long the jump input has been held down for. a minimum jump height is also applied.
     */
    if (jumpInput && !jumpHeld && timeSinceOnTrack == 0) {
        jumpHeld = true;
        timeSinceOnTrack = 1;
    }

    // allow player to have varied jump heights (40% to 100% power)
    int maxJumpCharge = 500;
    if (jumpInput && jumpHeld && timeSinceOnTrack <= maxJumpCharge) {
        speedy = std::sqrt(2 * jumpHeight * gravity) * (0.4 + (timeSinceOnTrack / 500.0) * 0.6);
    }

    // Ensure minimum jump of 40% power
    if (!jumpInput && jumpHeld && timeSinceOnTrack < 100) {
        speedy = std::sqrt(2 * jumpHeight * gravity) * 0.4;
    }

    // Update previous player input:
    if (!jumpInput) {
        jumpHeld = false;
    }
}


bool Player::AtEndOfTrack(Track &track) {
    // This function determines if the payer is currently above a trackSegment with the "endLevel" var set to true
    TrackSegment currentTrack = track.GetTrackAtIndex(int(gamex / width));                                     // get current track player is on

    if (currentTrack.endLevel) {                                                                                        // prevent jumping and falling
        speedy = 0;
        gamey = currentTrack.gamey - height;
        return true;
    }

    return false;
}





void Player::CreateTexture(SDL_Renderer* RENDERER) {
    // Create player model Texture
    surface = IMG_Load("../Resources/Images/Player/PlayerModel.png");
    playerTexture = SDL_CreateTextureFromSurface(RENDERER, surface);
    SDL_FreeSurface(surface);
}

void Player::Display(Track& track, Uint64 ELAPSED_TIME, SDL_Renderer* RENDERER){
    /*
     * This function handles the 3 display states of the player, to provide animations letting it move onto and off the
     * screen horizontally at the start and end of the level.
     */

    // set default rect values
    recty = gamey;
    rectx= offsetx;

    // Determine display type for providing different rect values
    TrackSegment currentTrack = track.GetTrackAtIndex(int(gamex / width));
    if (currentTrack.startLevel) {
        rectx = (gamex - offsetx < offsetx) ? double(ELAPSED_TIME) * speedx : offsetx;
    }
    if (currentTrack.endLevel) {
        rectx = (double(ELAPSED_TIME) * (speedx)) - gamex + (2*offsetx);
    }

    playerRect.x = int(int(rectx) - (width / 2.0));
    playerRect.y = int(recty - (height / 2.0));

    SDL_RenderCopy(RENDERER, playerTexture, nullptr, &playerRect);
}





int Player::GetLives() const {
    return lives;
}

bool Player::OnScreen(int xMin, int xMax) const {
    return (xMin <= rectx && rectx <= xMax);
}

std::vector<double> Player::GetGamePosition() {
    return {gamex, gamey};
}

std::vector<double> Player::GetPlayerSpeed() {
    return {speedx, speedy};
}

std::vector<int> Player::GetPlayerWidthHeight() {
    return {width, height};
}

SDL_Texture* Player::GetTexture() {
    return playerTexture;
}


void Player::ForceVoidOut() {
    forcedVoid = true;
}

void Player::ResetPlayerPosition(int MIN_TRACK_HEIGHT) {
    // set position for game, offset values
    gamex = offsetx;
    gamey = offsety = MIN_TRACK_HEIGHT - height;

    // Set checkpoint position
    checkx = gamex;
    checky = gamey;

    // Set initial rect position
    rectx = 0;
    recty = MIN_TRACK_HEIGHT - height;
}

void Player::AddPlayerLives(int addLives) {
    lives += addLives;
}

void Player::SetPlayerSpeed(double horizontalSpeed) {
    speedx = horizontalSpeed;
}

void Player::SetPlayerJumpCalcVars(int playerJumpHeight, double playerGravity) {
    jumpHeight = playerJumpHeight;
    gravity = playerGravity;
}