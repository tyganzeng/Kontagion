#include "StudentWorld.h"
#include "GameConstants.h"
#include "Actor.h"

#include <cmath>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

// Students:  Add code to this file, StudentWorld.h, Actor.h and Actor.cpp

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath)
{
}

StudentWorld::~StudentWorld(){
    cleanUp();
}

int StudentWorld::init()
{
    
    int numPits = getLevel();
    populateField(numPits, 0);

    int numFood = min(5 * getLevel(), 25);
    populateField(numFood, 1);
    

    int numDirt = max(180 - 20 * getLevel(), 20);
    populateField(numDirt, 2);
    
    socrates = new Socrates(this);
    
    pitsAndFood.clear();
    
    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
    
    // This code is here merely to allow the game to build, run, and terminate after you hit enter.
    // Notice that the return value GWSTATUS_PLAYER_DIED will cause our framework to end the current level.
    if(socrates->isAlive()){
        socrates->doSomething();
    }
    
    for(list<Actor*>::iterator i = actors.begin(); i != actors.end(); i++){
        
        if((*i)->isAlive()){
            (*i)->doSomething();
        }
        
        if(socrates->getHealth() <= 0){
            socrates->playDeathSound();
            decLives();
            return GWSTATUS_PLAYER_DIED;
        }
    }
    
    for(vector<Actor*>::iterator i = toBeAddedActors.begin(); i != toBeAddedActors.end(); i++){
        actors.push_back(*i);
    }
    toBeAddedActors.clear();
    
    
    
    //deleting dead actors
    for(list<Actor*>::iterator i = actors.begin(); i != actors.end(); i++){
        if(!(*i)->isAlive()){
            Actor* holder = *i;
            i = actors.erase(i);
            i--;
            delete holder;
        }
    }
    
    //adding new actors
    int chanceFungus = max(510 - getLevel() * 10, 200);
    addGoodies(chanceFungus, false);

    int chanceGoodie = max(510 - getLevel() * 10, 250);
    addGoodies(chanceGoodie, true);
    
    //updating game text
    ostringstream oss;
    oss << "Score: ";
    oss.fill('0');
    oss << setw(6) << getScore();
    oss.fill(' ');
    oss << "  Level: " << setw(2) << getLevel();
    oss << "  Lives: " << getLives();
    oss << "  Health: " << setw(3) << socrates->getHealth();
    oss << "  Sprays: " << setw(2) << socrates->numSprays();
    oss << "  Flames: " << setw(2) <<socrates->numFlames();
    setGameStatText(oss.str());
    
    //checking level completion
    bool levelComplete = true;
    for(list<Actor*>::iterator i = actors.begin(); i != actors.end(); i++){
        if((*i)->mustBeCleared()){
            levelComplete = false;
        }
    }
    
    if(levelComplete){
        return GWSTATUS_FINISHED_LEVEL;
    }

    return GWSTATUS_CONTINUE_GAME;

}

void StudentWorld::cleanUp()
{
    delete socrates;
    socrates = nullptr;
    for(vector<Actor*>::iterator i = toBeAddedActors.begin(); i != toBeAddedActors.end(); i++){
        delete *i;
    }
    toBeAddedActors.clear();
    
    for(list<Actor*>::iterator i = actors.begin(); i != actors.end(); i++){
        
        delete *i;
        
    }
    actors.clear();
}

void StudentWorld::pushActor(Actor* a)
{
    toBeAddedActors.push_back(a);
}

bool StudentWorld::isMovementOverlap(int x1, int y1, int x2, int y2){
    double distance = sqrt(pow(x2-x1, 2) + pow(y2-y1, 2) );
    if(distance <= SPRITE_WIDTH/2){
        return true;
    }
    return false;
}

bool StudentWorld::isOverlap(int x1, int y1, int x2, int y2){
    double distance = sqrt(pow(x2-x1, 2) + pow(y2-y1, 2) );
    if(distance <= SPRITE_WIDTH){
        return true;
    }
    return false;
}


Actor* StudentWorld::findOverlap(int x1, int y1, bool shooting, bool findingFood, bool movement){
    for(list<Actor*>::iterator i = actors.begin(); i != actors.end(); i++){
        if(movement){
            if(isMovementOverlap(x1, y1, (*i)->getX(), (*i)->getY()) && (*i)->blocksMovement()){
                return *i;
            }
        } else if(isOverlap(x1, y1, (*i)->getX(), (*i)->getY())){
            if(shooting && (*i)->canBeShot()){
         
                return *i;
            }
            if(findingFood && (*i)->isFood()){
                return *i;
            }

            
            if(!shooting && !findingFood){
                return *i;
            }
            
        }
    }
    
    return nullptr;
}

bool StudentWorld::socratesOverlap(int x1, int y1){
    if(isOverlap(x1, y1, socrates->getX(), socrates->getY())){
        return true;
    }
    return false;
}

Socrates* StudentWorld::getSocrates(){
    return socrates;
}

bool StudentWorld::isOutOfBounds(int x1, int y1){
    double distance = sqrt(pow(x1 - VIEW_WIDTH/2, 2) + pow(y1 - VIEW_HEIGHT/2,2));
    if(distance > VIEW_RADIUS){
        return true;
    }
    return false;
    
}

Actor* StudentWorld::findFood(int x1, int y1){
    int shortestDistance = 256;
    Actor* closestFood = nullptr;
    
    for(list<Actor*>::iterator i = actors.begin(); i != actors.end(); i++){
        if((*i)->isFood()){
            double distance = sqrt(pow((*i)->getX() - x1, 2) + pow((*i)->getY() - y1, 2) );
            if(distance < 128 && distance < shortestDistance){
                shortestDistance = distance;
                closestFood = *i;
            }
        }
        
    }
    
    return closestFood;
}

int StudentWorld::distanceFromSocrates(int x1, int y1){
    double distance = sqrt(pow(socrates->getX()-x1, 2) + pow(socrates->getY()-y1, 2) );
    return distance;

}

int StudentWorld::findDirection(Actor* actor, int x1, int y1){
    double direction;
    double distX;
    double distY;
    distX = actor->getX() - x1;
    distY = actor->getY() - y1;
    
    if(distX < 0 && distY > 0){
        direction = 180 + atan(distY / distX ) * 180 / (M_PI);
    } else if (distX < 0 && distY < 0){
        direction = -180 + atan(distY / distX ) * 180 / (M_PI);
    } else if (distX == 0){
        if(distY > 0){
            direction = 90;
        } else {
            direction = 270;
        }
    } else if (distY == 0){
        if(distX > 0){
            direction = 0;
        } else {
            direction = 180;
        }
    }
    else {
        direction = atan(distY / distX ) * 180 / (M_PI);
    }
    
    return direction;
}

void StudentWorld::populateField(int num, int type){
    for(int i = 0; i < num; i++){
        int distance = 300;
        int xDistance;
        int yDistance;
        xDistance = randInt(-120, 120);
        yDistance = randInt(-120, 120);
        
        if(type == 0 || type == 1){
            while(distance > 120 || findOverlap(VIEW_WIDTH/2 + xDistance, VIEW_HEIGHT/2 + yDistance , false, false, false) != nullptr){
                
                xDistance = randInt(-120, 120);
                yDistance = randInt(-120, 120);
                distance = sqrt((xDistance * xDistance) + (yDistance * yDistance));
            }
        } else {
            bool onTop = false;
            while(distance > 120 || onTop){
                xDistance = randInt(-120, 120);
                yDistance = randInt(-120, 120);
                onTop = false;
                for(vector<Actor*>::iterator i = pitsAndFood.begin(); i != pitsAndFood.end(); i++){
                    if(isOverlap(VIEW_WIDTH/2 + xDistance, VIEW_HEIGHT/2 + yDistance, (*i)->getX(), (*i)->getY())){
                        onTop = true;
                    }
                }
                distance = sqrt((xDistance * xDistance) + (yDistance * yDistance));
            }
        }
        Actor* newGuy;
        if(type == 0){
            newGuy = new Pit( VIEW_WIDTH/2 + xDistance, VIEW_HEIGHT/2 + yDistance, this);
            pitsAndFood.push_back(newGuy);
        } else if(type == 1){
            newGuy = new Food( VIEW_WIDTH/2 + xDistance, VIEW_HEIGHT/2 + yDistance, this);
            pitsAndFood.push_back(newGuy);

        } else if(type == 2){
            newGuy = new Dirt( VIEW_WIDTH/2 + xDistance, VIEW_HEIGHT/2 + yDistance, this);
            
        }
        actors.push_back(newGuy);
    }

}

void StudentWorld::addGoodies(int chance, bool isGood){
    int rand = randInt(0, chance - 1);
    if(rand == 0){
        int randAngle = randInt(0,359);
        int startX = VIEW_RADIUS * cos(randAngle * M_PI / 180) + VIEW_WIDTH/2;
        int startY = VIEW_RADIUS * sin(randAngle * M_PI / 180) + VIEW_HEIGHT/2;
        
        if(isGood){
            int randGoodie = randInt(0,9);
            if(randGoodie < 6){
                actors.push_back(new RestoreHealthGoodie(startX, startY, getLevel(), this));
            } else if (randGoodie < 9){
                actors.push_back(new FlamethrowerGoodie(startX, startY, getLevel(), this));
            } else if (randGoodie == 9){
                actors.push_back(new ExtraLifeGoodie(startX, startY, getLevel(), this));
            }
        } else{
            actors.push_back(new Fungus(startX, startY, getLevel(), this));
        }
    }
}
