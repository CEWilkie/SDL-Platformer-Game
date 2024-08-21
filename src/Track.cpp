//
// Created by cew05 on 21/02/2024.
//

#include "Track.h"

TrackObject::TrackObject(double x, double y, int w, int h) {
    objectStruct.Construct(x, y, w, h);
}

void TrackObject::CreateTexture(SDL_Renderer *RENDERER) {
    // set texture
    surface = IMG_Load(imgPath.c_str());
    objectTexture = SDL_CreateTextureFromSurface(RENDERER, surface);
    SDL_FreeSurface(surface);
}

void TrackObject::UpdateRect(Uint64 ELAPSED_TIME, double playerSpeed) {
    objectStruct.rectx = objectStruct.gamex - (double(ELAPSED_TIME) * playerSpeed);
}

void TrackObject::Display(SDL_Renderer* RENDERER) {
    if (!canDisplay) return;

    int rectx = int(objectStruct.rectx - objectStruct.width/2.0);                                                       // set rect position to top left of desired model position
    int recty = int(objectStruct.recty - objectStruct.height/2.0);
    selfRect = {rectx, recty, objectStruct.width, objectStruct.height};

    SDL_RenderCopy(RENDERER, objectTexture, nullptr, &selfRect);
}

bool TrackObject::PlayerCollision(Player player) const {
    if (!collisionEnabled){
        return false;
    }

    // get and store player position
    std::vector<double> playerPosition = player.GetGamePosition();

    // check x distance is ! <= to player width
    if (std::abs(objectStruct.gamex - playerPosition[0]) >
    (player.GetPlayerWidthHeight()[0] / 2.0 + objectStruct.width / 2.0)) return false;

    // check y distance is ! <= to player height
    if (std::abs(objectStruct.gamey - playerPosition[1]) >
    (player.GetPlayerWidthHeight()[1] / 2.0 + objectStruct.height / 2.0)) return false;

    // hence player is in bounds
    return true;
}


ObjectStruct *TrackObject::GetStruct() {
    return &objectStruct;
}





TrackObstacle::TrackObstacle(double x, double y, int w, int h) : TrackObject(x, y, w, h) {
    imgPath = "../Resources/Images/TrackObstacles/Boulder.png";
}

bool TrackObstacle::PlayerCollided(Player &player) {
    if (!PlayerCollision(player)) {
        return false;
    }

    printf("ACTIVATING PLAYER COLLISION RESPONSE\n");
    canDisplay = false;                                                                                                 // prevent obstacle from remaining as a constant hinderance to player
    collisionEnabled = false;                                                                                           // by hiding it and preventing future collisions to be detected

    return true;
}

int TrackObstacle::GetTrackIndex() const {
    return trackIndex;
}

void TrackObstacle::SetTrackIndex(int index) {
    trackIndex = index;
}




Track::Track(int MIN_TRACK_HEIGHT, int MAX_TRACK_HEIGHT, int w, int h) {
    TILE_WIDTH = w;
    TILE_HEIGHT = h;

    this->MIN_TRACK_HEIGHT = MIN_TRACK_HEIGHT;
    this->MAX_TRACK_HEIGHT = MAX_TRACK_HEIGHT;

    // Initialise rect
    trackRect = {0, 0, w, h};

    // Provide first trackSegment
    TrackSegment start;
    start.gamex = start.rectx = w/2.0;
    start.gamey = start.recty = MIN_TRACK_HEIGHT;
    start.trackIndex = 0;
    trackSegments.push_back(start);
}





void Track::ObtainTrackGenInfo(int DIFFICULTY) {
    sectionGenerationInfo.clear();                                                                                      // Empty previous generation info
    std::fstream trackGenerationInfoFile("../RequiredFiles/GenerationPercs.txt");                 // open file to read gen info
    std::string generationInfoLine;
    std::string div = ", ";
    unsigned long long pos;

    // Find line housing the difficulty generation information
    while (std::getline(trackGenerationInfoFile, generationInfoLine)) {                                           // while file is not empty
        pos = generationInfoLine.find(div);                                                                         // find first instance of ', '
        try {
            if (std::stoi(generationInfoLine.substr(0, pos)) == DIFFICULTY) {                                // ignore all lines where first value does not match the difficulty value
                generationInfoLine.erase(0, pos + div.length());                                                 // remove difficulty section
                break;                                                                                                  // exit while early since difficulty is found
            }
        }
        catch (std::exception &e) {                                                                                     // catches all lines where first value cannot be converted to string
            printf("DIFFICULTY: %d. Not difficulty line, %s failed\n", DIFFICULTY, e.what());
        }
    }
    trackGenerationInfoFile.close();

    // obtain track generation info
    while ((pos = generationInfoLine.find(div)) != std::string::npos) {                                             // while ', ' remains in the line
        std::string generationInfo = generationInfoLine.substr(0, pos);

        int infoValues[4] = {}, i = 0;
        unsigned long long infoPos;
        while ((infoPos = generationInfo.find(':')) != std::string::npos) {                                          // while ':' remains in the string
            infoValues[i] = std::stoi(generationInfo.substr(0, infoPos));                                    // split segment info values stored in generationInfo string into array of ints
            generationInfo.erase(0, infoPos+1);                                                                  // remove section from string
            i++;
        }

        sectionGenerationInfo.emplace_back();
        sectionGenerationInfo.back().Construct(infoValues[0], infoValues[1],
                                               infoValues[2], infoValues[3]);

        generationInfoLine.erase(0, pos + div.length());
    }
}





PossiblePosition Track::CreatePossiblePosition(TrackSegment prevSegment, int minLength, int maxLength) const {
    double verticalSpeed;                                                                                               // Jump speed of player
    double maxDy, minDy;                                                                                                // max and minimum change in y position (for the given deltaTicks)
    int dx;                                                                                                             // change in x position
    int deltaTicks;                                                                                                     // ticks taken to travel from end of prev section to mp of current tile

    std::vector<PossiblePosition> possibleHeights {};                                                                   // houses all possible heights player can jump/fall to

    for (int sl = minLength; sl <= maxLength; sl++) {
        dx = int((sl) * TILE_WIDTH);
        deltaTicks = int(dx / playerSpeed);

        // Follow jump charge method
        verticalSpeed = std::sqrt(2 * jumpHeight * GRAVITY);                                                         // used to determine how fast init velocity must be to travel certain height
        if (deltaTicks < 100) {
            verticalSpeed *= 0.4;
        }
        if (deltaTicks >= 100 && deltaTicks < 500) {
            verticalSpeed *= (0.4 + (deltaTicks/500.0)*0.6);
        }

        // obtain new values
        maxDy = (0.5 * GRAVITY * std::pow(deltaTicks, 2)) - (verticalSpeed * deltaTicks);
        minDy = (0.5 * GRAVITY * std::pow(deltaTicks, 2));

        // fix to multiples of height
        maxDy = int(maxDy / TILE_HEIGHT) * TILE_HEIGHT;
        minDy = int(minDy / TILE_HEIGHT) * TILE_HEIGHT;

        // check that newly calculated bounds are within acceptable min / max track heights
        if (prevSegment.gamey + maxDy >= MAX_TRACK_HEIGHT &&
            prevSegment.gamey + maxDy <= MIN_TRACK_HEIGHT && maxDy != 0) {
            possibleHeights.emplace_back();
            possibleHeights.back().x = prevSegment.gamex + dx;
            possibleHeights.back().y = prevSegment.gamey + maxDy;
        }

        if (prevSegment.gamey + minDy >= MAX_TRACK_HEIGHT &&
            prevSegment.gamey + minDy <= MIN_TRACK_HEIGHT && minDy != 0) {
            possibleHeights.emplace_back();
            possibleHeights.back().x = prevSegment.gamex + dx;
            possibleHeights.back().y = prevSegment.gamey + minDy;
        }
    }

    // Determine segment x and y positions
    PossiblePosition position {};
    if (possibleHeights.empty()) {                                                                                      // no possible heights with provided info
        position.x = 0; position.y = 0;                                                                                 // set values to 0 so they can be checked for later
        return position;
    }

    int ranPos = std::rand() % int(possibleHeights.size()) + 0;                                                         // construct track at random possible height
    position.x = (minLength == 1) ? prevSegment.gamex + TILE_WIDTH : possibleHeights[ranPos].x;                         // if minlength is 1, then no empty space between segments
    position.y = possibleHeights[ranPos].y;

    return position;
}





void Track::ConstructTrack(int DIFFICULTY) {
    ObtainTrackGenInfo(DIFFICULTY);                                                                                     // Fetch generation rates for the difficulty level
    endOfTrackIndex = 150 + DIFFICULTY * 20;                                                                            // Determine length of track
    nextCheckpointPosition = 5000;                                                                                      // Determine first checkpoint position

    trackSegments.clear();                                                                                              // Empty out the previous track segments
    trackObstacles.clear();                                                                                             // Empty out the previous track obstacles
    maxObstacles = 5 + DIFFICULTY;

    PossiblePosition nextPosition {};                                                                                   // Set initial position of track
    nextPosition.y = MIN_TRACK_HEIGHT;
    nextPosition.x = TILE_WIDTH / 2.0;
    int sectionLength;

    ConstructSegment(nextPosition, 20, "start");                                              // Construct the starting zone of the track

    // ConstructRect the main gameplay body of the track
    while (trackSegments.size() < endOfTrackIndex) {
        int rJumpVal = std::rand() % 100 + 0;
        int rTrackVal = std::rand() % 100 + 0;

        for (int i = 0; i < sectionGenerationInfo.size() - 2; i++) {                                                    // Determine which track segment to be generated next
            if (sectionGenerationInfo[i].WithinGenBounds(rTrackVal)) {
                int minLen = sectionGenerationInfo[i].minLen;
                int maxLen = sectionGenerationInfo[i].maxLen;
                sectionLength = std::rand() % (maxLen - minLen + 1) + minLen;
            }
        }

        if (sectionGenerationInfo[4].WithinGenBounds(rJumpVal)) {                                              // determine if jump is to be generated before the segment and create possible position
            nextPosition = CreatePossiblePosition(trackSegments.back(),
                                                  sectionGenerationInfo[4].minLen,
                                                  sectionGenerationInfo[4].maxLen);
        } else {                                                                                                        // no jump to be generated, determine possible position without jump
            nextPosition = CreatePossiblePosition(trackSegments.back(),
                                                  1,
                                                  sectionLength);
        }

        if (!ConstructSegment(nextPosition, sectionLength)) {
            printf("FAILED TO PRODUCE SECTION | CURR SECTIONS : %zu\n", trackSegments.size());
        }
    }

    startOfTrackIndex = trackSegments[20].trackIndex;                                                                   // fetch trackindex values of first and last tracks of the main body
    endOfTrackIndex = trackSegments[int(trackSegments.size()) - 1].trackIndex;

    // Construct the ending zone of the track
    nextPosition.y = trackSegments.back().gamey;
    printf("END | ex: %f, ey: %f\n", nextPosition.x, nextPosition.y);
    printf("FINAL SIZE | %d\n", int(trackSegments.size()));
    printf("GENERATED %zu OBSTACLES\n", trackObstacles.size());
    ConstructSegment(nextPosition, 80, "end");
}






bool Track::ConstructSegment(PossiblePosition &fromPosition, int sectionLength, const std::string& sectionType) {
    if (fromPosition.y == 0) {                                                                                          // no possible position so return failure
        return false;                                                                                                   // failed to make section
    }

    printf("LENGTH %d | FROM POS x: %f, y: %f |", sectionLength, fromPosition.x, fromPosition.y);

    // Determine if checkpoint is to be made
    bool addCheckpoint = false;
    if (trackSegments.back().gamex >= nextCheckpointPosition) {
        addCheckpoint = true;
        nextCheckpointPosition += 5000;
        printf(" CHECKPOINT GEN AT x: %f |", fromPosition.x);
    }

    // Determine if obstacle is to be made
    int obIndex = -1;
    if (sectionGenerationInfo[5].WithinGenBounds(std::rand() % 100 + 0)) {
        // Determine section index
        if (!addCheckpoint && sectionType.empty() && maxObstacles > 0){
            obIndex = std::rand() % (sectionLength) + 1;
            maxObstacles -= 1;
        }
    }

    // ConstructRect section:
    for (int s = 0; s < sectionLength; s++) {
        TrackSegment newTrack {};                                                                                       // create basic trackSegment
        newTrack.Construct(fromPosition.x + (TILE_WIDTH * s), fromPosition.y, TILE_WIDTH, TILE_HEIGHT);        // set position and size
        newTrack.SetIndex();
        newTrack.checkpoint = addCheckpoint;

        if (sectionType == "end") newTrack.endLevel = true;
        if (sectionType == "start") newTrack.startLevel = true;
        if (addCheckpoint) addCheckpoint = false;
        if (s == obIndex) {
            TrackObstacle newObstacle(newTrack.gamex, newTrack.gamey - TILE_HEIGHT,TILE_WIDTH, TILE_HEIGHT);// create new obstacle, set position and size
            newObstacle.SetTrackIndex(newTrack.trackIndex);
            trackObstacles.push_back(newObstacle);

            printf(" OBSTACLE GEN AT x: %f |", newTrack.gamex);
        }

        trackSegments.push_back(newTrack);
    }

    fromPosition.y = MIN_TRACK_HEIGHT + 1;
    fromPosition.x = trackSegments.back().gamex + TILE_WIDTH;

    printf("\n");

    return true;
}

void Track::UpdateTrackRects(Uint64 ELAPSED_TIME) {
    // Update track rect values to display correctly
    for (TrackSegment &trackSegment : trackSegments) {
        trackSegment.rectx = trackSegment.gamex - (double(ELAPSED_TIME) * playerSpeed);
    }

    for (TrackObject &obstacle : trackObstacles) {
        obstacle.UpdateRect(ELAPSED_TIME, playerSpeed);
    }
}



bool Track::CheckForCollision(Player& player) {
    for (TrackObstacle &obstacle : trackObstacles) {
        if (obstacle.PlayerCollided(player)) return true;
    }

    return false;
}





void Track::CreateTextures(SDL_Renderer *RENDERER) {
    // Create toplevel Texture
    std::string trackToplevelPath = "../Resources/Images/Track/TrackToplevel.png";
    surface = IMG_Load(trackToplevelPath.c_str());
    toplevelTexture = SDL_CreateTextureFromSurface(RENDERER, surface);
    SDL_FreeSurface(surface);

    // Create toplevel bg deco texture
    std::string ttlBgDecoPath = "../Resources/Images/Track/TrackToplevelBGDeco.png";
    surface = IMG_Load(ttlBgDecoPath.c_str());
    ttlBgTexture = SDL_CreateTextureFromSurface(RENDERER, surface);
    SDL_FreeSurface(surface);

    // Create track checkpoint deco texture
    std::string checkpointImgPath = "../Resources/Images/Track/TrackToplevelCheckpointDeco.png";
    surface = IMG_Load(checkpointImgPath.c_str());
    ttlCheckpointTexture = SDL_CreateTextureFromSurface(RENDERER, surface);
    SDL_FreeSurface(surface);

    // Create filler texture
    std::string trackFillerPath = "../Resources/Images/Track/TrackFiller.png";
    surface = IMG_Load(trackFillerPath.c_str());
    trackFillerTexture = SDL_CreateTextureFromSurface(RENDERER, surface);
    SDL_FreeSurface(surface);

    // Create background image texture
    std::string backgroundPath = "../Resources/Images/Backgrounds/MainCaveBackground.png";
    surface = IMG_Load(backgroundPath.c_str());
    backgroundTexture = SDL_CreateTextureFromSurface(RENDERER, surface);
    SDL_FreeSurface(surface);

    // Create obstacle textures
    for (TrackObstacle &obstacle : trackObstacles) {
        obstacle.CreateTexture(RENDERER);
    }
}

void Track::DisplayTrack(SDL_Renderer* RENDERER, SDL_Window* WINDOW) {
    int xMax, yMax;
    SDL_GetWindowSize(WINDOW, &xMax, &yMax);

    for (TrackSegment trackSegment : trackSegments) {
        // only display if track segment within window area
        if (trackSegment.rectx > xMax + TILE_WIDTH/2.0 && trackSegment.rectx < 0 - TILE_WIDTH/2.0) return;

        // display toplevel
        trackRect.x = int(trackSegment.rectx - TILE_WIDTH/2.0);
        trackRect.y = int(trackSegment.recty - TILE_HEIGHT/2.0);
        SDL_RenderCopy(RENDERER, toplevelTexture, nullptr, &trackRect);

        // display toplevel bg deco, switch to checkpoint deco if track is a checkpoint
        SDL_Texture* toplevelDeco = (trackSegment.checkpoint) ? ttlCheckpointTexture : ttlBgTexture;

        trackRect.y -= TILE_HEIGHT;
        SDL_RenderCopy(RENDERER, toplevelDeco, nullptr, &trackRect);
        trackRect.y += TILE_HEIGHT;

        // Display track filler
        while ((trackRect.y += TILE_HEIGHT) <= yMax  + TILE_HEIGHT/2.0) {
            SDL_RenderCopy(RENDERER, trackFillerTexture, nullptr, &trackRect);
        }
    }

    for (TrackObject &obstacle : trackObstacles) {
        obstacle.Display(RENDERER);
    }
}

void Track::DisplayBackground(SDL_Renderer* RENDERER, SDL_Window *WINDOW) {
    SDL_GetWindowSize(WINDOW, &backgroundRect.w, &backgroundRect.h);

    SDL_RenderCopy(RENDERER, backgroundTexture, nullptr, &backgroundRect);
}






TrackSegment Track::GetTrackAtIndex(int trackIndex) {
    if (trackIndex == -1) return trackSegments.back();
    for (TrackSegment track : trackSegments) {
        if (track.trackIndex == trackIndex) return track;
    }
    return trackSegments[startOfTrackIndex];
}

std::vector<int> Track::GetTrackWidthHeight() {
    return {TILE_WIDTH, TILE_HEIGHT};
}

bool Track::IsObstacleAtIndex(int trackIndex) {
    return std::any_of(trackObstacles.begin(), trackObstacles.end(), [&trackIndex](TrackObstacle &obstacle){
        return (obstacle.GetTrackIndex() == trackIndex);
    });
}

std::vector<int> Track::GetTrackStartEndIndex() {
    return {startOfTrackIndex, endOfTrackIndex};
}





void Track::SetWidthHeight(int w, int h) {
    TILE_WIDTH = w;
    TILE_HEIGHT = h;
}

void Track::SetPlayerJumpCalcVars(double trackJumpHeight, double trackGravity, double trackSpeed) {
    jumpHeight = trackJumpHeight;
    GRAVITY = trackGravity;
    playerSpeed = trackSpeed;
}