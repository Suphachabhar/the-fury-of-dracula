# the-fury-of-dracula
https://www.cse.unsw.edu.au/~cs2521/19T0/assignments/dracula/view/
Objectives

to implement the engine and some AIs for a game
to give you experience working in a team
to give you further practice with C and data structures
Admin
worth 15 marks towards your final mark in the course
deadlines for submissions are available on the relevant pages
this assignment is completed in your “Assignment 2” group
only one submission is needed for each team
Note
This assignment is in three parts. The first part is the introduction, where you learn about the game, and play with some of the data structures in the Week 4 Lab. The first submittable item is an implementation of the game view. The second submittable item involves regularly placing your game players (AIs) in a tournament system, which will play submissions against each other.

Details of the submittable items will be available soon. Marks for this assignment will be awarded based on each individual’s contribution to the group; no contribution gives no marks.

Background
This assignment is based on a real game called The Fury of Dracula, but the rules have been simplified for this assignment. The game might be more accurately called “Dracula Runs and Hides”, however, as the whole idea is that four Hunters are chasing Dracula and he is trying to evade them. Every time they catch him he loses some blood, and eventually, if he loses enough blood, he is vanquished. Dracula does the best he can to stay ahead of hunters who do the best they can to search for and find him.

FYI The game is a surprisingly good adaptation of the surprisingly good book Dracula by Bram Stoker. The game starts where the book leaves off. If you are interested, the book is available free from Project Gutenberg in written form and as a spoken novel.

In this assignment you will work in teams, where you will take on a Jekyll-and-Hyde split personality (apologies for the mixed monster metaphor). Part of you will be a Hunter, the other part will be Dracula himself. The Hunter part of you aims to hunt down and defeat Dracula. The Dracula part of you aims to evade the Hunters and regroup your dark minions so that one day you can wreak terrible vengeance on the world.

You will implement your Hunter and Dracula personalities via two AIs that will play the game against the AIs of other assignment teams.

What you are to do
This assignment has three phases: Introduction, View development, the Hunt (tournament). At this stage, only information for the Introduction is available. Information about the View and the Hunt will be released later.

Intro
In this phase, you should form your assignment team, read the Rules of the game and practice playing it with your team. The aim of this phase is for you to develop a good understanding of the problem before attempting to solve it.
View
In this phase, you develop the “eyes and ears” of your AIs. In technical terms, you will design and build an ADT that will give your player AIs access to the state of the game. There is one ADT for the Hunters (all Hunters behave the same) and one ADT for Dracula. The player AIs must then work out what’s happening in the game via this ADT, which will give them an appropriately restricted view of the game state. For further details, see the View.
Hunt
In this phase, you implement an AI for the Hunters and an AI for Dracula and enter them in tournaments to play against other teams’ AIs. In each tournament, your Dracula AI will play against all of the other submitted Hunter AIs, and your Hunter AI will play against all of the other submitted Dracula AIs. After all these games, rankings will be worked out for Hunters and Draculas. For further details, see the Hunt.
Maps of Europe
The game is set in Victorian (mid-late 1800’s) Europe, so a map of Europe from that time is an essential tool for playing the game. The critical aspects of the map are the cities, the seas and the road/rail/ferry links between them.

Note that you can only move from one city to another city via a direct road link between them, or you can move directly from one city to another over several rail links (trains are faster). From a port city, you can also move to the adjacent sea; once in a sea, you can subsequently move to any port city on that sea, or to any other sea which borders that sea.
