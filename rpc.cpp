// Copyright
#include <unistd.h>
#include <memory>
#include <string>
#include <gdstdlib.hpp>
#include "disc.hpp"

char const* sceneEnum[14] = {"what", "Idle", "Selecting level",
                                      "old_my_levels", "Watching level info",
                                      "Searching levels", "unused",
                                      "Browsing leaderboards",
                                      "Online", "Selecting official level",
                                      "Playing official level", "none", "none",
                                      "the_challenge"
                                      };
char const* modeEnum[7] = {"Cube", "Ship", "UFO", "Ball",
                           "Wave", "Robot", "Spider"};
char const* officialDifficulties[21] = {"easy", "easy", "normal",
                                        "normal", "hard", "hard", "harder",
                                        "harder", "harder", "insane", "insane",
                                        "insane", "insane", "easy_demon",
                                        "insane", "insane", "harder",
                                        "easy_demon", "harder", "medium_demon",
                                        "insane"};
char const* levelTypes[5] = {"none", "official", "editor", "saved", "online"};

char const* levelDifficulties[14] = {"Auto", "Demon", "NA", "n", "Easy",
                                    "Normal", "Hard", "Harder", "Insane",
                                    "Easy Demon", "Medium Demon", "Hard Demon",
                                    "Insane Demon", "Extreme Demon"};

char const* imageDifficulties[14] = {"auto", "demon", "na", "n", "easy",
                                    "normal", "hard", "harder", "insane",
                                    "easy-demon", "medium-demon", "hard-demon",
                                    "insane-demon", "extreme-demon"};


enum LevelType {
    NONE = 0,
    OFFICIAL,
    EDITOR,
    SAVED,
    ONLINE,
};

char const* findDifficulty(GJGameLevel* lv) {
    auto diff = lv->_difficulty();
    int add = 3;
    if (!diff.denominator)
        return "NA";
    if (lv->_demon())
        add += 5;

    return levelDifficulties[diff.numerator/diff.denominator + add];
}

char const* findGameMode(PlayLayer* pl) {
    GameModes gm = pl->_gameModes();
    if (gm.cube)
        return "Cube";
    if (gm.ship)
        return"Ship";
    if (gm.ball)
        return "Ball";
    if (gm.ufo)
        return "UFO";
    if (gm.wave)
        return "Wave";
    if (gm.robot)
        return "Robot";
    if (gm.spider)
        return "Spider";

    return "Cube";
}

char const* genSmallImage(GJGameLevel* lv) {
    auto diff = lv->_difficulty();

    if (!lv->_levelId())
        return "na";
    if (lv->_levelId() < 128)
        return officialDifficulties[lv->_levelId()-1];
    if (!diff.denominator)
        return "na";
    int add = 3;
    if (lv->_demon())
        add += 5;
    std::string dBase(imageDifficulties[diff.numerator/diff.denominator + add]);

    if (lv->_epic()) {
        dBase += "-epic";
    } else if (lv->_score()) {
        dBase += "-featured";
    }

    return dBase.c_str();
}

float findPercent(PlayLayer* pl) {
    auto playerobj = pl->_player1();
    return (playerobj->_positionX() / pl->_length())*100.;
}

void inject() {
    int scene;

    std::string state("empt");
    std::string details("empt");
    std::string smallImage("");
    std::string smallText("empt");

    auto accountManager = GJAccountManager::sharedState();
    auto gameManager = GameManager::sharedState();
    auto playLayer = gameManager->_playLayer();
    auto editorLayer = gameManager->_editorLayer();
    auto player = playLayer->_player1();
    auto username = accountManager->_username();
    auto levelSettings = playLayer->_levelSettings();
    auto level = levelSettings->_level();

    discordInit();

    while (1) {
        playLayer = gameManager->_playLayer();
        editorLayer = gameManager->_editorLayer();

        player = playLayer->_player1();
        username = accountManager->_username();
        levelSettings = playLayer->_levelSettings();
        level = levelSettings->_level();

        if (!username) {
            username = "Player";
        }

        if (!level) {    // not playing level
            if (!editorLayer) {
                scene = gameManager->_scene();
                details = std::string(sceneEnum[scene+1]);
                state = "";
            } else {  // in the editor
                auto editorLevel = editorLayer->_levelSettings()->_level();
                std::string levelName(editorLevel->_name());

                int objects = editorLayer->_objects()->count();

                state = levelName + " (" +
                        std::to_string(objects) + " objects)";
                details = "Editing level";
            }
        } else {    // playing a level
            float percent = findPercent(playLayer);
            int attempt = playLayer->_attempt();
            int bestNormal = level->_bestNormal();
            int bestPractice = level->_bestPractice();

            //  gathering level data

            char const* gameMode = findGameMode(playLayer);

            int levelId = level->_levelId();
            int levelStars = level->_stars();

            enum LevelType levelType = ONLINE;
            if (levelId < 128)
                levelType = OFFICIAL;
            if (editorLayer)
                levelType = EDITOR;
            if (!levelId)
                levelType = SAVED;


            std::string levelName(level->_name());
            std::string levelCreator(level->_author());
            std::string levelDifficulty(findDifficulty(level));

            smallImage = std::string(genSmallImage(level));

            if (levelType == OFFICIAL) {
                levelCreator = "RobTop";
                levelDifficulty = std::string(officialDifficulties[levelId-1]);
            } else if (levelType == EDITOR) {
                levelDifficulty = "na";
            }

            if (!playLayer->_practiceMode()) {
                state = "Attempt " + std::to_string(attempt) +
                        " [" + std::to_string(static_cast<int>(percent)) +
                        "%, best is " + std::to_string(bestNormal) + "%]";
            } else {
                state = "Practice mode [" +
                        std::to_string(static_cast<int>(percent)) +
                        "%, best is " + std::to_string(bestPractice) + "%]";
            }

            details = levelName;
            if (levelType != SAVED)
                details = details + " by " + std::string(levelCreator);

            if (levelType == EDITOR)
                details = details + " (editing)";

            if (levelType == ONLINE)
                details = details + " (" + std::to_string(levelId) + ")";

            if (levelType == SAVED)
                details = details + " (Local level)";

            smallText = std::to_string(levelStars) + " stars, " +
                        std::string(levelDifficulty) +
                        " (ID: " + std::to_string(levelId) + ")";
        }

        updateDiscordPresence(state.c_str(), details.c_str(),
                              smallImage.c_str(), smallText.c_str());
        sleep(1);
    }
}
