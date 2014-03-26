#pragma once
#ifndef SCENE_LEADERSBOARD_H
#define SCENE_LEADERSBOARD_H

#include "Iw2D.h"
#include "ig2d/ig.h"

// buttons
class LeadersboardButtonMenu: public IGButton {
public:
	LeadersboardButtonMenu();
	void buttonPressed();
	void buttonReleased();
};
class LeadersboardButtonBack: public IGButton {
public:
	LeadersboardButtonBack();
	void buttonPressed();
	void buttonReleased();
};
class LeadersboardButtonNext: public IGButton {
public:
	LeadersboardButtonNext();
	void buttonPressed();
	void buttonReleased();
};

// tags
typedef enum {
	LeadersboardTagItems = 200
} LeadersboardTags;

// instructions scene
class SceneLeadersboard: public IGScene {
public:
	SceneLeadersboard();
	~SceneLeadersboard();

	LeadersboardButtonBack* buttonBack;
	LeadersboardButtonNext* buttonNext;
	int numPages;
	int pageNumber;

	void pageBack();
	void pageNext();
	void updateLeadersboard();
};

#endif // SCENE_LEADERSBOARD_H
