#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_


#include <list>
#include <vector>
#include "GameWorld.h"

#include <string>
#include "Actor.h"

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetPath);
    ~StudentWorld();
    virtual int init();
    virtual int move();
    virtual void cleanUp();
    void pushActor(Actor* a);
    bool isMovementOverlap(int x1, int y1, int x2, int y2);
    bool isOverlap(int x1, int y1, int x2, int y2);
    Actor* findOverlap(int x1, int y1, bool shooting, bool findingFood, bool movement);
    bool socratesOverlap(int x1, int y1);
    Socrates* getSocrates();
    bool isOutOfBounds(int x1, int y1);
    Actor* findFood(int x1, int y1);
    int distanceFromSocrates(int x1, int y1);
    int findDirection(Actor* actor, int x1, int y1);
    void populateField(int num, int type);
    void addGoodies(int chance, bool isGood);
    
private:
    std::list<Actor*> actors;
    std::vector<Actor*> toBeAddedActors;
    std::vector<Actor*> pitsAndFood;
    Socrates* socrates;
};

#endif // STUDENTWORLD_H_
