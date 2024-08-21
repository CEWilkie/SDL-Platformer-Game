//
// Created by cew05 on 21/02/2024.
//

#ifndef CPP_PROGRAMMINGPROJECT_GAMEMENUANDWINDOWSETUP_CPP
#define CPP_PROGRAMMINGPROJECT_GAMEMENUANDWINDOWSETUP_CPP

#include "GameCore.h"

Menu::Menu(int w, int h) {
    width = w;
    height = h;

    // set bg, title rect values:
    bgImageRect = {0, 0, width, height};
}

void Menu::CreateTextures(SDL_Renderer* RENDERER) {
    /*
     * This function constructs and stores the textures and font that is used by the Start Screen for displaying text
     * and the background image. This includes the start prompt, and high scores label textures.
     */
    textWidgets = {};

    // create text font:
    std::string fontPath = "../Resources/CookieCrisp/CookieCrisp-L36ly.ttf";
    textFont = TTF_OpenFont(fontPath.c_str(), 200);

    // Create background image playerTexture
    std::string bgPath = "../Resources/Images/Backgrounds/MainCaveBackground.png";
    surface = IMG_Load(bgPath.c_str());
    bgImageTexture = SDL_CreateTextureFromSurface(RENDERER, surface);
    SDL_FreeSurface(surface);

    // Create start prompt playerTexture
    Text startPrompt;
    startPrompt.text = "PRESS SPACE TO BEGIN!";
    startPrompt.ConstructRect(width / 2, (height * 15/16),width / 2, height / 16.0);
    startPrompt.CreateTexture(RENDERER, textFont);
    textWidgets.push_back(startPrompt);

    // Create high scores text
    Text hsHeader;
    hsHeader.text = "Top 5 High Scores:";
    hsHeader.ConstructRect(width/2, height * 8/16, width/2, height / 16.0);
    hsHeader.CreateTexture(RENDERER, textFont);
    textWidgets.push_back(hsHeader);

    int xScore = width/3;
    int yScore = height * 9/16;

    for (HighScore &score : highScores) {
        score.scoreText.CreateTexture(RENDERER, textFont);
        score.dateText.CreateTexture(RENDERER, textFont);

        score.scoreText.ConstructRect(xScore, yScore, 0, height/16.0, true);
        score.dateText.ConstructRect(2 * xScore, yScore, 0, height/16.0, true);

        yScore += height/16;

        textWidgets.push_back(score.scoreText);
        textWidgets.push_back(score.dateText);
    }

    TTF_CloseFont(textFont);
}

void Menu::ObtainScoreInfo() {
    /*
     * This function obtains the 5 high scores that are stored within the file "PlayerScores.txt". The file is assumed
     * to exist.
     */
    std::fstream scoreFile("../RequiredFiles/PlayerScores.txt");
    std::string scoreLine;
    int maxDisplay = 5;
    std::string div = ", ";
    unsigned long long pos;

    while (std::getline(scoreFile, scoreLine)) {                                                                  // while there remains lines in the file
        if (highScores.size() > 4) break;                                                                               // only fetch maxDisplay number of scores
        pos = scoreLine.find(div);                                                                                  // find position of first ', ' in line

        try {                                                                                                           // ignore sections which begin without number:
            std::stoi(scoreLine.substr(0, pos));                                                             // try catch used to catch errors from lines that
            scoreLine.erase(0, pos + div.length());                                                              // dont begin with number
            maxDisplay -= 1;
        } catch (std::exception &e) {
            printf("MAXDISPLAY: %d | FAIL FROM %s ON NON-SCORE LINE\n", maxDisplay, e.what());
            continue; //next line
        }

        // obtain info from scores:
        std::vector<std::string> highScoreValues = {};
        while ((pos = scoreLine.find(div)) != std::string::npos) {                                                  // split data in line into separate strings. store in vector
            highScoreValues.push_back(scoreLine.substr(0, pos));
            scoreLine.erase(0, pos + div.length());
        }
        HighScore newScore;
        newScore.SetValues(highScoreValues);                                                                   // construct highScore using the strings stored in vector
        highScores.push_back(newScore);
    }
    scoreFile.close();

    // Sort scores
    std::sort(highScores.begin(), highScores.end(), [](HighScore &scoreA, HighScore &scoreB) {    // sorts scores from highest to lowest, descending.
        return std::stoi(scoreA.scoreText.text) > std::stoi(scoreB.scoreText.text);
    });
}

void Menu::SaveScore(HighScore& newScore) {
    /*
     * This function takes a HighScore struct passed to it from the GameLoop. The program first checks that the score is
     * valid, before adding it into the High Score list and sorting the list by score value. Following this, should the
     * list have > 5 high scores stored within it, the lowest one is removed from the list.
     */
    if (newScore.dateText.text == "void") return;                                                                       // catches non-scores from game section being closed before the gameplay starts
    int lowestScore = std::stoi(highScores.back().scoreText.text);
    int newScoreValue = std::stoi(newScore.scoreText.text);                                                         // check if greater than lowest value in current highScores.
    if (lowestScore >= newScoreValue && highScores.size() == 5) return;                                                 // or if there is less than 5 high scores currently

    highScores.push_back(newScore);                                                                                     // add new score to highScores

    std::sort(highScores.begin(), highScores.end(), [](HighScore &scoreA, HighScore &scoreB) {    // sorts scores from highest to lowest, descending.
        return std::stoi(scoreA.scoreText.text) > std::stoi(scoreB.scoreText.text);
    });

    if (highScores.size() > 5) highScores.pop_back();                                                                   // drop lowest score if theres more than 5 scores

    std::ofstream writeScoreFile("../RequiredFiles/PlayerScores.txt");
    for (int i = 0; i < highScores.size(); i++) {
        writeScoreFile << std::to_string(i+1) + ", " + highScores[i].GetScoreInfoString();
    }
    writeScoreFile.close();
}

void Menu::Display(SDL_Renderer* RENDERER) {
    /*
     * This function displays the background of the start screen, and then overlays all text on top of it by iterating
     * though a vector which holds all text objects.
     */

    SDL_RenderCopy(RENDERER, bgImageTexture, nullptr, &bgImageRect);                               // Display background image

    for (Text textWidget : textWidgets) {                                                                               // Display textWidgets
        textWidget.Display(RENDERER);
    }
}


#endif //CPP_PROGRAMMINGPROJECT_GAMEMENUANDWINDOWSETUP_CPP
