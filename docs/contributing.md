# Git guide

This is a very biginers and quick guide to git/github on how to contribute to the project.

## Setup
First, make sure you have the git software install on your computere, follow the instructions here: https://git-scm.com/book/en/v2/Getting-Started-Installing-Git

Next, create or log into github.com
Go to: https://github.com/pimoroni/32blit-beta

Click the Fork link, this will mirror the official repo (short for repository) into your github account and it will redirect you there automatically, ie: YourAccount/32blit-beta
While here, click the big green "Clone or download" button, you will copy the link provide, ie: https://github.com/YourAccount/32blit-beta.git

Now, on your computer navigate on your drive were you want your working local copy of the repo locatated, ie /home/YourAccount/repos
Then, type git clone and paste the link and hit enter, ie:
**git clone https://github.com/YourAccount/32blit-beta.git**

Where YourAccount is your github account

[more detailed info here..](https://git-scm.com/book/en/v2/Git-Basics-Getting-a-Git-Repository)


This will create a new folder called '32blit-beta' and download your repo here.
cd into that new folder, ie cd 32blit-beta
Here enter the following commads, with your info:
**git config --global user.name "Your Name"**
**git config --global user.email "your@email.com"**
Replace the values inside the quotes with your name and email address, this is to identify who made the canges and how to contact you, it's not your github account info.
[more detailed info here..](https://git-scm.com/book/en/v2/Getting-Started-First-Time-Git-Setup)

## Using git on your local repo

### listing changes
Firstly, navigate to your 32blit-beta repo folder to run all your git commands.
As a test create or modify a file in the repo and run this command:
**git status**

### Add command
It will list tracked and untracked files that have been changed.
We want git to track (or store) the changes of the files in a Staging Area (ie revision history), so we need to tell git to track them with the add command.
We can add the files individually or just add all the files recursively in the project with a single command:
**git add .**


[more detailed info on add here..](https://git-scm.com/book/en/v2/Getting-Started-Getting-Help) and (https://git-scm.com/book/en/v2/Git-Basics-Recording-Changes-to-the-Repository)


## Ignored files
view the file **.gitignore** in the repo root folder, it shows you which files will not be tracked and mostlikely you wont need to change these.


## local commit
To track your file changes over time, you will need to make (many) local commits to your files, run:
**git commit -m "Commit message"**

This command will store changes to all files modified, not in the **.gitignore** file.
The commit message should be a descriptive summary of the changes that you are committing to the repository.

## local/remote changes
git log
git diff 
git diff --staged
## Updating your remote repo

## Merging into offical repo


## Helpful links
* [Youtube video: Introduction to Git](https://www.youtube.com/watch?v=USjZcfj8yxE) [webpage: Introduction to Git] https://www.notion.so/Introduction-to-Git-ac396a0697704709a12b6a0e545db049) 
* [Cheat sheet] (https://github.github.com/training-kit)

