#include "menusDataSource.hpp"
#include <vector>

using namespace std;

void createMenuItems ();
vector<MenuItem> items;
vector<MenuItem> MenusDataSource::menuItems() {
    createMenuItems();
    return items;
};

vector<MenuItem>aboutMenuItems () {
    vector<MenuItem> about;

    // would be nice to have environment variables here from travis builds, no?
    about.push_back(MenuItem("Version", "1.0"));
    about.push_back(MenuItem("Build number", "1"));
    about.push_back(MenuItem("Build time", "01/01/20"));

    return about;
}

vector<OptionItem> difficultySelectionItems () {
    vector<OptionItem> options;

    options.push_back(OptionItem("Easy",0));
    options.push_back(OptionItem("Medium",1));
    options.push_back(OptionItem("Hard",2));
    options.push_back(OptionItem("Mega Hard",3));

    return options;
}

void createMenuItems () {

    items.push_back(
        MenuItem("Difficulty",
        difficultySelectionItems(),
        [](OptionItem option) {
            // option changed

            // Do something about it
        })
    );


    items.push_back(MenuItem("About",aboutMenuItems()));

    items.push_back(
        MenuItem("Exit Game",
        "Press A",
        "Are you sure?",
        [](){
            blit::switch_execution();
        })
    );
}



MenusDataSource::MenusDataSource () { }