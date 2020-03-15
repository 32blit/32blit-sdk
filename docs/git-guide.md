# Git guide

A Basic git/github instructions on how to contribute to the project.


## Setup
First, make sure you have the git software install on your computere, follow the instructions here: https://git-scm.com/book/en/v2/Getting-Started-Installing-Git

Next, create or log into github.com
Go to: https://github.com/pimoroni/32blit-beta

Click the Fork link, this will mirror the official repo (short for repository) into your github account and it will redirect you there automatically, ie: YourAccount/32blit-beta
While here, click the big green "Clone or download" button, you will copy the link provide, ie: https://github.com/YourAccount/32blit-beta.git

Now, on your computer navigate on your drive were you want your working local copy of the repo locatated, ie /home/YourAccount/repos
Then, type git clone and paste the link and hit enter, ie:
git clone https://github.com/YourAccount/32blit-beta.git

This will create a new folder called '32blit-beta' and download your repo here.
cd into that new folder, ie cd 32blit-beta
Here enter the following commads, with your info:
git config --global user.name "Your Name"
git config --global user.email "your@email.com"
Replace the values inside the quotes with your name and email address, this is to identify who made the canges and how to contact you, it's not your github account info.
[more detailed info here..](https://git-scm.com/book/en/v2/Getting-Started-First-Time-Git-Setup)

## Using git

Firstly, navigate to your 32blit-beta repo folder to run all your git commands.
Next, to see all your local repo changes, run:
git status

It will list tracked and untracked changes.
We want git to track the changes of files, we can add them individually or just add all the files in the project with a single command:
git add .

[more detailed info here..](https://git-scm.com/book/en/v2/Getting-Started-Getting-Help)


## Helpful links
* [Youtube video: Introduction to Git](https://www.youtube.com/watch?v=USjZcfj8yxE) [webpage: Introduction to Git] https://www.notion.so/Introduction-to-Git-ac396a0697704709a12b6a0e545db049) 
* [Cheat sheet] (https://github.github.com/training-kit)

