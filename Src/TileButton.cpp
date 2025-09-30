#include "TileButton.hpp"


TileButton::TileButton(ingenico::graphics::Widget& parent,
                       const std::string& bgPng,
                       const std::string& iconPng,
                       const std::string& labelTxt,
                       int posX, int posY)
  : background(0),
    icon(0),
	back(0),
    label(0),
    x(posX),
    y(posY),
    width(0),
    height(0)

{
    using namespace ingenico::graphics;

    // Tailles définies en dur
    const int buttonWidth = 140;
    const int buttonHeight = 140;
    const int labelHeight = 30;
    const int iconSize = 54;

    back = new Picture(parent, "");
    back->setTransformation(GL_TRANSFORMATION_NONE);
//    back->setTransformation(GL_TRANSFORMATION_FIT_ALL);
    back->setSource(bgPng);
    back->setPosition(x, y, GL_UNIT_PIXEL);
    back->setSize(buttonWidth, buttonHeight, GL_UNIT_PIXEL);


    // Background - couvre TOUT le bouton
    background = new Button(parent, "");
    background->setTransformation(GL_TRANSFORMATION_FIT_ALL);
//    background->setSource(bgPng);
    background->setPosition(x, y, GL_UNIT_PIXEL);
    background->setSize(buttonWidth, buttonHeight, GL_UNIT_PIXEL);
    background->setClickable(true);
    background->setFocusable(false);
    background->setPressable(false);
    // Label (40px de hauteur en bas)
    label = new Label(parent, "");
    label->setText(labelTxt);
    label->setTextAlign(GL_ALIGN_CENTER);
    label->setForeColor(GL_COLOR_BLACK);
    label->setPosition(x, y + buttonHeight - labelHeight - 10, GL_UNIT_PIXEL);
    label->setSize(buttonWidth, labelHeight, GL_UNIT_PIXEL);
    label->setForeAlign(GL_ALIGN_CENTER);
    label->setFontScale(GL_SCALE_MEDIUM);
    label->setFontName("Arial");
    // Icon (54px, centré dans l'espace restant)
    const int availableHeight = buttonHeight - labelHeight;
    const int iconX = x + (buttonWidth - iconSize) / 2;
    const int iconY = y + (availableHeight - iconSize) / 2;

    icon = new Picture(parent, "");
    icon->setSource(iconPng);
    icon->setSize(iconSize, iconSize, GL_UNIT_PIXEL);
    icon->setPosition(iconX, iconY, GL_UNIT_PIXEL);
}

TileButton::~TileButton() {
    delete background;
    delete icon;
    delete label;
}

