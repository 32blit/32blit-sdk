#include "menusDataSource.hpp"
#include <vector>

using namespace std;

void createMenuItems ();
vector<MenuItem> items;
vector<MenuItem> MenusDataSource::menuItems() {
    createMenuItems();
    return items;
};

static vector<MenuItem>aboutMenuItems () {
    return {
        {"Version","1.0"},
        {"Build number","120"},
        {"Name","FLIGHT!"}
    };
}

static vector<OptionItem> difficultySelectionItems () {
    return {
        {"Easy",0},
        {"Medium",1},
        {"Hard",2},
        {"Mega Hard!",3}
    };
}

void createMenuItems () {

    items.emplace_back(
        MenuItem("Difficulty",
        difficultySelectionItems(),
        [](OptionItem option) {
            // option changed

            // Do something about it
        })
    );


    items.emplace_back(MenuItem("About",aboutMenuItems()));

    items.emplace_back(
        MenuItem("Exit Game",
        "Press A",
        "Are you sure?",
        [](){
            blit::switch_execution();
        })
    );
}



MenusDataSource::MenusDataSource () { }