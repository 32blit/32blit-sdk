# A quick Git guide

This is a very beginners and quick guide to git/github on how to contribute to the project.
First you're guided in setting up your clone github repo (short for repository) and then how to contribute to the main project.


---

## Setup your clone repo
First, make sure you have the git software install on your computer, follow the instructions here: https://git-scm.com/book/en/v2/Getting-Started-Installing-Git

Next, create or log into github.com

Go to: https://github.com/pimoroni/32blit-beta

Click the Fork link, this will mirror the official repo into your github account and it will redirect you there automatically, ie: YourAccount/32blit-beta.

Next, click the big green "Clone or download" button, you will copy the link provide, ie: github.com/YourAccount/32blit-beta.git

Now, on your computer navigate on your drive were you want your working local copy of the repo located, ie /home/YourAccount/repos

Then, type git clone and paste the link and hit enter, ie:

$ git clone https://github.com/YourAccount/32blit-beta.git

Where YourAccount is your github account

[more detailed info here..](https://git-scm.com/book/en/v2/Git-Basics-Getting-a-Git-Repository)

This will create a new folder called '32blit-beta' and download your repo here.

cd into that new folder, ie cd 32blit-beta.

Here enter the following commands, with your info:

$ git config --global user.name "Your Name"

$ git config --global user.email "your(at)email.com"

Replace the values inside the quotes with your name and email address, this is to identify who made the changes and how to contact you, it's not your github account info.


If you use vi or vim, change the defualt editor to:
$ git config --global core.editor vim

$ git config --global merge.tool vimdiff



[more detailed info here..](https://git-scm.com/book/en/v2/Getting-Started-First-Time-Git-Setup)

---

## Using git on your local repo

### listing changes
Firstly, navigate to your 32blit-beta repo folder to run all your git commands.

As a test create or modify a file in the repo and run this command:

$ git status

### Add command
It will list tracked and untracked files that have been changed.

We want git to track (or store) the changes of the files in a Staging Area (ie revision history), so we need to tell git to track them with the add command.

We can add the files individually or just add all the files recursively in the project with a single command:

$ git add .


[more detailed info on add here..](https://git-scm.com/book/en/v2/Getting-Started-Getting-Help) and [here](https://git-scm.com/book/en/v2/Git-Basics-Recording-Changes-to-the-Repository)


### Ignored files
view the file .gitignore in the repo root folder, it shows you which files will not be tracked and most-likely you wont need to change these.


### Local changes
To see your changes in the repo run:

$ git diff 


To limit to files only tracked (in the staged area), run:

$ git diff --staged

### Removing or moving a local file
Remove it as normal, then to tell git to remove it from the repository, run:

$ git rm deleted-filename

Similarly, moving a file run:

$ git mv moved-filename


### Local commit
To track your file changes over time, you will need to make (many) local commits to your files, run:

$ git commit -a -m "Commit message"

This command will store changes to all files modified, not in the .gitignore file and use the message in between quotes.

Remember, if you remove or add a file, you'll need to do a "git add ." command again, before committing.


---

## Your remote repo

### Updating your remote repo
You will be asked for you github username and password each time:

$ git push

---
---

## Merging into offical Head repo

Log into github.com.
On your 32blit-beta clone repo, click the "Pull request" button.
Now, clieck the "create pull request" button.

---

# Helpful links
* [Youtube video: Introduction to Git](https://www.youtube.com/watch?v=USjZcfj8yxE) and the [webpage: Introduction to Git](https://www.notion.so/Introduction-to-Git-ac396a0697704709a12b6a0e545db049)
* [Cheat sheet](https://github.github.com/training-kit)
