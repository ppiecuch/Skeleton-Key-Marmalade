#include "scene_leadersboard.h"
#include "IwResManager.h"
#include "settings.h"
#include "sounds.h"
#include "scene_menu.h"
#include "leadersboard.h"

// menu button
LeadersboardButtonMenu::LeadersboardButtonMenu() {
	imageNormalName = std::string("leadersboard_menu");
	imageNormal = IGResourceManager::getInstance()->getImage(imageNormalName);
	imageSelectedName = std::string("leadersboard_menu2");
	imageSelected = IGResourceManager::getInstance()->getImage(imageSelectedName);
	image = imageNormal;
	this->set(IGPoint(160,452));
	touchSize = IGRect(98,56);
	z = 2;
	tag = 0;
}
void LeadersboardButtonMenu::buttonPressed() {
	Sounds::getInstance()->playClick();
}
void LeadersboardButtonMenu::buttonReleased() {
	parent->removeAllChildren();
	IGDirector::getInstance()->switchScene(new SceneMenu());
}

// back button
LeadersboardButtonBack::LeadersboardButtonBack() {
	imageNormalName = std::string("achievements_back");
	imageNormal = IGResourceManager::getInstance()->getImage(imageNormalName);
	imageSelectedName = std::string("achievements_back2");
	imageSelected = IGResourceManager::getInstance()->getImage(imageSelectedName);
	image = imageNormal;
	this->set(IGPoint(55,452));
	touchSize = IGRect(111,56);
	z = 2;
	tag = 0;
}
void LeadersboardButtonBack::buttonPressed() {
	Sounds::getInstance()->playClick();
}
void LeadersboardButtonBack::buttonReleased() {
	((SceneLeadersboard*)parent)->pageBack();
}

// next button
LeadersboardButtonNext::LeadersboardButtonNext() {
	imageNormalName = std::string("leadersboard_next");
	imageNormal = IGResourceManager::getInstance()->getImage(imageNormalName);
	imageSelectedName = std::string("leadersboard_next2");
	imageSelected = IGResourceManager::getInstance()->getImage(imageSelectedName);
	image = imageNormal;
	this->set(IGPoint(265,452));
	touchSize = IGRect(111,56);
	z = 2;
	tag = 0;
}
void LeadersboardButtonNext::buttonPressed() {
	Sounds::getInstance()->playClick();
}
void LeadersboardButtonNext::buttonReleased() {
	((SceneLeadersboard*)parent)->pageNext();
}

// leadersboard scene
SceneLeadersboard::SceneLeadersboard() : IGScene()
{
	IGLog("SceneLeadersboard init");
	
	// load the resources
	IwGetResManager()->LoadGroup("leadersboard.group");
	
	// the background
	IGSprite* spriteBackground = new IGSprite("background_wood", IGPoint(160,240), 0);
	this->addChild(spriteBackground);

	// header
	IGSprite* spriteHeader = new IGSprite("leadersboard_header", IGPoint(160,41), 1);
	this->addChild(spriteHeader);

	// the menu items
	buttonBack = new LeadersboardButtonBack();
	buttonBack->setOpacity(64);
	LeadersboardButtonMenu* buttonMenu = new LeadersboardButtonMenu();
	buttonNext = new LeadersboardButtonNext();
	this->addChild(buttonBack);
	this->addChild(buttonMenu);
	this->addChild(buttonNext);

	// pages
	pageNumber = 0;
	numPages = (int)(LEADERSBOARD_NUM / 5);
	if(LEADERSBOARD_NUM % 5 > 0)
		numPages++;

	// display leadersboard
	updateLeadersboard();
}

SceneLeadersboard::~SceneLeadersboard() {
}

void SceneLeadersboard::pageBack() {
	if(pageNumber > 0) {
		pageNumber--;
		if(pageNumber == 0)
			buttonBack->setOpacity(64);
		buttonNext->setOpacity(255);
		updateLeadersboard();
	}
}

void SceneLeadersboard::pageNext() {
	if(pageNumber < numPages-1) {
		pageNumber++;
		if(pageNumber == numPages-1)
			buttonNext->setOpacity(64);
		buttonBack->setOpacity(255);
		updateLeadersboard();
	}
}

void SceneLeadersboard::updateLeadersboard() {
	int i;
	// start by getting rid of all the old achievements
	for(i=0; i<4*5; i++) // 4 element for each of 5 achivments on the page
	  removeChildByTag(LeadersboardTagItems+i);

	// figure out what achievements need displaying
	int start = pageNumber*5;
	int end = start+5;
	if(end >= LEADERSBOARD_NUM)
		end = LEADERSBOARD_NUM;

	// display them
	int curTag = LeadersboardTagItems;
	int curItem = 0;
	for(i=start; i<end; i++) {
		LeadersboardScore a = Leadersboard::getInstance()->getLeadersboardScore(i);
		
		// row background
		IGSprite* spriteRowBackground = new IGSprite("achievements_row_background", IGPoint(160,116+68*curItem), 5, curTag);
		this->addChild(spriteRowBackground);
		curTag++;

		// name and description
		IGLabel* labelName = new IGLabel("font_gabriola_16b", a.name.c_str(), IGPoint(196,105+68*curItem), IGRect(232,34), 6, curTag);
		labelName->setColor(255,255,255,255);
		this->addChild(labelName);
		curTag++;
		const char *score = f_ssprintf("level %d (in %d seconds)", a.level, a.sec);
		IGLabel* labelDescription = new IGLabel("font_gabriola_14", score, IGPoint(196,127+68*curItem), IGRect(232,34), 6, curTag);
		labelDescription->setColor(196,207,226,255);
		this->addChild(labelDescription);
		curTag++;

		curItem++;
	}
}
