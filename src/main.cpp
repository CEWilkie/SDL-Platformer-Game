//
// Created by cew05 on 20/02/2024.
//
#include "GameCore.h"
#include "Track.h"
#include "Player.h"
#include "TrackBonusItems.h"

HighScore GameLoop(std::string &viewScreen, SDL_Window* WINDOW, SDL_Renderer* RENDERER) {
    /*
     * The GameLoop function is where the programs actual gameloop is held. Within this function, the program will
     * repeat the game object update cycle until the player either presses the close button or runs out of lives
     * causing a game-over. The loop returns a HighScore struct to be tested against the prior High Scores.
     */

    HighScore newScore;                                                                                                 // Create new score to save game results into
    newScore.SetValues({"void", "void", "void"});                                                              // Provide rubbish data so program knows an invalid score is returned
    if (viewScreen != "game") return newScore;                                                                          // not game view so dont load the gameloop

    //  CONSTRUCT THE "GLOBAL" VARS THAT MANAGE THE GAME
    int GAME_WINDOW_HEIGHT, GAME_WINDOW_WIDTH;
    SDL_GetWindowSize(WINDOW, &GAME_WINDOW_WIDTH, &GAME_WINDOW_HEIGHT);                                           // fetch window size

    int TILE_WIDTH = 48;                                                                                                // set w h of game tile
    int TILE_HEIGHT = 48;

    int MIN_TRACK_HEIGHT = GAME_WINDOW_HEIGHT - int(2.5 * TILE_HEIGHT);                                                 // set min and max height of track generation
    int MAX_TRACK_HEIGHT = MIN_TRACK_HEIGHT - (10 * TILE_HEIGHT);

    int DIFFICULTY = 0;                                                                                                 // Set difficulty of the game to base level 0
    int JUMPHEIGHT = 6*TILE_HEIGHT;
    double GRAVITY = JUMPHEIGHT / (2*std::pow(1500/4.0, 2));

    const unsigned int RANDOM_SEED = time(nullptr);                                                                // seed the random calculations randomly
    //const unsigned int RANDOM_SEED = 1712404869;                                                                      // seed the random calculations predictably
    std::srand(RANDOM_SEED);
    printf("Using SEED: %u\n", RANDOM_SEED);

    // ConstructRect track, player and ScoreCollectables class
    Player player {TILE_WIDTH, TILE_HEIGHT, MIN_TRACK_HEIGHT};
    Track track {MIN_TRACK_HEIGHT, MAX_TRACK_HEIGHT, TILE_WIDTH, TILE_HEIGHT};
    ScoreCollectables collectables{};

    // Set player states:
    player.SetPlayerSpeed((TILE_WIDTH/240.0) * (1 + DIFFICULTY*0.1));                                      // takes 240 gameticks to cross 1 tile, increases with difficulty
    player.SetPlayerJumpCalcVars(JUMPHEIGHT, GRAVITY);

    // Set track states:
    track.SetPlayerJumpCalcVars(JUMPHEIGHT, GRAVITY, player.GetPlayerSpeed()[0]);

    // CREATE FIRST LEVEL TRACK
    // Create track
    track.ConstructTrack(DIFFICULTY);

    // Create collectibles
    collectables.ConstructCoins(track);
    collectables.ConstructSpecials(track);

    // Create Textures
    player.CreateTexture(RENDERER);
    track.CreateTextures(RENDERER);
    collectables.CreateTextures(RENDERER);

    // end of level adding score/lives management
    bool endOfTrackScoreAdded = false;
    bool endOfTrackLivesAdded = false;

    // set deltaTicks vars
    Uint64 prevtick = SDL_GetTicks64();
    Uint64 tickchange = 0;                                                                                              // for retaining tick change since SDL_GetTicks64() last called
    Uint64 scoreTick = 0;                                                                                               // for determining when player recieves score over time
    Uint64 ELAPSED_TIME = 0;                                                                                            // Set elapsed time of the game to 0

    // game pausing control
    bool paused = false;

    while (viewScreen == "game" && player.GetLives() > -1) {
        // update deltaTick vars
        ELAPSED_TIME += tickchange;
        scoreTick += tickchange;                                                                                        // tickchange prevented from increasing whilst game paused
        tickchange = (paused) ? 0 : SDL_GetTicks64() - prevtick;                                                        // which prevents player position from changing
        prevtick = SDL_GetTicks64();                                                                                    // and track/collectable rect positions from changing

        // check for close event
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    viewScreen = "end";                                                                                 // breaks while loop for gameloop
                    break;

                default:
                    break;
            }
        }

        // check keyinput events
        const std::uint8_t *keystates = SDL_GetKeyboardState(nullptr);
        if (keystates[SDL_SCANCODE_P]) paused = true;                                                                   // pause the game
        if (keystates[SDL_SCANCODE_R]) paused = false;                                                                  // unpause the game
        if (keystates[SDL_SCANCODE_SPACE]) player.Jump(true);                                                  // player jump management
        if (!keystates[SDL_SCANCODE_SPACE]) player.Jump(false);                                                // player jump management (player not holding jump key)

        // MAIN GAMEPLAY HANDLING
        if (!player.AtEndOfTrack(track)) {
            // update player
            player.MovementUpdate(track, tickchange, ELAPSED_TIME);
            player.CheckVoidOut(GAME_WINDOW_HEIGHT, ELAPSED_TIME);

            // Update track
            track.UpdateTrackRects(ELAPSED_TIME);

            // Update collectables
            collectables.UpdateCoins(player, ELAPSED_TIME);
            collectables.UpdateSpecials(player, ELAPSED_TIME);

            // Check for obstacle collision:
            if (track.CheckForCollision(player)) {
                player.ForceVoidOut();
            }

            // apply score over time
            if (scoreTick >= 50) {
                collectables.AddScore(5);
                scoreTick = (scoreTick > 50) ? scoreTick - 50 : 0;
            }
        }

        // END OF LEVEL HANDLING : ENDING MOVEMENT + SCORE + LIVES
        if (player.OnScreen(0, GAME_WINDOW_WIDTH) && player.AtEndOfTrack(track)) {
            // End of level handling:
            track.UpdateTrackRects(ELAPSED_TIME);

            // Award score for completing level
            if (!endOfTrackScoreAdded) {
                collectables.AddScore(3000);
                endOfTrackScoreAdded = true;
            }

            // Check if player has collected all 3 collectables
            if (collectables.CollectablesObtained() && !endOfTrackLivesAdded) {
                player.AddPlayerLives(1);
                endOfTrackLivesAdded = true;
            }
        }

        // END OF LEVEL HANDLING : NEW LEVEL GENERATION
        if (!player.OnScreen(0, GAME_WINDOW_WIDTH)) {
            // Increase Difficulty (max 6)
            DIFFICULTY = (DIFFICULTY < 6) ? DIFFICULTY + 1 : 6;

            // Reset player position, increase speed
            player.ResetPlayerPosition(MIN_TRACK_HEIGHT);
            player.SetPlayerSpeed((TILE_WIDTH/240.0) * (1 + DIFFICULTY*0.1));

            // Reset track, set new jump calc vars (for changed playerspeed)
            track.SetPlayerJumpCalcVars(JUMPHEIGHT, GRAVITY, player.GetPlayerSpeed()[0]);
            track.ConstructTrack(DIFFICULTY);
            track.CreateTextures(RENDERER);

            // Reset collectables
            collectables.ConstructCoins(track);
            collectables.ConstructSpecials(track);
            collectables.CreateTextures(RENDERER);

            endOfTrackScoreAdded = false;
            endOfTrackLivesAdded = false;

            // Reset elapsed time
            ELAPSED_TIME = 0;
        }

        // DISPLAY HANDLING
        track.DisplayBackground(RENDERER, WINDOW);
        track.DisplayTrack(RENDERER, WINDOW);

        player.Display(track, ELAPSED_TIME, RENDERER);

        collectables.DisplayItems(RENDERER, 0, GAME_WINDOW_WIDTH);
        collectables.DisplayScoreLives(RENDERER, WINDOW, player);

        SDL_RenderPresent(RENDERER);
        SDL_RenderClear(RENDERER);
    }

    // HANDLE END OF GAME
    if (viewScreen != "end") viewScreen = "menu";                                                                       // viewScreen "end" is a request to close the program fully

    // Get date:
    char timeString[sizeof("yyyy-mm-ddThh:mm:ssZ")];
    time_t t = time(nullptr);                                                                                     // Gets the current time
    std::strftime(timeString, sizeof(timeString), "%d/%m/%Y", localtime(&t));
    newScore.SetValues({timeString,
                        std::to_string(collectables.GetScore()),
                        std::to_string(RANDOM_SEED)}
                        );

    printf("%s\n", newScore.GetScoreInfoString().c_str());
    return newScore;
}



void MenuLoop(std::string &viewScreen, Menu menu, SDL_Renderer* RENDERER) {
    /*
     * This is the MenuLoop, where the user is held whilst the start screen should be showing. THe program will remain
     * within this loop until the user either presses the start game button, or the user closes the window.
     * The loop will otherwise continue to display the start screen information.
     */
    if (viewScreen != "menu") return;

    while (viewScreen == "menu") {
        // check for close event
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    viewScreen = "end";
                    break;
            }
        }

        // check keyinput events
        const std::uint8_t *keystates = SDL_GetKeyboardState(nullptr);
        if (keystates[SDL_SCANCODE_SPACE]) viewScreen = "game";

        menu.Display(RENDERER);

        // Update screen display
        SDL_RenderPresent(RENDERER);
        SDL_RenderClear(RENDERER);
    }
}




int main()
/*
 * Initialise SDL and SDL_ttf, create a window for the program and set it to the correct dimensions. Within main, the
 * program constructs a loop based upon the contents of var viewScreen. The loop will only end once the user closes
 * the SDL window, causing viewScreen to be set to "end". SDL and SDL_ttf are uninitialised and the program ended.
 */

{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0){                                                                       // Initialise SDL
        printf("Error initialising SDL: %s", SDL_GetError());
        return 0;
    }

    if (TTF_Init() != 0){                                                                                               // Initialise TTF
        printf("Error initialising TTF: %s", TTF_GetError());
        return 0;
    }

    // Create Windowed-Fullscreen Game Window

    SDL_Rect winRect = {0, 0, 500, 500};
    if (SDL_GetDisplayUsableBounds(0, &winRect) != 0) {
        printf("error obtaining desktop screen size: %s\n", SDL_GetError());
    }

    SDL_Window* WINDOW = SDL_CreateWindow("GameWindow", winRect.x, winRect.y,winRect.w, winRect.h, 0);        // Create test window to fetch bounds from

    int top = 0, bottom = 0, left = 0, right = 0;                                                                       // set border widths default values
    if (SDL_GetWindowBordersSize(WINDOW, &top, &left, &bottom, &right) != 0){                                           // obtain border widths of screen
        printf("Error obtaining window border sizes: %s", SDL_GetError());
    }

    winRect.y = top;                                                                                                    // reposition window to fit within screen
    winRect.h -= bottom + top;                                                                                          // change window size to fit within screen
    winRect.w = winRect.h * 9/7;                                                                                        // change window size to fit ratio

    SDL_SetWindowSize(WINDOW, winRect.w, winRect.h);                                                                    // set window size
    SDL_SetWindowPosition(WINDOW, SDL_WINDOWPOS_CENTERED, winRect.y);                                                // set window to centre of screen
    printf("WINDOW SIZE | w: %d, h: %d", winRect.w, winRect.h);

    SDL_Renderer* RENDERER = SDL_CreateRenderer(WINDOW, -1, SDL_RENDERER_ACCELERATED);                       // Create renderer to render images to SDL window

    Menu menu(winRect.w, winRect.h);                                                                                    // Construct start screen menu
    menu.ObtainScoreInfo();                                                                                             // Fetch high score info stored in "GenerationPercs.txt"
    menu.CreateTextures(RENDERER);

    std::string viewScreen = "menu";
    while (viewScreen != "end"){                                                                                        // The program loop ends when user hits the close button
        MenuLoop(viewScreen, menu, RENDERER);                                                                        // loads either the game or menu,
        HighScore newScore = GameLoop(viewScreen, WINDOW, RENDERER);                                                 // depending on what viewScreen is set to
        menu.SaveScore(newScore);
        menu.CreateTextures(RENDERER);
    }

    TTF_Quit();                                                                                                         // Deinitialise libraries for safe program exit
    SDL_Quit();

    return 0;
}