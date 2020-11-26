// Copyright
#include <time.h>
#include <unistd.h>
#include <vector>
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

char const* levelDifficulties[9] = {"Auto", "Demon", "NA", "n", "Easy",
                                    "Normal", "Hard", "Harder", "Insane"};
template<typename ... Args>
std::string string_format(const std::string& format, Args ... args ) {
    size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1;
    if (size <= 0)
        throw std::runtime_error("Error during formatting.");
    std::unique_ptr<char[]> buf(new char[size]);
    snprintf(buf.get(), size, format.c_str(), args ...);
    return std::string(buf.get(), buf.get() + size - 1);
}

enum LevelType {
    NONE = 0,
    OFFICIAL,
    EDITOR,
    SAVED,
    ONLINE,
};
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

float findPercent(PlayLayer* pl) {
    auto playerobj = pl->_player1();
    return (playerobj->_positionX() / pl->_length())*100.;
}
void inject() {
    int scene;

    std::string state("empt");
    std::string details("empt");
    std::string smallImage;
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
                std::string objectCount(std::to_string(objects));

                std::ostringstream ss;
                ss << levelName << " (" << objectCount << " objects)";
                state = ss.str();

                details = "Editing level";
            }
        } else {    // playing a level
            float percent = findPercent(playLayer);
            int attempt = playLayer->_attempt();
            int bestNormal = level->_bestNormal();
            int bestPractice = level->_bestPractice();

            char const* mode = "normal";
            if (playLayer->_practiceMode())
                mode = "practice";

            //  gathering level data

            char const* gameMode = findGameMode(playLayer);

            int levelId = level->_levelId();
            int levelStars = 69;

            enum LevelType levelType = ONLINE;
            if (levelId < 128)
                levelType = OFFICIAL;
            if (editorLayer)
                levelType = EDITOR;
            bool levelIsFeatured = false;
            bool levelIsEpic = true;

            std::string levelName(level->_name());
            std::string levelCreator(level->_author());
            std::string levelDifficulty("ez");

            if (levelType == OFFICIAL) {
                levelCreator = "RobTop";
                printf("%d\n", levelId);
                levelDifficulty = std::string(officialDifficulties[levelId-1]);
                levelIsFeatured = false;
                levelIsEpic = false;
            } else if (levelType == EDITOR) {
                levelDifficulty = "na";
            }

            printf("what\n");
            printf("%f %d %d\n", percent, bestNormal, bestPractice);


            state = "Attempt " + std::to_string(attempt) +
                    " [" + std::to_string(static_cast<int>(percent)) +
                    "%, best is " + std::to_string(bestNormal) + "%]";

            details = levelName + " by " + std::string(levelCreator);

            if (levelType == EDITOR)
                details = details + " (editing)";

            if (levelType != OFFICIAL)
                details = details + " (" + std::to_string(levelId) + ")";

            smallText = std::to_string(levelStars) + "star, " +
                        std::string(levelDifficulty) +
                        " (ID: " + std::to_string(levelId) + ")";
        }

        updateDiscordPresence(state.c_str(), details.c_str(),
                              "easy-featured", smallText.c_str());
        sleep(1);
    }
}
